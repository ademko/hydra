
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

#include <QFile>
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
    << endl;
}
static void showError(QTextStream &out, std::exception &e)
{
  out << "error: " << e.what() << endl;
}

/**
 * Given a filename, decompose it into its logical parts.
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
    throw ArgumentParser::ErrorException("No .ecpp file extension found");

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

static void commandCompile(QTextStream &out, hydra::ArgumentParser &args)
{
  QString infile;
  QString outfile;

  while (args.hasNext()) {
    QString a;
    bool isswitch;

    a = args.next(&isswitch);

    if (isswitch) {
      if (a == "-i")
        infile = args.nextParam(a);
      else if (a == "-o")
        outfile = args.nextParam(a);
    }
  }

  if (infile.isEmpty())
    throw ArgumentParser::ErrorException("Missing -i filename");

  QStringList parts;
  QString subext, ext;

  parseFileName(infile, parts, subext, ext);

  if (ext != "ecpp")
    throw ArgumentParser::ErrorException("Infile must end in .ecpp, not " + ext);

  if (parts.size() < 2)
    throw ArgumentParser::ErrorException("Filename must be of the form class::method");

  if (outfile.isEmpty())
    outfile = infile.mid(0, infile.size() - ext.size()) + "cpp";

  if (subext != "html")
    throw ArgumentParser::ErrorException("Only html sub-types are supported (for now)");

//qDebug() << "infile" << infile << "subext" << subext << "ext" << ext << "parts" << parts << "outfile" << outfile;

  TemplateTokenList toklist;

  // parse the input stream
  {
    QFile in(infile);

    if (!in.open(QIODevice::ReadOnly))
      throw ArgumentParser::ErrorException("Cannot open input file: " + infile);

    HTMLTemplateParser().parse(in, toklist);
    compressList(toklist);
  }

//for (TemplateTokenList::const_iterator ii=toklist.begin(); ii!=toklist.end(); ++ii)
//qDebug() << "TOKEN" << (*ii)->type() << (*ii)->data();

  {
    QFile out(outfile);

    if (!out.open(QIODevice::WriteOnly))
      throw ArgumentParser::ErrorException("Cannot open output file: " + outfile);

    QTextStream outstream(&out);
    QStringList::iterator sl;

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
      outstream << "#line " << (*ii)->lineno() << " \"" << infile << "\"\n";
      switch ((*ii)->type()) {
        case 'L'://literal
          outstream << "wexus::output() << \"";
          bufferToCString((*ii)->data(), outstream);
          outstream << "\";\n";
          break;
        case ' '://code
        case '='://how to handle output? functions or streams?
        {
          outstream << "  " << (*ii)->data() << '\n';
          break;
        }
      }//switch
    }//for

    // end the function and file
    outstream << "}\n\n";
  }

  qDebug() << "wrote" << outfile;
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

