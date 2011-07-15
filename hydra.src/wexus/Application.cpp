
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Application.h>

#include <assert.h>

using namespace wexus;

Application::registry_type Application::registry;

Application::Application(void)
{
  // setup DB
  dm_db = QSqlDatabase::addDatabase("QSQLITE", "C1");
  assert(dm_db.isValid());

  dm_db.setDatabaseName("/tmp/one.sqlite");
  bool good = dm_db.open();
  assert(good);
//qDebug() << "creating DB";
}

Application::~Application()
{
  // tear down DB
  if (dm_db.isOpen()) {
    QSqlDatabase::removeDatabase("C1");

    dm_db.close();
  }
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

  // see if we a have controller for this request
  for (int i=0; i<dm_controllers.size(); ++i)
    if (controllername == dm_controllers.name(i)) {
      Context ctx(this, action, req, reply);

      std::shared_ptr<Controller> C = dm_controllers.create(i);

      assert(C.get());

      C->handleControllerRequest(action);

      return;
    }

  throw ControllerNotFoundException("wexus::Application: Controller not found: " + controllername);
}

