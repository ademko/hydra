
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/WebExport.h>

#include <assert.h>

#include <algorithm>
#include <iostream>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>

#include <hydra/Engine.h>
#include <hydra/FileOp.h>
#include <hydra/Records.h>
#include <hydra/RotateCode.h>
#include <hydra/Thumb.h>
#include <hydra/XMLUtil.h>

using namespace hydra;

namespace {
const char *StyleCSSFileName = "hydraweb.css";
}

// TODO always make sure that dm_outdir has a trailing / (if not empty) so that
// we dont have to do that in every usage

//
//
// DirEntry
//
//

WebExport::DirEntry::DirEntry(const QString &_basedir)
    : isroot(false), totalfiles(-1), basedir(_basedir) {
    if (basedir.isEmpty())
        urlname = "index";
    else {
        for (QString::const_iterator ii = basedir.begin(); ii != basedir.end();
             ++ii)
            if (ii == basedir.begin() && *ii == '/')
                ; // do nothing
            else if (*ii == '/')
                urlname.append(',');
            else
                urlname.append(*ii);
    }

    urlname.append(".html");

    justname = QFileInfo(basedir).fileName();
}

//
//
// FileEntry
//
//

void WebExport::FileEntry::calc(void) {
    assert(!basefilename.isEmpty());
    assert(urlnamebase.isEmpty());

    for (QString::const_iterator ii = basefilename.begin();
         ii != basefilename.end(); ++ii)
        if (ii == basefilename.begin() && *ii == '/')
            ; // do nothing
        else if (*ii == '/')
            urlnamebase.append(',');
        else
            urlnamebase.append(*ii);

    urlhtml = urlnamebase + ".html";
    // urlorigimage = "original_files/" + urlnamebase;
    urlorigimage = urlnamebase;
    urlviewimage = "VIEW," + urlnamebase;
    urlthumbimage = "THUMB," + urlnamebase;

    original_file_size = QFileInfo(fullfilename).size();
}

//
//
// WebExport
//
//

WebExport::WebExport(const QString &outputdir, QTextStream &out)
    : dm_out(out), dm_outdir(outputdir), dm_title("Gallery"), dm_configDelayImageLoadMS(12000) {
    addDirComponents("");
}

void WebExport::setTitle(const QString &title) {
    dm_title = title;
}

void WebExport::setImageLoadDelay(int delayMs) {
    dm_configDelayImageLoadMS = delayMs;
}

void WebExport::addFile(const QString &fullfilename, const QString &basedir,
                        hydra::FileItemRecord &item, const QString &filehash) {
    // dm_out << "adding full=" << fullfilename.toUtf8().constData() << "
    // basedir=" << basedir.toUtf8().constData() << endl;

    std::shared_ptr<FileEntry> entry(new FileEntry);

    entry->rotateCode = detectMultiRotate(fullfilename, item.tags);

    entry->id = item.id;
    entry->fullfilename = fullfilename;
    entry->justname = QFileInfo(fullfilename).fileName();
    entry->title = item.title;
    entry->desc = item.desc;
    entry->filetype = item.filetype;
    entry->filehash = filehash;

    entry->basedir = basedir;
    // entry->basejustname = entry->justname;
    // entry->basefilename = entry->basedir + "/" + entry->basejustname;

    // insert until its not a dupe
    int dupeid = 1;
    do {
        entry->basejustname.clear();
        QTextStream str(&entry->basejustname);

        if (dupeid > 1)
            str << dupeid++;

        QFileInfo info(entry->justname);
        str << info.completeBaseName() << "_r"
            << rotateCodeToDegrees(entry->rotateCode) << "." + info.suffix();

        entry->basefilename = entry->basedir + "/" + entry->basejustname;

        dupeid++;
    } while (dm_basefiles.count(entry->basefilename) > 0 &&
             dm_basefiles[entry->basefilename]->id != item.id);

    entry->calc();

    // insert the new entry
    dm_basefiles[entry->basefilename] = entry;

    // add the dir and add this item to that dir
    addDirComponents(basedir);
    if (item.filetype == 0)
        dm_basedirs[basedir]->subimages.push_back(entry);
    else
        dm_basedirs[basedir]->subfiles.push_back(entry);
    entry->parent = dm_basedirs[basedir];
}

int WebExport::commitWebSite(void) {
    // QDir(dm_outdir).mkdir("original_files");
    Thumb::mkThumbDir();

    buildDirTree();
    computeDirCounts(*dm_basedirs[""]);

    for (DirMap::const_iterator ii = dm_basedirs.begin();
         ii != dm_basedirs.end(); ++ii)
        sortDirIndex(*(ii->second));

    writeStaticFiles();

    for (DirMap::const_iterator ii = dm_basedirs.begin();
         ii != dm_basedirs.end(); ++ii) {
        QString outfilename(dm_outdir + "/" + ii->second->urlname);

        if (writeDirIndexHtml(outfilename, *(ii->second)))
            dm_out << "INDEX " << outfilename << Qt::endl;
        else
            dm_out << "!ERROR " << outfilename << Qt::endl;

        writeAllImageHtmls(*(ii->second));
    }

    return writeImageFiles();
}

int WebExport::commitFileCopy(void) {
    buildDirTree();
    fileCopyMakeDirs();
    return fileCopyCopyFiles();
}

void WebExport::addDirComponents(const QString &dir) {
    QString cur;

    // add the root component
    if (dm_basedirs.find(cur) == dm_basedirs.end())
        dm_basedirs[cur] = std::shared_ptr<DirEntry>(new DirEntry(cur));

    if (dir.isEmpty())
        return; // end here, nothing more to do

    // now add all the dir compoenents
    QString::const_iterator ii = dir.begin();

    while (true) {
        if (ii == dir.end() || *ii == '/') {
            // found a component
            if (dm_basedirs.find(cur) == dm_basedirs.end())
                dm_basedirs[cur] = std::shared_ptr<DirEntry>(new DirEntry(cur));
            cur += '/';
        } else
            cur += *ii;
        if (ii == dir.end()) // are we done
            return;
        ii++;
    }
}

void WebExport::buildDirTree(void) {
    DirMap::iterator ii, kk;

    // add the root node
    assert(dm_basedirs[""].get());
    dm_basedirs[""]->isroot = true;

    // for each directory, find its parent
    // a parent is the longest (in terms of number of /)
    // that is still a string subset of this dir
    for (ii = dm_basedirs.begin(); ii != dm_basedirs.end(); ++ii) {
        if (ii->first.isEmpty())
            continue; // skip the root, it has no parents

        QString bestsofar;
        int bestn = -1; // "" has 0 slashes, while "blah/blah" has two (one
                        // extre virtual one, so that "blah" and "" dont end up
                        // havin the same count
        int curn, myn;

        // first, count how many slashes in this node
        myn = 1; // start from 1, so not to override the root node
        for (QString::const_iterator uu = ii->first.begin();
             uu != ii->first.end(); ++uu)
            if (*uu == '/')
                ++myn;

        for (kk = dm_basedirs.begin(); kk != dm_basedirs.end(); ++kk) {
            if (ii == kk)
                continue; // you cant be your own parent
            if (kk->first.isEmpty())
                continue; // no need to compare against the root
            // next, do subset comparison
            if (kk->first.size() >= ii->first.size())
                continue; // subset must be shorter than current
            if (ii->first.leftRef(kk->first.size()) != kk->first)
                continue; // not a subset match
            // count the / in this potential new parent
            curn = 1; // start from 1, so not to override the root node
            for (QString::const_iterator uu = kk->first.begin();
                 uu != kk->first.end(); ++uu)
                if (*uu == '/')
                    ++curn;
            // finally, is this a new winner?
            if (curn > bestn && curn < myn) {
                bestn = curn;
                bestsofar = kk->first;
            }
        } // kk

        // new parent found, add it
        dm_basedirs[bestsofar]->subdirs.push_back(ii->second);
        ii->second->parent = dm_basedirs[bestsofar];
    } // for
}

void WebExport::computeDirCounts(DirEntry &dir) {
    dir.totalfiles = 0;

    for (DirSet::iterator ii = dir.subdirs.begin(); ii != dir.subdirs.end();
         ++ii) {
        computeDirCounts(*(*ii));
        dir.totalfiles += (*ii)->totalfiles;
    }
    dir.totalfiles += dir.subimages.size();
    dir.totalfiles += dir.subfiles.size();
}

int WebExport::writeImageFiles(void) {
    int code;
    int count = 0;
    QString outfile, outthumb;

    for (FileMap::iterator ii = dm_basefiles.begin(); ii != dm_basefiles.end();
         ++ii) {
        FileEntry &entry = *(ii->second);

        outfile = dm_outdir + "/" + entry.urlorigimage;
        if (!QFileInfo(outfile).exists()) {
            dm_out << "  ORIGINAL " << entry.fullfilename << Qt::endl;

            smartCopy(entry.fullfilename, outfile);
        }

        // if it's not an image, stop now
        if (entry.filetype != 0)
            continue;

        ++count;

        // make the thumbnails
        Thumb T(entry.fullfilename);

        assert(entry.rotateCode >= 0);

        outfile = dm_outdir + "/" + entry.urlthumbimage;
        if (!QFileInfo(outfile).exists()) {
            outthumb =
                Thumb::fileName(entry.filehash, entry.rotateCode,
                                Thumb::DEFAULT_THUMB_W, Thumb::DEFAULT_THUMB_H);

            dm_out << "  THUMB " << entry.fullfilename << Qt::endl;

            code = T.generate(outthumb, 0, entry.rotateCode,
                              Thumb::DEFAULT_THUMB_W, Thumb::DEFAULT_THUMB_H);
            if (code != Thumb::Generate_Ok &&
                code != Thumb::Generate_FileExists) {
                dm_out << "!ERROR THUMB " << outthumb << " code " << code
                       << Qt::endl;
                continue;
            }
            smartCopy(outthumb, outfile);
        } // if

        outfile = dm_outdir + "/" + entry.urlviewimage;
        if (!QFileInfo(outfile).exists()) {
            outthumb =
                Thumb::fileName(entry.filehash, entry.rotateCode,
                                Thumb::DEFAULT_VIEW_W, Thumb::DEFAULT_VIEW_H);

            dm_out << "  VIEWTHUMB " << entry.fullfilename << Qt::endl;

            code = T.generate(outthumb, 0, entry.rotateCode,
                              Thumb::DEFAULT_VIEW_W, Thumb::DEFAULT_VIEW_H);
            if (code != Thumb::Generate_Ok &&
                code != Thumb::Generate_FileExists) {
                dm_out << "!ERROR VIEWTHUMB " << outthumb << " code " << code
                       << Qt::endl;
                continue;
            }
            smartCopy(outthumb, outfile);
        } // if
    }     // big for

    return count;
}

void WebExport::writeHydraImg(QTextStream &out, int id,
                              const FileEntry &entry) {
    out << "   <hydraimg id=\"" << id << "\" url=\""
        << escapeForXML(entry.urlhtml) << "\" view_img=\""
        << escapeForXML(entry.urlviewimage) << "\" img=\""
        << escapeForXML(entry.urlorigimage) << "\" />\n";
}

static inline bool IsOkId(int test_id, int num_ids) {
    return test_id > 0 && (test_id + 1) < num_ids;
}

void WebExport::writeStaticFiles(void) {
    QString outfilename(dm_outdir + "/" + StyleCSSFileName);
    QFile outfile(outfilename);

    if (!outfile.open(QIODevice::WriteOnly))
        return; // switch to throwing exceptions on errors?

    QTextStream out(&outfile);

    out << R"EOF(
.navBut {
	-moz-box-shadow:inset 0px 1px 3px 0px #91b8b3;
	-webkit-box-shadow:inset 0px 1px 3px 0px #91b8b3;
	box-shadow:inset 0px 1px 3px 0px #91b8b3;
	background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #768d87), color-stop(1, #6c7c7c));
	background:-moz-linear-gradient(top, #768d87 5%, #6c7c7c 100%);
	background:-webkit-linear-gradient(top, #768d87 5%, #6c7c7c 100%);
	background:-o-linear-gradient(top, #768d87 5%, #6c7c7c 100%);
	background:-ms-linear-gradient(top, #768d87 5%, #6c7c7c 100%);
	background:linear-gradient(to bottom, #768d87 5%, #6c7c7c 100%);
	filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#768d87', endColorstr='#6c7c7c',GradientType=0);
	background-color:#768d87;
	-moz-border-radius:5px;
	-webkit-border-radius:5px;
	border-radius:5px;
	border:1px solid #566963;
	display:inline-block;
	cursor:pointer;
	color:#ffffff;
	font-family:Arial;
	font-size:15px;
	font-weight:bold;
	padding:6px 10px;
	text-decoration:none;
	text-shadow:0px -1px 0px #2b665e;
}
.navBut:hover {
	background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #6c7c7c), color-stop(1, #768d87));
	background:-moz-linear-gradient(top, #6c7c7c 5%, #768d87 100%);
	background:-webkit-linear-gradient(top, #6c7c7c 5%, #768d87 100%);
	background:-o-linear-gradient(top, #6c7c7c 5%, #768d87 100%);
	background:-ms-linear-gradient(top, #6c7c7c 5%, #768d87 100%);
	background:linear-gradient(to bottom, #6c7c7c 5%, #768d87 100%);
	filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#6c7c7c', endColorstr='#768d87',GradientType=0);
	background-color:#6c7c7c;
}
.navBut:active {
	position:relative;
	top:1px;
}

div#topBar { width: 100%; text-align: center; margin: 6px 0px; }
div#topBar #leftBut { float: left; margin-left: 2px; }
div#topBar #rightBut { float: right; margin-right: 2px; }
div#imageCaption { color: grey; font-family:Arial; font-size:15px; margin: 1em 1em; text-align: center; }
.optionsDropDown { position: relative; display: inline-block; }
.optionsBut { }
.optionsContent { display: none; position: absolute; right:-2px; z-index: 1; 
text-align: right; background: black; padding: 6px 4px 10px 10px; border-radius: 0 0 0 10px;}
.optionsLink { margin: 2px 0; white-space: nowrap; }
.optionsDropDown:hover .optionsContent { display: block; }

img#mainImage { width: 98vw; object-fit: contain; max-height: 90vh; }

div.wideLinkListHeader { font-family: Arial; font-weight: bold; text-align: center; }
div.wideLinkList div { font-family: Arial; display: inline-block; width: 17em; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; border: 1px solid #000000; }
div.wideLinkList a { color: #8f96ff; font-weight: bold; text-decoration: none; }
div.wideLinkList a:hover { text-decoration: underline; }

div#imageIndex img { }
.albumTitle { color: white; font-family:Arial; font-size:28px; text-align: center; }

body { background-color: black; color: white; }
img { border:0; }
)EOF";
}

void WebExport::sortDirIndex(DirEntry &entry) {
    // sort by name for display
    std::sort(entry.subdirs.begin(), entry.subdirs.end(), DirEntryLT);
    // sort by name for display
    std::sort(entry.subfiles.begin(), entry.subfiles.end(), FileEntryLT);
    // sort by name for display
    std::sort(entry.subimages.begin(), entry.subimages.end(), FileEntryLT);
}

bool WebExport::writeImageHtml(const QString &outfilename, int myid,
                               int randomId, int numpeers,
                               const FileEntry &entry) {
    QFile outfile(outfilename);

    if (!outfile.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&outfile);

    out.setRealNumberPrecision(2);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    std::shared_ptr<DirEntry> parent(entry.parent);

    assert(parent.get());

    out << "<!DOCTYPE html>\n"
           "<html>\n"
           "<!-- hydraweb " HYDRA_VERSION_STRING " demko.ca -->\n"
           "<head>\n";

    out << "<title>#" << (myid + 1) << '/' << numpeers << ' '
        << escapeForXML(entry.justname)
        << "</title>\n"
           "<meta name=\"viewport\" content=\"width=device-width, "
           "initial-scale=1\" />\n"
           "<meta http-equiv=\"Content-Type\" content=\"text/html\" />\n"
           "<meta charset=\"utf-8\" />\n"
           "<link href=\""
        << StyleCSSFileName
        << "\" type=\"text/css\" rel=\"stylesheet\" />\n"
           "</head>\n";

    // body

    // navbar
    out << "<body>\n\n"
           "<div id=\"topBar\" >\n"
           "<span id=\"leftBut\"><a class=\"navBut\" href=\""
        << escapeForXML(parent->urlname) << "\">Album</a></span>\n";

    out << "<span id=\"rightBut\" class=\"optionsDropDown\">\n"
           "<a class=\"navBut optionsBut\" href=\"#\">Menu...</a>\n"
           "<div class=\"optionsContent\">\n"
           "<a class=\"navBut optionsLink\" href=\""
        << escapeForXML(entry.urlorigimage) << "\">Download</a>\n";
    if (myid > 0)
        out << "<a class=\"navBut optionsLink\" href=\""
            << escapeForXML(parent->subimages[0]->urlhtml)
            << "\">&lt;&lt;First</a>\n";
    if (myid + 1 < numpeers)
        out << "<a class=\"navBut optionsLink\" href=\""
            << escapeForXML(parent->subimages[numpeers - 1]->urlhtml)
            << "\">Last&gt;&gt;</a>\n";

    if (numpeers >= 1000) { // threshold to show the new bar 1000
        out << " <a class=\"navBut optionsLink\" href=\""
            << escapeForXML(parent->subimages[randomId]->urlhtml)
            << "\">Random</a>\n";

        for (int delta = 1000000; delta > 1; delta /= 10) {
            if (myid + delta < numpeers)
                out << "<a class=\"navBut optionsLink\" href=\""
                    << escapeForXML(parent->subimages[myid + delta]->urlhtml)
                    << "\">Jump +" << delta << "</a>\n";
        }
        for (int delta = 10; delta <= 1000000; delta *= 10) {
            if (myid - delta >= 0)
                out << " <a class=\"navBut optionsLink\" href=\""
                    << escapeForXML(parent->subimages[myid - delta]->urlhtml)
                    << "\">Jump -" << delta << "</a>\n";
        }
    }
    out << "</div></span>\n";

    if (myid > 0)
        out << "<a class=\"navBut\" href=\""
            << escapeForXML(parent->subimages[myid - 1]->urlhtml)
            << "\">&lt;Prev</a>\n";
    if (myid + 1 < numpeers)
        out << "<a class=\"navBut\" href=\""
            << escapeForXML(parent->subimages[myid + 1]->urlhtml)
            << "\">Next&gt;</a>\n";
    out << "</div>\n\n";

    // image
    out << "<a href=\"";
    if (myid + 1 < numpeers)
        out << escapeForXML(parent->subimages[myid + 1]->urlhtml);
    else
        out << escapeForXML(parent->subimages[0]->urlhtml);
    out << "\"><img id=\"mainImage\" border=\"0\" src=\"" << escapeForXML(entry.urlviewimage)
        << "\" /></a>\n";

    out << "<div id=\"imageCaption\">#" << (myid + 1) << " / " << numpeers
        << "</div>\n";

    //<hydraimg> tags
    out << "<hydrainfo id=\"" << myid << "\">\n";
    if (myid > 0) // this might emit a dupe from the delta loop, whatever for
                  // now i guess
        writeHydraImg(out, 0, *parent->subimages[0]);
    for (int delta = 1000000; delta >= 1; delta /= 10)
        if (IsOkId(myid - delta, numpeers))
            writeHydraImg(out, myid - delta, *parent->subimages[myid - delta]);
    writeHydraImg(out, myid, entry);
    for (int delta = 1000000; delta >= 1; delta /= 10)
        if (IsOkId(myid + delta, numpeers))
            writeHydraImg(out, myid + delta, *parent->subimages[myid + delta]);
    if (myid + 1 < numpeers) // this might emit a dupe from the delta loop,
                             // whatever for now i guess
        writeHydraImg(out, numpeers - 1, *parent->subimages[numpeers - 1]);
    writeHydraImg(out, randomId, *parent->subimages[randomId]);
    out << "</hydrainfo>\n";

    // the following script html tag is all for keyboard control
    out << R"EOF(<script>
let timerId = -1;
function getSlideShowValue() {
   return sessionStorage.getItem("slideShow") === "true";
}
function setSlideShowValue(value) {
   sessionStorage.setItem("slideShow", String(Boolean(value)));
}
function setSlideShowHandler(value) {
   if (timerId !== -1) {
      clearTimeout(timerId);
      timerId = -1;
   }
   if (value) {
      timerId = setTimeout(() => {
         handleEvent({ key: "Enter"});
      }, 8000);
   }
}
)EOF";
    out << "var handleEvent = function (event) {\n"
            "switch (event.key) {\n"
                "case \"ArrowLeft\":\n"
                "case \"k\":\n";
    if (myid > 0)
        out << "window.location = \"" << escapeForXML(parent->subimages[myid - 1]->urlhtml) << "\";\n";
    out <<
                    "break;\n";
    out <<
                "case \" \":\n"
                "case \"ArrowRight\":\n"
                "case \"Enter\":\n"
                "case \"j\":\n";
    if (myid + 1 < numpeers)
        out << "window.location = \"" << escapeForXML(parent->subimages[myid + 1]->urlhtml) << "\";\n";
    out <<
                    "break;\n";
    out <<
                "case \"Home\":\n";
    if (myid > 0)
        out << "window.location = \"" << escapeForXML(parent->subimages[0]->urlhtml) << "\";\n";
    out <<
                    "break;\n"
                "case \"End\":\n";
    if (myid + 1 < numpeers)
        out << "window.location = \"" <<  escapeForXML(parent->subimages[numpeers - 1]->urlhtml) << "\";\n";
    out <<
                    "break;\n"
                "case \"r\":\n"
                "case \"Tab\":\n";
    out << "window.location = \"" <<  escapeForXML(parent->subimages[randomId]->urlhtml) << "\";\n";
    out <<
                    "break;\n"
                "case \"d\":\n"
                "case \"f\":\n";
    out << "window.location = \"" << escapeForXML(entry.urlorigimage) << "\";\n";
    out << R"EOF(
break;
case "t":
window.open(window.location, "_blank", "", "");
break;
case "s":
setSlideShowValue(!getSlideShowValue());
setSlideShowHandler(getSlideShowValue());
break;
}//switch
}//handleEvent
setSlideShowHandler(getSlideShowValue());
document.addEventListener('keydown', (event) => { handleEvent(event); });
)EOF";

    // delay loader for full res image
    out << "setTimeout(() => {\n"
        "document.getElementById(\"mainImage\").src = \""
        <<  escapeForXML(entry.urlorigimage)
        << "\"; }, " << dm_configDelayImageLoadMS <<  ");\n";

    out << R"EOF(
</script>
</body>
</html>
)EOF";

    return true;
}

bool WebExport::writeDirIndexHtml(const QString &outfilename,
                                  const DirEntry &entry) {
    QFile outfile(outfilename);
    if (!outfile.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&outfile);

    out.setRealNumberPrecision(2);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    out << "<!DOCTYPE html>\n"
           "<html>\n"
           "<!-- hydraweb " HYDRA_VERSION_STRING " demko.ca -->\n"
           "<head>\n";

    out << "<title>" << escapeForXML(entry.isroot ? dm_title : entry.basedir)
        << "</title>\n"
           "<meta name=\"viewport\" content=\"width=device-width, "
           "initial-scale=1\" />\n"
           "<meta http-equiv=\"Content-Type\" content=\"text/html\" />\n"
           "<meta charset=\"utf-8\" />\n"
           "<link href=\""
        << StyleCSSFileName
        << "\" type=\"text/css\" rel=\"stylesheet\" />\n"
           "</head><body>\n";

    // nav bar
    // parent dir and sub dirs
    out << "<div id=\"topBar\" >\n"
           "<span id=\"leftBut\"><a class=\"navBut\" href=\"";
    if (entry.isroot) {
        out << "../\">&lt;&lt;Exit Gallery";
    } else {
        out << escapeForXML(entry.parent.lock()->urlname);
        out << "\">&lt;&lt;Up Album";
    }
    out << "</a></span>\n"
        << "<span class=\"albumTitle\">"
        << escapeForXML(entry.isroot ? dm_title : entry.basedir) << "</span>\n";
    out << "</div>\n\n";

    // sub directories
    if (!entry.subdirs.empty() &&
        !entry.subfiles.empty()) // display his heading only if we're displaying
                                 // the other one too
        out << "<div class=\"wideLinkListHeader\">Albums</div>\n";
    out << "<div id=\"albumSubDirList\" class=\"wideLinkList\">\n";
    for (DirSet::const_iterator ii = entry.subdirs.begin();
         ii != entry.subdirs.end(); ++ii)
        out << "<div><a href=\"" << escapeForXML((*ii)->urlname) << "\">"
            << escapeForXML((*ii)->justname) << "</a> (" << (*ii)->totalfiles
            << ")</div>\n";
    out << "</div>\n\n";

    // extra files
    if (!entry.subfiles.empty()) {
        out << "<div class=\"wideLinkListHeader\">Bonus Files</div>\n";
        out << "<div id=\"albumExtraFiles\" class=\"wideLinkList\">\n";
        for (FileSet::const_iterator ii = entry.subfiles.begin();
             ii != entry.subfiles.end(); ++ii)
            out << "<div><a href=\"" << (*ii)->urlorigimage << "\">"
                << (*ii)->justname << "</a> ("
                << ((*ii)->original_file_size / 1000000.0) << "MB)</div>\n";
        out << "</div>\n\n";
    }
    // images
    out << "<div id=\"imageIndex\">\n";
    for (FileSet::const_iterator ii = entry.subimages.begin();
         ii != entry.subimages.end(); ++ii)
        out << "<a href=\"" << escapeForXML((*ii)->urlhtml)
            << "\"><img border=\"0\" src=\""
            << escapeForXML((*ii)->urlthumbimage) << "\" /></a>\n";
    out << "</div>\n";

    //<hydraimg> tags
    out << "<hydrainfo>\n";
    if (entry.subimages.size() > 0)
        writeHydraImg(out, 0, *entry.subimages[0]);
    if (entry.subimages.size() > 1)
        writeHydraImg(out, entry.subimages.size() - 1,
                      *entry.subimages[entry.subimages.size() - 1]);
    out << "</hydrainfo>\n";

    out << "</body>\n";

    out << "</html>\n\n";

    return true;
}

void WebExport::writeAllImageHtmls(DirEntry &entry) {
    int i = 0;

    std::vector<int> random_ids(entry.subimages.size());

    for (unsigned i = 0; i < random_ids.size(); ++i)
        random_ids[i] = i;
    auto rng = QRandomGenerator::securelySeeded();
    std::shuffle(random_ids.begin(), random_ids.end(), rng);

    for (FileSet::const_iterator ii = entry.subimages.begin();
         ii != entry.subimages.end(); ++ii) {
        QString outfilename(dm_outdir + "/" + (*ii)->urlhtml);

        if (writeImageHtml(outfilename, i, random_ids[i],
                           static_cast<int>(entry.subimages.size()), *(*ii)))
            dm_out << "  VIEW " << outfilename << Qt::endl;
        else
            dm_out << "!ERROR failed to open for output: " << outfilename
                   << Qt::endl;
        ++i;
    }
}

void WebExport::fileCopyMakeDirs(const QString dir) {
    if (dir != "") {
        // qDebug() << "mkdir(" << (dm_outdir + "/" + dir) << ")";
        QDir(dm_outdir).mkdir(dir);
    }

    // recurse
    DirSet &subdirs = dm_basedirs[dir]->subdirs;

    for (DirSet::const_iterator ii = subdirs.begin(); ii != subdirs.end(); ++ii)
        fileCopyMakeDirs((*ii)->basedir);
}

int WebExport::fileCopyCopyFiles(void) {
    int count = 0;

    for (FileMap::const_iterator ii = dm_basefiles.begin();
         ii != dm_basefiles.end(); ++ii) {
        const FileEntry &entry = *(ii->second);

        // qDebug() << "cp" << entry.fullfilename << (dm_outdir + "/" +
        // entry.basefilename);
        // TODO justname might not be unique for that give directory
        // figure out something, but without the _r codes?
        smartCopy(entry.fullfilename,
                  dm_outdir + "/" + entry.basedir + "/" + entry.justname);
        ++count;
    }

    return count;
}

bool WebExport::DirEntryLT(const std::shared_ptr<DirEntry> &lhs,
                           const std::shared_ptr<DirEntry> &rhs) {
    return lhs->justname < rhs->justname;
}

bool WebExport::FileEntryLT(const std::shared_ptr<FileEntry> &lhs,
                            const std::shared_ptr<FileEntry> &rhs) {
    return lhs->basejustname < rhs->basejustname;
}
