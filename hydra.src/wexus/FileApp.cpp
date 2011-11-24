
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

void FileApp::init(const QVariantMap &settings)
{
  // TODO future: add option1 = "keys" that translate to FileHTTPHandler directory options

  QString appdir = settings["appdir"].toString();

  assert(!appdir.isEmpty());

  // parse all the dirs
  int i=1;
  QString key;
  while (settings.contains(key = "dir" + QString::number(i))) {
    dm_dirs.append(appdir + "/" + settings.value(key).toString());
    ++i;
  }
  if (dm_dirs.isEmpty())
    dm_dirs.append(appdir);
}

void FileApp::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
qDebug() << "FileApp::handling " << filteredRequest;
  for (int i=0; i<dm_dirs.size(); ++i) {
    FileHTTPHandler::processRequest(0, dm_dirs[i], filteredRequest, reply);
    if (reply.hasReply())
      return; // dont try any more dirs, we got one
  }
}

static wexus::RegisterApp<wexus::FileApp> r1("ca.demko.file");

