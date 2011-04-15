
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/FileHTTPHandler.h>

#include <QFile>
#include <QDebug>

using namespace wexus;

FileHTTPHandler::FileException::FileException(const QString &usermsg)
  : HTTPException("FileException: " + usermsg)
{
}

FileHTTPHandler::FileHTTPHandler(const QString &docdir)
  : dm_docdir(docdir)
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

qDebug() << "FileHTTPHandler serveing " << fullpath;

  // TODO check extension
  // and only service up approved files (security feature)

  // check mime type
  // TODO
  //reply.setContentType("application/octet-stream");
  reply.setContentType("text/plain");

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

