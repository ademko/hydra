
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Application.h>

#include <QDebug>

#include <assert.h>

using namespace wexus;

Application::Application(void)
{
  // setup DB
  dm_db = QSqlDatabase::addDatabase("QSQLITE", "C1");
  assert(dm_db.isValid());

  dm_db.setDatabaseName("/tmp/one.sqlite");
  bool good = dm_db.open();
  assert(good);
}

Application::~Application()
{
  // tear down DB
  if (dm_db.isOpen()) {
    dm_db.close();

    dm_db = QSqlDatabase(); // we need to "null" dm_db otherwise removeDatabasew ill think its open

    QSqlDatabase::removeDatabase("C1");
  }
}

void Application::setMountPoint(const QString &mountPoint)
{
  dm_mountpoint = mountPoint;
}

void Application::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  QString controllername(filteredRequest.mid(1));
  QString action("index");

  if (controllername.isEmpty()) {
    //controllername = "home";
    // if no controllername is supplied, redir the user to an explicit default one
    QString redir_url(req.request());

    if (redir_url[redir_url.size()-1] != '/')
      redir_url += "/";
    redir_url += "home/";

    reply.redirectTo(redir_url);
    return;
  } else {
    int index = controllername.indexOf("/");

    // must trail in a /
    // (this is so simple linkTo()s work nicely
    if (index == -1) {
      QString redir_url(req.request());

      redir_url += "/";
      reply.redirectTo(redir_url);
      return;
    }

    assert(index != -1);

    if (index+1 < controllername.size())
      action = controllername.mid(index+1);
    controllername = controllername.left(index);
  }

  // compute the action
  // find where the action starts
qDebug() << "Application::handleApplicationRequest" << filteredRequest << "controllername" << controllername << "action" << action;

  // get my own info object
  // this cant be done in the ctor, as the decendant object isnt built yet
  // see if we a have controller for this request
  if (appInfo()->controllers.contains(controllername)) {
    Context ctx(this, action, req, reply);

    std::shared_ptr<Controller> C( appInfo()->controllers[controllername]->loader() );

    assert(C.get());

    C->handleControllerRequest(action);
  } else
    throw ControllerNotFoundException("wexus::Application: Controller not found: " + controllername);
}

std::shared_ptr<Registry::AppInfo> Application::appInfo(void)
{
  if (!dm_appinfo.get()) {
    dm_appinfo = Registry::appsByType()[typeid(*this).name()];

    assert(dm_appinfo.get());
  }

  return dm_appinfo;
}

