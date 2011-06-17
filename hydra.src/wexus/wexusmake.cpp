
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <QTextStream>

#include <assert.h>

#include <hydra/ArgumentParser.h>
#include <hydra/Engine.h>   // for HYDRA_COPYRIGHT_STRING

#include <wexus/TemplateTokenList.h>
#include <wexus/HTMLTemplateParser.h>
#include <wexus/ModelTokenList.h>
#include <wexus/HeaderModelParser.h>
#include <wexus/CPPScanner.h>

#include <QFile>
#include <QDirIterator>
#include <QDebug>

using namespace hydra;
using namespace wexus;

static void showHelp(const QString &progname, QTextStream &out)
{
  out <<
    progname << " -- Wexus Utility -- " " (" __DATE__ " " __TIME__ ")\n"
    "A programmers tool for Wexus Library users\n"
    "Copyright (c) " HYDRA_COPYRIGHT_STRING ", Aleksander B. Demko\n\n"
    "Usage:\n"
    "  " << progname << " command [command options]\n\n"
    "Where command can be one of:\n"
    "  template   converts an .ecpp file to a .cpp file\n"
    "               -i filename         the input filename (required)\n"
    "               -o filename         the output filename (optional)\n"
    "The input filename must be of the form  class__method_name.html.ecpp\n"
    "(yes, double underscores represent :: delimeters)\n"
    "Optionally, additional namespace__ prefixes are accepted\n"
    "\n"
    "  views directory\n"
    "             convert all the .ecpp in the given directory (and all\n"
    "             its sub-directories\n"
    "  models directory\n"
    "             convert all the .eh in the given directory (and all\n"
    "             its sub-directories\n"
    << endl;
}
static void showError(QTextStream &out, std::exception &e)
{
  out << "error: " << e.what() << endl;
}

/**
 * Given a filename, decompose it into its logical parts.
 * (this is used for the template command)
 * 
 * filename cannot be empty
 *
 * Throws argument expcetions on errors.
 * @author Aleksander Demko
 */ 
void parseFileName(const QString &filename,
    QStringList &outparts,
    QString &outsubext,
    QString &outext)
{
  assert(!filename.isEmpty());

  outparts.clear();
  outsubext.clear();
  outext.clear();

  // start at the back
  int i = filename.size() - 1;

  // find the extension
  for (;i>=0; i--)
    if (filename[i] == '.')
      break;

  if (filename[i] != '.')
    throw ArgumentParser::ErrorException("No file extension found");

  int extmarker = i;
  outext = filename.mid(extmarker+1);

  // find the subextension
  i--;
  for (;i>=0; i--)
    if (filename[i] == '.')
      break;

  if (filename[i] != '.')
    throw ArgumentParser::ErrorException("No .html or similar sub-extension found");

  int subextmarker = i;
  outsubext = filename.mid(subextmarker+1, extmarker-subextmarker-1);

  int lastmarker = i;
  bool have_one = false;

  i--;
  for (; i>=0; i--) {
    if (filename[i] == '/')
      break;
    if (!have_one) {
      if (filename[i] == '_')
        have_one = true;
    } else { // have_one
      have_one = false;
      if (filename[i] == '_') { // have two!
        QString sub = filename.mid(i+2, lastmarker-i-2);
        outparts.push_front(sub);
        lastmarker = i;
      }
    }
  }
  // add the first segment, which was never delimeted
  QString lastsub = filename.mid(i+1, lastmarker-(i+1));
  outparts.push_front(lastsub);
}

void parseFilePathParts(const QString &filename,
    QStringList &outparts)
{
  assert(!filename.isEmpty());

  outparts.clear();

  int startmarker = 0;

qDebug() << filename;
  for (int i=0; i<filename.size(); ++i) {
    if (filename[i] == '/') {
      if (i - startmarker > 0)
        outparts.push_back(filename.mid(startmarker, i-startmarker));
      startmarker = i+1;
    }
  }//for
}

static void bufferToCString(const QByteArray &ary, QTextStream &out)
{
  for (int i=0; i<ary.size(); ++i) {
    char c = ary[i];
    switch (c) {
      case '\n': out << "\\n"; break;
      case '\t': out << "\\t"; break;
      case '\r': out << "\\r"; break;
      case '\"': out << "\\\""; break;
      default: out << c; break;
    }
  }
}

static void codeBufferToStream(const QByteArray &ary, QTextStream &out,
    int lineno, const QString &infilename)
{
  QByteArray::const_iterator ii = ary.begin(), endii = ary.end();
  while (ii != endii) {
    out << "#line " << lineno << " \"" << infilename << "\"\n";

    while (ii != endii) {
      bool wasnewline = *ii == '\n';
      out << static_cast<char>(*ii);
      ++ii;
      if (wasnewline)
        break;
    }

    ++lineno;
  }
}

void generateCPPOutput(const QString &infilename, QIODevice &outfile, const TemplateTokenList &toklist, const QStringList &parts)
{
  QTextStream outstream(&outfile);
  QStringList::const_iterator sl;

  // preamble
  outstream << "//generated by wexusmake\n" "#include <wexus/TemplateInclude.h>\n\n" "#include <";

  // include the class's .h file
  int class_parts = parts.size() - 1;
  assert(class_parts>0);

  sl = parts.begin();
  class_parts--;
  outstream << *sl;
  for (sl++; class_parts>0; ++sl, class_parts--) {
    outstream << "/" << *sl;
  }
  outstream << ".h>\n\n";

  outstream << "using namespace wexus;\n\n";

  // include the namespace, if any
  int namespace_parts = parts.size() - 2;
  sl = parts.begin();
  if (namespace_parts > 0) {
    outstream << "using namespace " << *sl;
    namespace_parts--;
    while (namespace_parts > 0) {
      outstream << "::" << *sl;
      sl++;
      namespace_parts--;
    }
    outstream << ";\n\n";
  }

  // start the function
  outstream << "void ";
  sl = parts.begin();
  outstream << *sl;
  for (sl++; sl!=parts.end(); ++sl) {
    outstream << "::" << *sl;
  }
  outstream << "(void)\n{\n";

  // go through all the tokens
  for (TemplateTokenList::const_iterator ii=toklist.begin(); ii != toklist.end(); ++ii) {
    switch ((*ii)->type()) {
      case 'L'://literal
        outstream << "#line " << (*ii)->lineno() << " \"" << infilename << "\"\n";
        outstream << "  output() << \"";
        bufferToCString((*ii)->data(), outstream);
        outstream << "\";\n";
        break;
      case ' '://code
        codeBufferToStream((*ii)->data(), outstream, (*ii)->lineno(), infilename);
        outstream << '\n';
        break;
      case '='://return value code
        outstream << "  htmlOutput() <<\n";
        codeBufferToStream((*ii)->data(), outstream, (*ii)->lineno(), infilename);
        outstream << ";\n";
        break;
    }//switch
  }//for

  // end the function and file
  outstream << "}\n\n";
}

static void commandCompile(QTextStream &out, hydra::ArgumentParser &args)
{
  QString infilename;
  QString outfilename;

  while (args.hasNext()) {
    QString a;
    bool isswitch;

    a = args.next(&isswitch);

    if (isswitch) {
      if (a == "-i")
        infilename = args.nextParam(a);
      else if (a == "-o")
        outfilename = args.nextParam(a);
    }
  }

  if (infilename.isEmpty())
    throw ArgumentParser::ErrorException("Missing -i filename");

  QStringList parts;
  QString subext, ext;

  parseFileName(infilename, parts, subext, ext);

  if (ext != "ecpp")
    throw ArgumentParser::ErrorException("Input file must end in .ecpp, not " + ext);

  if (parts.size() < 2)
    throw ArgumentParser::ErrorException("Filename must be of the form class::method");

  if (outfilename.isEmpty())
    outfilename = infilename.mid(0, infilename.size() - ext.size()) + "cpp";

  if (subext != "html")
    throw ArgumentParser::ErrorException("Only html sub-types are supported (for now)");

//qDebug() << "infilename" << infilename << "subext" << subext << "ext" << ext << "parts" << parts << "outfilename" << outfilename;

  TemplateTokenList toklist;

  // parse the input stream
  {
    QFile in(infilename);

    if (!in.open(QIODevice::ReadOnly))
      throw ArgumentParser::ErrorException("Cannot open input file: " + infilename);

    HTMLTemplateParser().parse(in, toklist);
    compressList(toklist);
  }

//for (TemplateTokenList::const_iterator ii=toklist.begin(); ii!=toklist.end(); ++ii)
//qDebug() << "TOKEN" << (*ii)->type() << (*ii)->data();

  {
    QFile out(outfilename);

    if (!out.open(QIODevice::WriteOnly))
      throw ArgumentParser::ErrorException("Cannot open output file: " + outfilename);

    generateCPPOutput(infilename, out, toklist, parts);
  }

  qDebug() << "wrote" << outfilename;
}

static void commandViews(QTextStream &out, hydra::ArgumentParser &args)
{
  QString viewdir;

  while (args.hasNext()) {
    QString a;
    bool isswitch;

    a = args.next(&isswitch);

    if (!isswitch && viewdir.isEmpty())
      viewdir = a;
  }

  if (viewdir.isEmpty())
    throw ArgumentParser::ErrorException("Missing view directory");

  QDirIterator dd(viewdir, QDirIterator::Subdirectories);

  while (dd.hasNext()) {
    QString infilename(dd.next());
    QFileInfo info(infilename);
    QString baseName(info.baseName());
    QString ext("ecpp");

    if (!info.isFile() || info.suffix() != ext)
      continue;

    if (baseName.indexOf("html", 0, Qt::CaseInsensitive) == -1)
      throw ArgumentParser::ErrorException("Only html sub-types are supported (for now): " + infilename);

    // parse the file

    TemplateTokenList toklist;
    QFile in(infilename);

    if (!in.open(QIODevice::ReadOnly))
      throw ArgumentParser::ErrorException("Cannot open input file: " + infilename);

    HTMLTemplateParser().parse(in, toklist);
    compressList(toklist);

    QStringList parts;
    QString outfilename;

    parseFilePathParts(infilename.mid(viewdir.size()), parts);

    parts.push_back(baseName);
    outfilename = infilename.mid(0, infilename.size() - ext.size()) + "cpp";

qDebug() << "infilename" << infilename << "ext" << ext << "parts" << parts << "outfilename" << outfilename;

    // render the output
    QFile out(outfilename);

    if (!out.open(QIODevice::WriteOnly))
      throw ArgumentParser::ErrorException("Cannot open output file: " + outfilename);

    generateCPPOutput(infilename, out, toklist, parts);

    qDebug() << "wrote" << outfilename;
  }
}

static void commandModels(QTextStream &out, hydra::ArgumentParser &args)
{
  QString modeldir;

  while (args.hasNext()) {
    QString a;
    bool isswitch;

    a = args.next(&isswitch);

    if (!isswitch && modeldir.isEmpty())
      modeldir = a;
  }

  if (modeldir.isEmpty())
    throw ArgumentParser::ErrorException("Missing model directory");

  QDirIterator dd(modeldir, QDirIterator::Subdirectories);

  while (dd.hasNext()) {
    QString infilename(dd.next());
    QFileInfo info(infilename);
    QString baseName(info.baseName());
    QString ext("eh");

    if (!info.isFile() || info.suffix() != ext)
      continue;

    // parse the file

    ModelTokenList toklist;
    QFile in(infilename);

    if (!in.open(QIODevice::ReadOnly))
      throw ArgumentParser::ErrorException("Cannot open input file: " + infilename);

    try {
      HeaderModelParser().parse(in, toklist);
    }
    catch (std::exception &e){
      showError(out, e);
      return;
    }

    for (ModelTokenList::const_iterator mm=toklist.begin(); mm != toklist.end(); ++mm) {
      qDebug() << "type" << (*mm)->type();
      if ((*mm)->type() == 'L') {
        LiteralModelToken *lm = dynamic_cast<LiteralModelToken*>(mm->get());
        assert(lm);
        qDebug() << "  " << lm->data();
      }

      if ((*mm)->type() == 'F') {
        FieldModelToken *fm = dynamic_cast<FieldModelToken*>(mm->get());
        assert(fm);
        qDebug() << "  " << fm->fieldType() << fm->fieldName();
      }
    }

    //compressList(toklist);

    /*QStringList parts;
    QString outfilename;

    parseFilePathParts(infilename.mid(modeldir.size()), parts);

    parts.push_back(baseName);
    outfilename = infilename.mid(0, infilename.size() - ext.size()) + "cpp";

qDebug() << "infilename" << infilename << "ext" << ext << "parts" << parts << "outfilename" << outfilename;

    // render the output
    QFile out(outfilename);

    if (!out.open(QIODevice::WriteOnly))
      throw ArgumentParser::ErrorException("Cannot open output file: " + outfilename);

    generateCPPOutput(infilename, out, toklist, parts);

    qDebug() << "wrote" << outfilename;*/
  }
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  hydra::ArgumentParser args;
  QTextStream out(stdout);

  QString progname = args.next(); // eat the program name

  // iterate through the params
  try {
    if (!args.hasNext())
      throw ArgumentParser::HelpException();

    QString cmd(args.next());

    if (cmd == "template")
      commandCompile(out, args);
    else if (cmd == "views")
      commandViews(out, args);
    else if (cmd == "models")
      commandModels(out, args);
    else
      throw ArgumentParser::ErrorException("Unknown command: " + cmd);
  }
  catch (ArgumentParser::HelpException &) {
    showHelp(progname, out);
    return 0;
  }
  catch (std::exception &e){
    showError(out, e);
    return 1;
  }
}

