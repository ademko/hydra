
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <QCoreApplication>

#include <assert.h>

#include <QTextStream>
#include <QDebug>

#include <hydra/ArgumentParser.h>
#include <hydra/FileIterator.h>
#include <hydra/Engine.h>
#include <hydra/Records.h>
#include <hydra/Query.h>

#include <hydra/WebExport.h>

using namespace hydra;

static void showHelp(QTextStream &out)
{
  out <<
    "hydraweb -- command line website maker -- version " HYDRA_VERSION_STRING " (" __DATE__ " " __TIME__ ")\n"
    "A suite of applications and libraries for noninvasive image and file tagging\n"
    "Copyright (c) " HYDRA_COPYRIGHT_STRING ", Aleksander B. Demko\n\n"
    "Usage:\n"
    "  hydraweb [options] commands_files_and_dirs\n\n"
    "Where options include:\n"
    "  -o outdir           REQUIRED. The output directory to write the website too\n"
    "  -t --title=desc     set the title variable to be desc\n"
    "  -f                  Build a file tree rather than a website\n"
    "  -v                  Verbose output/display progress\n"
    "\n"
    "Commands can include:\n"
    "  flat                just dump all the files into the single output directory (the default)\n"
    "  subdir              use the source files directory when putting files in the output directory\n"
    "  pivot PIVOT_TAG     use tag pivot placement, on the given tag (example blah/ to match blah/foo blah/bar etc)\n"
    "  query Q             use the given query to filter results (default=all)\n"
    "  webpath PATH        place the files in the given web path (default=emptystring)\n"
    "  file_or_dir         process the given file or directory set\n"
    "     the options can be repeaded and sequenced as needed\n"
    "\n";

}

static void showError(QTextStream &out, ArgumentParser::Exception &e)
{
  out << "error: " << e.what() << endl;
}

static QString prepPrefix(const QString &prefix)
{
  QString ret;

  if (QFileInfo(prefix).isDir())
    ret = makeAbsolute(prefix + "/"); // this elimates dangling .
  else
    ret = makeAbsolute("./");  // just use the current dir then, and ignore the path in the prefix

  assert(!ret.isEmpty());
  if (ret[ret.size()-1] != '/')
    ret.append('/');

  return ret;
}

static QString stripPrefix(const QString &val, const QString &prefix)
{
  return val.mid(prefix.size());
}

// lops off the ending filename of the given filepath
static QString JustPath(const QString &filepath)
{
  //qDebug() << "JustPath(" << filepath << "=" << QFileInfo(filepath).path();
  //return QFileInfo(filepath).path();
  QString ret(QFileInfo(filepath).path());

  if (ret == ".")
    ret.clear();
  return ret;
}

static QString AppendPath(const QString &base, const QString &suffix)
{
  if (base.isEmpty())
    return suffix;
  if (suffix.isEmpty())
    return base;
  if (base[base.size()-1] != '/' && suffix[suffix.size()-1] != '/')
    return base + "/" + suffix;
  return base + suffix;   // they already have a / in between em
}

// returns true on success
static bool PivotPath(const QString &pivot_tag, const FileItemRecord::tags_t &tags, QString &outpath)
{
  assert(!pivot_tag.isEmpty());

  // find the proper tag
  for (FileItemRecord::tags_t::const_iterator ii=tags.begin(); ii != tags.end(); ++ii)
    if ((*ii).leftRef(pivot_tag.size()) == pivot_tag) {
      // found the pivot tag, now extra the ending
      outpath = (*ii).mid(pivot_tag.size());
      return true;
    }

  return false;
}

static void commandWebExport(QTextStream &out, ArgumentParser &parser)
{
  std::vector<QString> ops;
  QString outputdir, title;
  bool build_file_tree = false;
  bool verbose_output = false;

  // parse the command line
  while (parser.hasNext()) {
    bool isswitch;
    QString cmd(parser.next(&isswitch));

    if (cmd == "-o")
      outputdir = parser.nextParam("-o");
    else if (cmd == "-t")
      title = parser.nextParam("-t");
    else if (cmd == "-f")
      build_file_tree = true;
    else if (cmd == "-v")
      verbose_output = true;
    else if (isswitch)
      throw ArgumentParser::ErrorException("Unknown switch: " + cmd);
    else
      ops.push_back(cmd);
  }

  if (outputdir.isEmpty())
    throw ArgumentParser::ErrorException("The output (-o) directory must be specified");

  if (!QFileInfo(outputdir).isDir())
    throw ArgumentParser::ErrorException(outputdir + " must be a directory");

  // parse the ops into the webexporter

  WebExport exporter(outputdir, out);
  QString fullcur, dir_prefix;
  int j, maxj;
  enum {
    flat_type,
    subdir_type,
    pivot_type,
  };
  short curtype = flat_type;
  QString pivot_tag, pivot_path, webpath;
  std::tr1::shared_ptr<hydra::Token> query_tok, pivot_tok;
  int scanned_count = 0;

  parseQueryTokens("", query_tok);   // init it

  if (!title.isEmpty())
    exporter.setTitle(title);

  maxj = static_cast<int>(ops.size());
  j = 0;
  while (j<maxj) {
    QString &param = ops[j];

    if (param == "flat") {
      curtype = flat_type;
    } else if (param == "subdir") {
      curtype = subdir_type;
    } else if (param == "pivot") {
      j++;
      if (j<maxj) {
        curtype = pivot_type;
        pivot_tag = ops[j];
        parseQueryTokens(pivot_tag + "*", pivot_tok);
      } else {
        out << "passing empty tag parameter to pivot subcommand!" << endl;
        curtype = flat_type;    // fail
      }
    } else if (param == "webpath") {
      j++;
      if (j<maxj)
        webpath = ops[j];
    } else if (param == "query") {
      j++;
      if (j<maxj)
        parseQueryTokens(ops[j], query_tok);
    } else {
      // must be a file param
      FileIterator ii(param);
      FileItemRecord item;
      FilePathRecord path;

      dir_prefix = prepPrefix(param);

      while (ii.hasNext()) {
        fullcur = ii.next();

        scanned_count++;
        if (verbose_output && scanned_count % 100 == 0)
          out << "Files scanned: " << scanned_count << endl;
        // load and check query
        if (Engine::instance()->getFileItem(fullcur, &item, 0, &path) != Engine::Load_OK)
          continue;
        if (!query_tok->isMatch(item.tags))
          continue;

        //std::cerr << "prefix=" << dir_prefix.utf8_str() << " cur=" << fullcur.utf8_str() << endl;
        switch (curtype) {
          case subdir_type:
            exporter.addFile(fullcur, AppendPath(webpath, JustPath(stripPrefix(fullcur, dir_prefix))), item, path.hash);
            break;
          case pivot_type:
            if (!PivotPath(pivot_tag, item.tags, pivot_path))
              continue;
            exporter.addFile(fullcur, pivot_path, item, path.hash);
            break;
          // assume flat
          default:
            exporter.addFile(fullcur, webpath, item, path.hash);
        }
      }//while
    }// if-else chain
    j++;
  }//while j

  // perform the export
  if (build_file_tree) {
    int count = exporter.commitFileCopy();

    if (count == -1)
      throw ArgumentParser::ErrorException("failed to copy files");

    out << count << " files copied" << endl;
  } else {
    int count = exporter.commitWebSite();

    if (count == -1)
      throw ArgumentParser::ErrorException("failed to build site");

    out << count << " files processed for the web site" << endl;
  }
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  ArgumentParser parser;
  QTextStream out(stdout);

  parser.next();    // eat the program name
  if (!parser.hasNext()) {
    showHelp(out);
    return 0;
  }

  try {
    Engine eng;

    commandWebExport(out, parser);
  }
  catch (ArgumentParser::HelpException &) {
    showHelp(out);
    return 0;
  }
  catch (ArgumentParser::Exception &e){
    showError(out, e);
    return 1;
  }

  return 0;
}

