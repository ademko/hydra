
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
}

Application::~Application()
{
}

void Application::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  QString controllername(filteredRequest.mid(1));

  if (controllername.isEmpty())
    controllername = "home";

  // compute the action
  // find where the action starts
  QString action = "index";
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

  throw ControllerNotFoundException("Controller not found: " + controllername);
}

