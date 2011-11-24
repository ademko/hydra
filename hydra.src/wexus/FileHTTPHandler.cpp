
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/FileHTTPHandler.h>

#include <wexus/MimeTypes.h>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace wexus;

FileHTTPHandler::FileException::FileException(const QString &usermsg)
  : Exception("FileException: " + usermsg)
{
}

FileHTTPHandler::FileHTTPHandler(const QString &docdir, int flags)
  : dm_docdir(docdir), dm_flags(flags)
{
  if (dm_docdir[dm_docdir.size()-1] != '/')
    dm_docdir += '/';    // always have a / on the end
}

void FileHTTPHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  const QString &relpath = req.request();

  // check the relpath for any . that follow a /, and fail them
  bool had_slash = false;
  for (QString::const_iterator ii=relpath.begin(); ii != relpath.end(); ++ii) {
    if (*ii == '/')
      had_slash = true;
    else if (had_slash) {
      if (*ii == '.')
        throw FileException("Bad path 1001"); // fail it, potensially bad path
      had_slash = false;
    }
  }

  processRequest(dm_flags, dm_docdir, relpath, reply);
}

void FileHTTPHandler::processRequest(int flags, const QString &docdir, const QString &relpath, wexus::HTTPReply &reply)
{
  QString fullpath(docdir + relpath);
  QFileInfo info(fullpath);

qDebug() << "FileHTTPHandler serving " << fullpath;

  // check if its a dir
  if (info.isDir()) {
    QString indexfullpath(fullpath + "/index.html");
    QFileInfo indexinfo(indexfullpath);

    // check if there is a index.html and fall through with that
    if (flags & IndexHtml && indexinfo.isFile()) {
      // fall through to file handling below
      fullpath = indexfullpath;
      info = indexinfo;
    } else {
      // not an indexhtml, see if we need to auto gen
      if (flags & AutoDirIndex)
        generateDirIndex(fullpath, relpath, reply);

      // if were here, that means its a dir but we cant handle it
      // lets just bail and assume some other handler will take it
      return;
    }
  }

  // assume its a file
  if (!info.isFile())
    return;

  QString fileext(info.suffix().toLower());

  // check mime type
  if ((flags & AllowAllMimeTypes) && !MimeTypes::containsMimeType(fileext))
    reply.setContentType(MimeTypes::binaryMimeType());  // send binary if I can and don't have a type
  else
    reply.setContentType(MimeTypes::mimeType(fileext));

  if (!sendFile(fullpath, reply.output().device()))
    throw FileException("QFile::open failed");
}

bool FileHTTPHandler::sendFile(const QString &filename, QIODevice * outputdev)
{
  // open and send the file
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly))
    return false;

  char buf[1024*4];
  qint64 num;

  while ( (num = file.read(buf, sizeof(buf))) > 0)
    if (num != outputdev->write(buf, num))
      throw FileException("QFile::write short-wrote");

  return true;
}

void FileHTTPHandler::generateDirIndex(const QString &fullpath,
    const QString &relpath, wexus::HTTPReply &reply)
{
  reply.output() << "<h2>Index of: " << relpath <<
      "</h2>\n<UL>\n<LI><A HREF=\"../\">Parent Directory</A></LI><P>\n";

  QDir dir(fullpath);
  QStringList entries(dir.entryList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::DirsFirst|QDir::Name));

  for (QStringList::const_iterator ii=entries.begin(); ii != entries.end(); ++ii) {
    QString curfullpath(fullpath + "/" + *ii);
    QFileInfo curinfo(curfullpath);
    bool isdir = curinfo.isDir();
    QString escaped(escapeForXML(*ii));

    reply.output() <<  "<LI><A HREF=\"" << escaped;

    if (isdir)
      reply.output() << '/';

    reply.output() << "\">" << escaped << "</A>  ";

    if (isdir)
      reply.output() << "(DIRECTORY)";
    else
      reply.output() << (curinfo.size()/1024) << " kbytes";
    reply.output() << "</LI>\n";
  }

  reply.output() << "</UL>\n";
}

