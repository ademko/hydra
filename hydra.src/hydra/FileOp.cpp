
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/FileOp.h>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

bool hydra::linkFile(const QString &srcfile, const QString &destfile)
{
  qDebug() << "LINK " << srcfile << destfile;
#ifdef Q_OS_LINUX
  // we need hardlinks, not the soft, so lets do a sys call
  return 0 == ::link(srcfile.toUtf8().constData(), destfile.toUtf8().constData());
#else
  // or QFile::link?
  //return QFile::link(srcfile, destfile);
  return false;
#endif
}

bool hydra::moveFile(const QString &srcfile, const QString &destfile)
{
  qDebug() << "MOVE " << srcfile << destfile;
  //return 0 == ::rename(srcfile.toUtf8().constData(), destfile.toUtf8().constData());
  return QFile::rename(srcfile, destfile);
}

bool hydra::delFile(const QString &srcfile)
{
  qDebug() << "UNLINK " << srcfile;
  //return 0 == ::unlink(srcfile.toUtf8().constData());
  return QFile::remove(srcfile);
}

bool hydra::copyFile(const QString &srcfile, const QString &destfile)
{
  qDebug() << "COPY " << srcfile << destfile;
  QFile input(srcfile);
  QFile output(destfile);
  QByteArray buf;
  int sz;

  if (!input.open(QIODevice::ReadOnly))
    return false;
  if (!output.open(QIODevice::WriteOnly))
    return false;

  buf.resize(16*1024);

  while ( (sz = input.read(buf.data(), buf.size())) > 0)
    output.write(buf.constData(), sz);

  return true;
}

// returns true on success
bool hydra::smartCopy(const QString &srcfile, const QString &destfile)
{
  // hard link version
  //qDebug() << "SMART_COPY " << srcfile << " to " << destfile;

  if (QFileInfo(destfile).exists())
    return false;

  if (hydra::linkFile(srcfile, destfile))
    return true;

  // do a full copy since we cant do link
  return hydra::copyFile(srcfile, destfile);
}

bool hydra::delDir(const QString &dirpath)
{
  qDebug() << "RMDIR " << dirpath;

  return QDir().rmdir(dirpath);
}

