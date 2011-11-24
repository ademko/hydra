
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/FileApp.h>

#include <QDebug>

#include <wexus/FileHTTPHandler.h>

using namespace wexus;

FileApp::FileApp(void)
{
}

FileApp::FileApp(const QString &docdir)
{
  dm_dirs.append(DirFlags(docdir, FileHTTPHandler::AutoDirIndex|FileHTTPHandler::AllowAllMimeTypes));
}

static int parseFileHTTPHandlerFlags(const QString &flagsstring)
{
  QStringList parts = flagsstring.split(" ");
  int r = 0;

  for (QStringList::const_iterator ii=parts.begin(); ii != parts.end(); ++ii) {
    if ((*ii).toLower() == "indexhtml")
      r |= FileHTTPHandler::IndexHtml;
    if ((*ii).toLower() == "autodirindex")
      r |= FileHTTPHandler::AutoDirIndex;
    if ((*ii).toLower() == "allowallmimetypes")
      r |= FileHTTPHandler::AllowAllMimeTypes;
  }

  return r;
}

void FileApp::init(const QVariantMap &settings)
{
  QString appdir = settings["appdir"].toString();

  assert(!appdir.isEmpty());

  // parse all the dirs
  int i=1;
  QString key, flagskey;
  while (settings.contains(key = "dir" + QString::number(i))) {
    int flags = 0;
    if (settings.contains(flagskey = "options" + QString::number(i)))
      flags = parseFileHTTPHandlerFlags(settings.value(flagskey).toString());
    dm_dirs.append(DirFlags(appdir + "/" + settings.value(key).toString(), flags));
    ++i;
  }
  if (dm_dirs.isEmpty())
    dm_dirs.append(appdir);
}

void FileApp::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
qDebug() << "FileApp::handling " << filteredRequest;
  for (int i=0; i<dm_dirs.size(); ++i) {
    FileHTTPHandler::processRequest(dm_dirs[i].flags, dm_dirs[i].dirname, filteredRequest, reply);
    if (reply.hasReply())
      return; // dont try any more dirs, we got one
  }
}

static wexus::RegisterApp<wexus::FileApp> r1("ca.demko.file");

