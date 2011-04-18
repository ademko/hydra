
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
  : HTTPException("FileException: " + usermsg)
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

  QString fullpath(dm_docdir + relpath);
  QFileInfo info(fullpath);

qDebug() << "FileHTTPHandler serveing " << fullpath;

  // check if its a dir
  if (info.isDir()) {
    QString indexfullpath(fullpath + "/index.html");
    QFileInfo indexinfo(indexfullpath);

    // check if there is a index.html and fall through with that
    if (dm_flags & IndexHtml && indexinfo.isFile()) {
      // fall through to file handling below
      fullpath = indexfullpath;
      info = indexinfo;
    } else {
      // not an indexhtml, see if we need to auto gen
      if (dm_flags & AutoDirIndex)
        generateDirIndex(req, reply, fullpath, relpath);

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
  if ((dm_flags & AllowAllMimeTypes) && !MimeTypes::instanceCreate()->hasMimeType(fileext))
    reply.setContentType(MimeTypes::binaryMimeType());  // send binary if I can and don't have a type
  else
    reply.setContentType(MimeTypes::instanceCreate()->mimeType(fileext));

  // open and send the file
  {
    QFile file(fullpath);

    if (!file.open(QIODevice::ReadOnly))
      throw FileException("QFile::open failed");

    char buf[1024*4];
    qint64 num;
    QIODevice *outputdev = reply.output().device();

    while ( (num = file.read(buf, sizeof(buf))) > 0)
      if (num != outputdev->write(buf, num))
        throw FileException("QFile::write short-wrote");
  }
}

void FileHTTPHandler::generateDirIndex(wexus::HTTPRequest &req, wexus::HTTPReply &reply,
  const QString &fullpath, const QString &relpath)
{
  reply.output() << "<h2>Index of: " << relpath <<
      "</h2><UL><LI><A HREF=\"../\">Parent Directory</A><P>\n";

  QDir dir(fullpath);
  QStringList entries(dir.entryList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::DirsFirst|QDir::Name));

  for (QStringList::const_iterator ii=entries.begin(); ii != entries.end(); ++ii) {
    QString escaped(*ii);

    reply.output() <<  "<LI><A HREF=\"" << escaped;

    //if (ii->second == -1)
      //reply.output() << '/';

    reply.output() << "\">" << escaped << "</A>  ";

    //if (ii->second == -1)
      //reply.output() << "(DIRECTORY)";
    //else
      //reply.output() << ii->second << " kbytes";
    reply.output() << '\n';
  }

  //
  // TODO finish this function
  //  make sure relatives work
  //  show (DIR) or file size for entries
  //  do escape for html
  //

  reply.output() << "</UL>\n";
}

