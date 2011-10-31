
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/RedirectApp.h>

#include <wexus/HTTPServer.h>

#include <QDebug>

using namespace wexus;

RedirectApp::RedirectApp(void)
{
}

void RedirectApp::init(const QVariantMap &settings)
{
  Application::initBasic(settings);

  // TODO future, add file support
  // TODO future, add regex/remapping, etc

  // parse all the links
  int i=1;
  QString key;
  while (settings.contains(key = "link" + QString::number(i))) {
    QStringList parts = settings.value(key).toString().split(" ");

    if (parts.size() != 2)
      throw HTTPServer::Exception("RedirectApp: error in .ini file, only one space allowed: \"" +
          settings.value(key).toString() + "\"");
    dm_redirs[parts[0]] = parts[1];
    ++i;
  }
  
}

void RedirectApp::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  if (dm_redirs.contains(filteredRequest))
    reply.redirectTo(dm_redirs[filteredRequest]);
}

static wexus::RegisterApp<wexus::RedirectApp> r1("ca.demko.redirect");

