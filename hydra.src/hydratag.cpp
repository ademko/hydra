
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <QCoreApplication>

#include <QDebug>
#include <QTextStream>

#include <hydra/ArgumentParser.h>
#include <hydra/CommandImport.h>
#include <hydra/Engine.h>
#include <hydra/FileIterator.h>
#include <hydra/FileOp.h>
#include <hydra/Hash.h>
#include <hydra/Query.h>
#include <hydra/Records.h>
#include <hydra/RotateCode.h>
#include <hydra/XMLUtil.h>

using namespace hydra;

QTextStream & operator<<(QTextStream &out, QUuid uuid) {
    out << uuid.toString();
    return out;
}

/*QString clipUuidBraces(const QUuid &uuid)
{
  QString s(uuid);

  return s.mid(1, s.size()-2);
}*/

/**
 * Split s by spaces and put the seperated words into out.
 *
 * @author Aleksander Demko
 */
static void parseOutTags(const QString &s, std::set<QString> &out) {
    QStringList list(s.split(' ', Qt::SkipEmptyParts));

    for (QStringList::const_iterator ii = list.begin(); ii != list.end(); ++ii)
        out.insert(*ii);
}

static void showHelp(QTextStream &out) {
    out << "hydratag -- command line tool -- version " HYDRA_VERSION_STRING
           " (" __DATE__ " " __TIME__ ")\n"
           "A suite of applications and libraries for noninvasive image and "
           "file tagging\n"
           "Copyright (c) " HYDRA_COPYRIGHT_STRING ", Aleksander B. Demko\n\n"
           "Usage:\n"
           "  hydratag command [command options]\n\n"
           "Where command can be one of:\n"
           "  add        adds files to the tag database\n"
           "               -a                  add all files, not just images\n"
           "               -u                  just update/freshen files "
           "already in the db\n"
           "               -t tag --tag=tag    also add the given tag for the "
           "files\n"
           "  list       lists the database information for the given files\n"
           "               -q --query=exp      filter by the given tag query "
           "string\n"
           "               -a                  show all files, even those not "
           "in db\n"
           "               -f                  format: list only files\n"
           "               -x                  format: xml output format\n"
           "               -0                  format: use null strings, "
           "suitable for xargs -0\n"
           "  tag        tags the given files\n"
           "               -q --query=exp      filter by the given tag query "
           "string\n"
           "               -c --clear          clear existing tags\n"
           "               -t tag --tag=tag    add tag\n"
           "               -r tag --rm=rag     remove tag\n"
           "  export     export the complete database to the given xml file\n"
           "  import     import database information from the given xml file\n"
           "               -m do a time-based merge (overwrites occur only\n"
           "                  if the new record is newer than existing)\n"
           "  test       tests the contents of the given directories\n"
           "               -v be verbose during processing\n"
           "                  A test involves finding all image files and "
           "hashing them,\n"
           "                  then verifying that that hash is in the DB.\n"
           "                  This function can be used to thoroughly check "
           "backups.\n"
           "  dupe       finds and notes all the dupes in the give "
           "directories/files\n"
           "               -a show all files, not just those that are dupes\n"
           "               -h show only those files that have multiple hashes\n"
           "                  (this will not show those files that just have "
           "path dupes)\n"
           "               -D DELETEs hash duplicate files. This will only "
           "keep one file\n"
           "                  of every unique hash\n"
           "  touch      rehashs all the given files. this is identical to a "
           "\"forget\"\n"
           "             followed by an \"add\" (the add will just rehash the "
           "files)\n"
           "  forget     forgets the path associations with the given "
           "directories/files\n"
           "             NOTE! forget is usually a developer function and "
           "usually should\n"
           "             not be needed by regular users.\n"
           "               -h forgets the hash association too (NOTE! this "
           "will untie\n"
           "                  the files with their current item records.)\n"
           "                  Upon readdition, the files will have new, blank "
           "item records.\n"
           "\n"
           "Typical tag styles:\n"
           "  lettersnumbers   basic tag\n"
           "  $ $$$ $$$$$      scoring tags\n"
           "  key=value        key value tag\n"
           "     rotate=90     rotate override tag\n"
           "  path/path/path   path tags, used for creating web sites\n"
           "\n"
           "Queries are a string of postfix operators:\n"
           "  TAG        true if contains tag TAG\n"
           "  TAG*       true if contains tag that starts with TAG (prefix "
           "match)\n"
           "  EXP not    inverse of EXP\n"
           "  L R and    true if both L and R are true\n"
           "  L R or     true if either L and/or R are true\n"
           "  L R xor    true if either L or R are true (but not both)\n"
           "  empty      true if the file contains no tags\n"
           "  all        always returns true\n"
        << Qt::endl;
}

static void showError(QTextStream &out, ArgumentParser::Exception &e) {
    out << "error: " << e.what() << Qt::endl;
}

static void commandAdd(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;
    bool all_files = false;
    bool just_update = false;
    std::set<QString> add_tags;
    FileItemRecord rec;
    QDateTime now(QDateTime::currentDateTime());

    // parse the command line
    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (cmd == "-a")
            all_files = true;
        else if (cmd == "-u")
            just_update = true;
        else if (cmd == "-t" || cmd == "--tag")
            parseOutTags(parser.nextParam("--tag"), add_tags);
        else if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        else
            fileList.push_back(cmd);
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    // perform the add
    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());

            if (I.isTraversing()) {
                // only do these checks if this file is the result
                // of a directory traversal
                if (!all_files && !isImageFile(justname))
                    continue;
            }

            if (just_update) {
                int getcode = Engine::instance()->getFileItem(fullcur, 0, 0, 0);

                if (getcode != Engine::Load_ErrorNeedsUpdate)
                    continue; // try the next file
                              // otherwise, fall through and freshes this file
            }

            int addcode = Engine::instance()->addFile(fullcur);

            out << Engine::codeToChar(addcode) << ' ' << fullcur << Qt::endl;

            if (addcode != Engine::Add_Error) {
                int code = Engine::instance()->getFileItem(fullcur, rec);
                if (code == Engine::Load_OK) {
                    bool did_something = false;
                    for (std::set<QString>::const_iterator ii =
                             add_tags.begin();
                         ii != add_tags.end(); ++ii)
                        did_something =
                            rec.tags.insertTag(*ii) || did_something;
                    if (did_something)
                        Engine::instance()->saveFileItem(rec, now);
                }
            }
        } // while
    }     // for fileList
}

static void commandList(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;
    FilePathRecord path;
    FileHashRecord hash;
    FileItemRecord item;
    bool list_all = false;
    bool format_file = false;
    bool format_xml = false;
    bool format_null = false;
    std::shared_ptr<Token> query_token;

    parseQueryTokens("", query_token);

    // parse the command line
    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (cmd == "-q" || cmd == "--query") {
            if (!parseQueryTokens(parser.nextParam("--query"), query_token))
                throw ArgumentParser::ErrorException("Query parse error");
        } else if (cmd == "-a")
            list_all = true;
        else if (cmd == "-f")
            format_file = true;
        else if (cmd == "-x")
            format_xml = true;
        else if (cmd == "-0")
            format_null = true;
        else if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        else
            fileList.push_back(cmd);
    }

    if (fileList.isEmpty())
        fileList.push_back(".");

    // perform the command
    if (format_xml)
        out << "<hydradb>" << Qt::endl;
    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());

            int code =
                Engine::instance()->getFileItem(fullcur, &item, &hash, &path);

            if (list_all && code == Engine::Load_ErrorFatal)
                continue;
            if (!list_all && (code == Engine::Load_ErrorFatal ||
                              code == Engine::Load_ErrorNotFound))
                continue;

            if (!query_token->isMatch(item.tags))
                continue;

            // -f will take priority over -x, i guess
            if (format_file || format_null) {
                out << fullcur;
                if (format_file)
                    out << Qt::endl;
                else
                    out << '\0';
                continue;
            }

            if (format_xml) {
                if (code != Engine::Load_OK)
                    continue; // xml format cannot do files that are not in the
                              // db

                out << "<path filename=\"" << escapeForXML(fullcur)
                    << "\" hash=\"" << path.hash << "\" />" << Qt::endl;
                out << "<hash hash=\"" << path.hash << "\" id=\"" << hash.id
                    << "\" />" << Qt::endl;
                out << "<item id=\"" << hash.id << "\" title=\""
                    << escapeForXML(item.title) << "\" desc=\""
                    << escapeForXML(item.desc) << "\" filetype=\""
                    << item.filetype << "\" modtime=\"" << item.modtimeAsInt64()
                    << "\" modtime_iso_local=\""
                    << item.modtime.toString(Qt::ISODate)
                    << "\" modtime_iso_utc=\""
                    << item.modtime.toUTC().toString(Qt::ISODate) << "\" >";
                for (FileItemRecord::tags_t::iterator ii = item.tags.begin();
                     ii != item.tags.end(); ++ii)
                    out << "<tag title=\"" << escapeForXML(*ii) << "\" />";
                out << "</item>" << Qt::endl;
                continue;
            } // format_xml

            // normal format continues

            out << Engine::codeToChar(code) << ' ' << fullcur;

            if (code == Engine::Load_ErrorNeedsUpdate) {
                out << " !NEEDS_UPDATE" << Qt::endl;
                continue;
            }

            if (code != Engine::Load_OK) {
                out << Qt::endl;
                continue;
            }

            out << " filetype=" << item.filetype << " rotatecode="
                << detectMultiRotate(fullcur, item.tags) // rotate code
                << " {";
            for (FileItemRecord::tags_t::iterator ii = item.tags.begin();
                 ii != item.tags.end(); ++ii)
                out << ' ' << *ii;
            out << " }" << Qt::endl;
        } // while
    }     // for fileList
    if (format_xml)
        out << "</hydradb>" << Qt::endl;
}

static void commandTag(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;
    bool do_clear = false;
    std::set<QString> add_tags, rm_tags;
    std::shared_ptr<Token> query_token;
    FileItemRecord rec;
    QDateTime now(QDateTime::currentDateTime());

    parseQueryTokens("", query_token);

    // parse the command line
    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (cmd == "-q" || cmd == "--query") {
            if (!parseQueryTokens(parser.nextParam("--query"), query_token))
                throw ArgumentParser::ErrorException("Query parse error");
        } else if (cmd == "-c" || cmd == "--clear")
            do_clear = true;
        else if (cmd == "-t" || cmd == "--tag")
            parseOutTags(parser.nextParam("--tag"), add_tags);
        else if (cmd == "-r" || cmd == "--rm")
            parseOutTags(parser.nextParam("--rm"), rm_tags);
        else if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        else
            fileList.push_back(cmd);
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    // perform the command
    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());
            int code = Engine::instance()->getFileItem(fullcur, rec);
            bool did_something = false;

            if (code != Engine::Load_OK)
                continue;

            if (!query_token->isMatch(rec.tags))
                continue;

            if (do_clear)
                did_something = rec.tags.clearTags() || did_something;

            for (std::set<QString>::const_iterator ii = add_tags.begin();
                 ii != add_tags.end(); ++ii)
                did_something = rec.tags.insertTag(*ii) || did_something;

            for (std::set<QString>::const_iterator ii = rm_tags.begin();
                 ii != rm_tags.end(); ++ii)
                did_something = rec.tags.eraseTag(*ii) || did_something;

            out << (did_something ? "T " : "  ") << fullcur << Qt::endl;

            if (did_something)
                Engine::instance()->saveFileItem(rec, now);
        }
    }
}

static void commandExport(QTextStream &out, ArgumentParser &parser) {
    QString filename;
    Engine *eng = Engine::instance();

    filename = parser.nextParam("export xml file filename");

    // do the export
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
        throw ArgumentParser::ErrorException("can't open file for writing: " +
                                             filename);

    out << "writing to xml file: " << filename << Qt::endl;

    QTextStream outfile(&file);

    outfile << "<hydradb>" << Qt::endl;
    {
        FilePathRecord path;
        Cursor cur(eng->filePathDB());

        while (cur.next()) {
            if (!cur.get(path))
                continue;

            outfile << "<path filename=\"" << escapeForXML(cur.getKey())
                    << "\" hash=\"" << path.hash << "\" />" << Qt::endl;
        } // while
    }
    {
        FileHashRecord hash;
        Cursor cur(eng->fileHashDB());

        while (cur.next()) {
            if (!cur.get(hash))
                continue;

            outfile << "<hash hash=\"" << cur.getKey() << "\" id=\"" << hash.id
                    << "\" />" << Qt::endl;
        } // while
    }
    {
        FileItemRecord item;
        Cursor cur(eng->fileItemDB());

        while (cur.next()) {
            if (!cur.get(item))
                continue;

            outfile << "<item id=\"" << cur.getKey() << "\" title=\""
                    << escapeForXML(item.title) << "\" desc=\""
                    << escapeForXML(item.desc) << "\" filetype=\""
                    << item.filetype << "\" modtime=\"" << item.modtimeAsInt64()
                    << "\" modtime_iso_local=\""
                    << item.modtime.toString(Qt::ISODate)
                    << "\" modtime_iso_utc=\""
                    << item.modtime.toUTC().toString(Qt::ISODate) << "\" >";
            for (FileItemRecord::tags_t::iterator ii = item.tags.begin();
                 ii != item.tags.end(); ++ii)
                outfile << "<tag title=\"" << escapeForXML(*ii) << "\" />";
            outfile << "</item>" << Qt::endl;
        } // while
    }
    outfile << "</hydradb>" << Qt::endl;
}

static void commandImport(QTextStream &out, ArgumentParser &parser) {
    bool smartmerge = false;
    QString filename;

    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (cmd == "-m")
            smartmerge = true;
        else if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        else
            filename = cmd;
    }

    if (filename.isEmpty())
        throw ArgumentParser::ErrorException("import filename required");

    out << "processing xml file: " << filename << Qt::endl;

    int count = commandImport(filename, smartmerge);

    if (count == -1)
        throw ArgumentParser::ErrorException("failed to import data from: " +
                                             filename);

    out << count << " (new) file items added or merged" << Qt::endl;
}

static void commandTest(QTextStream &out, ArgumentParser &parser) {
    bool verbose = false;
    QStringList fileList;

    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (cmd == "-v")
            verbose = true;
        else if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        else
            fileList.push_back(cmd);
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    // start the test

    bool curok;
    int okcount = 0;
    typedef std::list<QString> failed_t;
    failed_t failed;

    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());

            if (isNormalFile(justname) && isImageFile(justname)) {
                curok = true;
                if (verbose)
                    out << fullcur << Qt::endl;

                // hash the file
                QString filehash = calcFileHash(fullcur);

                if (filehash.isEmpty())
                    curok = false;

                // check if it's in the DB
                if (curok)
                    if (Engine::instance()->getFileItemByHash(filehash, 0, 0) !=
                        Engine::Load_OK)
                        curok = false;

                if (curok)
                    ++okcount;
                else
                    failed.push_back(fullcur);
            } // if
        }     // while
    }         // for

    if (failed.empty())
        out << okcount << " files all OK" << Qt::endl;
    else {
        out << "Files not in DB:" << Qt::endl;
        for (failed_t::const_iterator ii = failed.begin(); ii != failed.end();
             ++ii)
            out << *ii << Qt::endl;
        out << failed.size() << " failed files listed" << Qt::endl;
    }
}

static void commandDupe(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;
    typedef std::set<QString> stringset_t;
    typedef std::map<QString, stringset_t> hashpathmap_t;
    typedef std::map<QUuid, stringset_t> idhashmap_t;
    typedef std::map<QUuid, std::shared_ptr<FileItemRecord>> itemmap_t;
    hashpathmap_t hashpath;
    idhashmap_t idhash;
    itemmap_t itemmap;
    bool show_all = false;
    bool show_hash_only = false;
    bool delete_dupes = false;

    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (!isswitch)
            fileList.push_back(cmd);
        if (isswitch) {
            if (cmd == "-a")
                show_all = true;
            else if (cmd == "-h")
                show_hash_only = true;
            else if (cmd == "-D")
                delete_dupes = true;
            else
                throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        }
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());
            FileItemRecord item;
            FileHashRecord hash;
            FilePathRecord path;

            int code =
                Engine::instance()->getFileItem(fullcur, &item, &hash, &path);

            if (code != Engine::Load_OK)
                continue;

            // out << " loading " << fullcur << '\n';

            // add the item if need be
            if (itemmap.count(item.id) == 0)
                itemmap[item.id] =
                    std::shared_ptr<FileItemRecord>(new FileItemRecord(item));

            // add this hash to the id
            idhash[item.id].insert(path.hash);

            // add the path to the hash
            hashpath[path.hash].insert(fullcur);
        } // while I
    }     // for ii

    // now, go through the structures a print out all the dupes
    int count = 0;
    int total = 0;
    int delcount = 0;

    for (idhashmap_t::iterator ii = idhash.begin(); ii != idhash.end(); ++ii) {
        bool printed_item = false;

        // see if we need to print this item
        if (show_all)
            printed_item = true;
        else if (ii->second.size() > 1)
            printed_item = true;
        else if (!show_hash_only)
            for (stringset_t::iterator hh = ii->second.begin();
                 hh != ii->second.end(); ++hh)
                if (hashpath[*hh].size() > 1) {
                    printed_item = true;
                    break;
                }

        total++;

        if (printed_item) {
            count++;
            out << ii->first << '\n';

            for (stringset_t::iterator hh = ii->second.begin();
                 hh != ii->second.end(); ++hh) {
                // out << "  " << *hh << '\n';

                stringset_t &S = hashpath[*hh];
                bool nuke_this = false;
                for (stringset_t::iterator pp = S.begin(); pp != S.end();
                     ++pp) {
                    // out << "    " << *pp << '\n';
                    if (nuke_this) {
                        out << " D  " << *hh << "    " << *pp << '\n';
                        delFile(*pp);
                        delcount++;
                    } else
                        out << "    " << *hh << "    " << *pp << '\n';

                    nuke_this = delete_dupes;
                } // for pp
            }     // for hh
        }         // if printed_item
    }             // for ii

    out << count << "/" << total << " duplicates found.\n";

    if (delete_dupes)
        out << delcount << " duplicates deleted.\n";
}

static void commandTouch(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;

    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (!isswitch)
            fileList.push_back(cmd);
        if (isswitch)
            throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());
            bool did_something = false;
            FilePathRecord path;

            int code = Engine::instance()->getFileItem(fullcur, 0, 0, &path);

            if (code != Engine::Load_OK)
                continue;

            did_something =
                Engine::instance()->erasePath(fullcur) || did_something;

            if (!did_something)
                continue;

            int addcode = Engine::instance()->addFile(fullcur);

            out << Engine::codeToChar(addcode) << ' ' << fullcur << Qt::endl;
        } // while I
    }     // for ii
}

static void commandForget(QTextStream &out, ArgumentParser &parser) {
    QStringList fileList;
    bool de_hash = false;

    while (parser.hasNext()) {
        bool isswitch;
        QString cmd(parser.next(&isswitch));

        if (!isswitch)
            fileList.push_back(cmd);
        if (isswitch) {
            if (cmd == "-h")
                de_hash = true;
            else
                throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
        }
    }

    if (fileList.isEmpty())
        throw ArgumentParser::ErrorException(
            "Specify one or more files or directories");

    for (QStringList::const_iterator ii = fileList.begin();
         ii != fileList.end(); ++ii) {
        FileIterator I(*ii);

        while (I.hasNext()) {
            QString fullcur(I.next());
            QString justname(QFileInfo(fullcur).fileName());
            bool did_something = false;
            FilePathRecord path;

            int code = Engine::instance()->getFileItem(fullcur, 0, 0, &path);

            if (code != Engine::Load_OK)
                continue;

            did_something =
                Engine::instance()->erasePath(fullcur) || did_something;
            if (de_hash)
                did_something =
                    Engine::instance()->eraseHash(path.hash) || did_something;

            out << (did_something ? "F " : "  ") << fullcur << Qt::endl;
        } // while I
    }     // for ii
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    ArgumentParser parser;
    QTextStream out(stdout);

    parser.next(); // eat the program name
    if (!parser.hasNext()) {
        showHelp(out);
        return 0;
    }

    try {
        Engine eng;
        QString cmd(parser.next());

        if (cmd == "add")
            commandAdd(out, parser);
        else if (cmd == "list")
            commandList(out, parser);
        else if (cmd == "tag")
            commandTag(out, parser);
        else if (cmd == "export")
            commandExport(out, parser);
        else if (cmd == "import")
            commandImport(out, parser);
        else if (cmd == "test")
            commandTest(out, parser);
        else if (cmd == "dupe")
            commandDupe(out, parser);
        else if (cmd == "touch")
            commandTouch(out, parser);
        else if (cmd == "forget")
            commandForget(out, parser);
        else
            throw ArgumentParser::ErrorException("Unknown command: " + cmd);
    } catch (ArgumentParser::HelpException &) {
        showHelp(out);
        return 0;
    } catch (ArgumentParser::Exception &e) {
        showError(out, e);
        return 1;
    }

    return 0;
}
