
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/WebExport.h>

#include <assert.h>

#include <iostream>
#include <algorithm>

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <hydra/Engine.h>
#include <hydra/XMLUtil.h>
#include <hydra/Records.h>
#include <hydra/XMLUtil.h>
#include <hydra/Thumb.h>
#include <hydra/FileOp.h>
#include <hydra/RotateCode.h>

using namespace hydra;

//
//
// DirEntry
//
//

WebExport::DirEntry::DirEntry(const QString &_basedir)
  : isroot(false), totalfiles(-1), basedir(_basedir)
{
  if (basedir.isEmpty())
    urlname = "index";
  else {
    for (QString::const_iterator ii=basedir.begin(); ii != basedir.end(); ++ii)
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

void WebExport::FileEntry::calc(void)
{
  assert(!basefilename.isEmpty());
  assert(urlnamebase.isEmpty());

  for (QString::const_iterator ii=basefilename.begin(); ii != basefilename.end(); ++ii)
    if (ii == basefilename.begin() && *ii == '/')
      ; // do nothing
    else if (*ii == '/')
      urlnamebase.append(',');
    else
      urlnamebase.append(*ii);

  urlhtml = urlnamebase + ".html";
  //urlorigimage = "original_files/" + urlnamebase;
  urlorigimage = urlnamebase;
  urlviewimage = "VIEW," + urlnamebase;
  urlthumbimage = "THUMB," + urlnamebase;
}

//
//
// WebExport
//
//

WebExport::WebExport(const QString &outputdir, QTextStream &out)
  : dm_out(out), dm_outdir(outputdir), dm_title("Gallery")
{
  addDirComponents("");
}

void WebExport::setTitle(const QString &title)
{
  dm_title = title;
}

void WebExport::addFile(const QString &fullfilename, const QString &basedir,
        hydra::FileItemRecord &item, const QString &filehash)
{
  //dm_out << "adding full=" << fullfilename.toUtf8().constData() << " basedir=" << basedir.toUtf8().constData() << endl;

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
  //entry->basejustname = entry->justname;
  //entry->basefilename = entry->basedir + "/" + entry->basejustname;

  // insert until its not a dupe
  int dupeid = 1;
  do {
    entry->basejustname.clear();
    QTextStream str(&entry->basejustname);

    if (dupeid>1)
      str << dupeid++;

    QFileInfo info(entry->justname);
    str << info.baseName() << "_r" << rotateCodeToDegrees(entry->rotateCode) << "." + info.suffix();

    entry->basefilename = entry->basedir + "/" + entry->basejustname;

    dupeid++;
  } while (
      dm_basefiles.count(entry->basefilename) > 0 &&
      dm_basefiles[entry->basefilename]->id != item.id
      );

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

int WebExport::commitWebSite(void)
{
  //QDir(dm_outdir).mkdir("original_files");
  Thumb::mkThumbDir();
  buildDirTree();
  computeDirCounts(*dm_basedirs[""]);
  writeIndexFiles();
  return writeImageFiles();
}

int WebExport::commitFileCopy(void)
{
  buildDirTree();
  fileCopyMakeDirs();
  return fileCopyCopyFiles();
}

void WebExport::addDirComponents(const QString &dir)
{
  QString cur;

  // add the root component
  if (dm_basedirs.find(cur) == dm_basedirs.end())
    dm_basedirs[cur] = std::shared_ptr<DirEntry>(new DirEntry(cur));

  if (dir.isEmpty())
    return;   // end here, nothing more to do

  // now add all the dir compoenents
  QString::const_iterator ii=dir.begin();

  while (true) {
    if (ii == dir.end() || *ii == '/') {
      // found a component
      if (dm_basedirs.find(cur) == dm_basedirs.end())
        dm_basedirs[cur] = std::shared_ptr<DirEntry>(new DirEntry(cur));
      cur += '/';
    } else
      cur += *ii;
    if (ii == dir.end())  // are we done
      return;
    ii++;
  }
}

void WebExport::buildDirTree(void)
{
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
    int bestn = -1;  // "" has 0 slashes, while "blah/blah" has two (one extre virtual one, so that "blah" and "" dont end up havin the same count
    int curn, myn;

    // first, count how many slashes in this node
    myn = 1;//start from 1, so not to override the root node
    for (QString::const_iterator uu=ii->first.begin(); uu!=ii->first.end(); ++uu)
      if (*uu == '/')
        ++myn;

    for (kk = dm_basedirs.begin(); kk != dm_basedirs.end(); ++kk) {
      if (ii == kk)
        continue; // you cant be your own parent
      if (kk->first.isEmpty())
        continue; // no need to compare against the root
      // next, do subset comparison
      if (kk->first.size() >= ii->first.size())
        continue;   // subset must be shorter than current
      if (ii->first.leftRef(kk->first.size()) != kk->first)
        continue;   // not a subset match
      // count the / in this potential new parent
      curn = 1;//start from 1, so not to override the root node
      for (QString::const_iterator uu=kk->first.begin(); uu!=kk->first.end(); ++uu)
        if (*uu == '/')
          ++curn;
      // finally, is this a new winner?
      if (curn > bestn && curn < myn) {
        bestn = curn;
        bestsofar = kk->first;
      }
    }//kk

    // new parent found, add it
    dm_basedirs[bestsofar]->subdirs.push_back(ii->second);
    ii->second->parent = dm_basedirs[bestsofar];
  }//for
}

void WebExport::computeDirCounts(DirEntry &dir)
{
  dir.totalfiles = 0;

  for (DirSet::iterator ii=dir.subdirs.begin(); ii != dir.subdirs.end(); ++ii) {
    computeDirCounts(*(*ii));
    dir.totalfiles += (*ii)->totalfiles;
  }
  dir.totalfiles += dir.subimages.size();
  dir.totalfiles += dir.subfiles.size();
}

int WebExport::writeImageFiles(void)
{
  int code;
  int count = 0;
  QString outfile, outthumb;

  for (FileMap::iterator ii=dm_basefiles.begin(); ii != dm_basefiles.end(); ++ii) {
    FileEntry &entry = *(ii->second);

    outfile = dm_outdir + "/" + entry.urlorigimage;
    if (!QFileInfo(outfile).exists()) {
      dm_out << "  ORIGINAL " << entry.fullfilename << endl;

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
      outthumb = Thumb::fileName(entry.filehash, entry.rotateCode, Thumb::DEFAULT_THUMB_W, Thumb::DEFAULT_THUMB_H);

      dm_out << "  THUMB " << entry.fullfilename << endl;

      code = T.generate(outthumb, 0, entry.rotateCode, Thumb::DEFAULT_THUMB_W, Thumb::DEFAULT_THUMB_H);
      if (code != Thumb::Generate_Ok && code != Thumb::Generate_FileExists) {
        dm_out << "!ERROR " << outthumb << endl;
        continue;
      }
      smartCopy(outthumb, outfile);
    }//if

    outfile = dm_outdir + "/" + entry.urlviewimage;
    if (!QFileInfo(outfile).exists()) {
      outthumb = Thumb::fileName(entry.filehash, entry.rotateCode, Thumb::DEFAULT_VIEW_W, Thumb::DEFAULT_VIEW_H);

      dm_out << "  VIEWTHUMB " << entry.fullfilename << endl;

      code = T.generate(outthumb, 0, entry.rotateCode, Thumb::DEFAULT_VIEW_W, Thumb::DEFAULT_VIEW_H);
      if (code != Thumb::Generate_Ok && code != Thumb::Generate_FileExists) {
        dm_out << "!ERROR " << outthumb << endl;
        continue;
      }
      smartCopy(outthumb, outfile);
    }//if
  }//big for

  return count;
}

bool WebExport::writeFileIndex(int myid, int numpeers, FileEntry &entry)
{
  QString outfilename(dm_outdir + "/" + entry.urlhtml);
  QFile outfile(outfilename);

  if (!outfile.open(QIODevice::WriteOnly)) {
    dm_out << "!ERROR " << outfilename << endl;
    return false;
  }

  QTextStream out(&outfile);

  std::shared_ptr<DirEntry> parent(entry.parent);

  assert(parent.get());

  out <<
    "<html>\n"
    "<!-- hydraweb " HYDRA_VERSION_STRING " demko.ca -->\n"
    "<head>\n"
    "<title>#" << (myid+1) << '/' << numpeers << ' ' << escapeForXML(entry.justname) << "</title>\n"
    "</head><body>\n"
    "<center><table border=\"0\"><tr><td align=\"center\" colspan=\"3\">\n";
  if (myid + 1 < numpeers)
    out << "<a href=\"" << escapeForXML(parent->subimages[myid+1]->urlhtml) << "\">";
  out << "<img border=\"0\" src=\"" << escapeForXML(entry.urlviewimage) << "\" />";
  if (myid + 1 < numpeers)
    out << "</a>\n";
  out << "</td></tr><tr><th align=\"left\" width=\"33%\">";
  if (myid>0)
    out << "<a href=\"" << escapeForXML(parent->subimages[myid-1]->urlhtml) << "\">&lt;&lt; Previous image</a>";
  out << "</th><th align=\"center\" width=\"34%\">"
    "<a href=\"" << escapeForXML(parent->urlname) << "\">Return to album</a></th>"
    "<th align=\"right\" width=\"33%\">";
  if (myid + 1 < numpeers)
    out << "<a href=\"" << escapeForXML(parent->subimages[myid+1]->urlhtml) << "\">Next image &gt;&gt;</a>";
  out << "</th></tr><tr><td align=\"center\" colspan=\"3\">"
    "<a href=\"" << escapeForXML(entry.urlorigimage) << "\">Download the high-quality, original file</a>"
    "</td></tr></table></center></body></html>";

  dm_out << "  VIEW " << outfilename << endl;

  return false;
}

bool WebExport::writeDirIndex(DirEntry &entry)
{
  QString outfilename(dm_outdir + "/" + entry.urlname);
  QFile outfile(outfilename);
  if (!outfile.open(QIODevice::WriteOnly)) {
    dm_out << "!ERROR " << outfilename << endl;
    return false;
  }
  QTextStream out(&outfile);

  out <<
    "<html>\n"
    "<!-- hydraweb " HYDRA_VERSION_STRING " demko.ca -->\n"
    "<head>\n"
    "<title>" << escapeForXML(entry.isroot ? dm_title : entry.basedir) << "</title>\n"
    "</head>\n"
    "<body><h2>" << escapeForXML(entry.isroot ? dm_title : entry.basedir) << "</h2>\n"
    "<font size=\"+1\"><pre>\n";

  // parent dir and sub dirs
  if (entry.isroot) {
    out << "<i><a href=\"../\">&lt;&lt;Exit Gallery</a></i>\n\n";
  } else {
    out << "<i><a href=\"";
    std::shared_ptr<DirEntry> parent(entry.parent);
    out << escapeForXML(parent->urlname);
    out << "\">&lt;&lt;Go Back</a></i>\n\n";
  }

  // sort by name for display
  std::sort(entry.subdirs.begin(), entry.subdirs.end(), DirEntryLT);
  // sub directories
  for (DirSet::const_iterator ii=entry.subdirs.begin(); ii != entry.subdirs.end(); ++ii)
    out << "  <a href=\"" << escapeForXML((*ii)->urlname) << "\">"
      << escapeForXML((*ii)->justname) << "</a> ("
      << (*ii)->totalfiles << ")\n";
  out << 
    "</pre></font>\n";

  // sort by name for display
  std::sort(entry.subfiles.begin(), entry.subfiles.end(), FileEntryLT);
  // extra files
  if (!entry.subfiles.empty()) {
    out << "<pre>\n  Extra files:\n";
    for (FileSet::const_iterator ii=entry.subfiles.begin(); ii != entry.subfiles.end(); ++ii)
      out << "    <a href=\"" << (*ii)->urlorigimage << "\">" << (*ii)->justname << "</a>\n";
    out << "</pre><br/>\n";
  }

  // sort by name for display
  std::sort(entry.subimages.begin(), entry.subimages.end(), FileEntryLT);
  // images
  for (FileSet::const_iterator ii=entry.subimages.begin(); ii != entry.subimages.end(); ++ii)
    out <<
      "<a href=\"" << escapeForXML((*ii)->urlhtml) << "\"><img border=\"0\" src=\""
      << escapeForXML((*ii)->urlthumbimage) << "\" /></a>\n";

  out << "</body></html>\n\n";

  dm_out << "INDEX " << outfilename << endl;

  int i=0; 
  for (FileSet::const_iterator ii=entry.subimages.begin(); ii != entry.subimages.end(); ++ii) {
    writeFileIndex(i, static_cast<int>(entry.subimages.size()), *(*ii));
    ++i;
  }

  return true;
}

void WebExport::writeIndexFiles(void)
{
  for (DirMap::const_iterator ii=dm_basedirs.begin(); ii != dm_basedirs.end(); ++ii)
    writeDirIndex(*(ii->second));
}

void WebExport::fileCopyMakeDirs(const QString dir)
{
  if (dir != "") {
    //qDebug() << "mkdir(" << (dm_outdir + "/" + dir) << ")";
    QDir(dm_outdir).mkdir(dir);
  }

  // recurse
  DirSet &subdirs = dm_basedirs[dir]->subdirs;

  for (DirSet::const_iterator ii=subdirs.begin(); ii != subdirs.end(); ++ii)
    fileCopyMakeDirs((*ii)->basedir);
}

int WebExport::fileCopyCopyFiles(void)
{
  int count = 0;

  for (FileMap::const_iterator ii=dm_basefiles.begin(); ii != dm_basefiles.end(); ++ii) {
    const FileEntry &entry = *(ii->second);

    //qDebug() << "cp" << entry.fullfilename << (dm_outdir + "/" + entry.basefilename);
    //TODO justname might not be unique for that give directory
    // figure out something, but without the _r codes?
    smartCopy(entry.fullfilename, dm_outdir + "/" + entry.basedir + "/" + entry.justname);
    ++count;
  }

  return count;
}

bool WebExport::DirEntryLT(const std::shared_ptr<DirEntry> &lhs,
        const std::shared_ptr<DirEntry> &rhs)
{
  return lhs->justname < rhs->justname;
}

bool WebExport::FileEntryLT(const std::shared_ptr<FileEntry> &lhs,
        const std::shared_ptr<FileEntry> &rhs)
{
  return lhs->basejustname < rhs->basejustname;
}

