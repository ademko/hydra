
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Controller.h>

#include <QDebug>

using namespace wexus;

//
//
// ControllerContext
//
//


ControllerContext::ControllerContext(void)
  : dm_req(0), dm_reply(0)
{
}

void ControllerContext::setupContext(const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  dm_actionname = actionname;
  dm_req = &req;
  dm_reply = &reply;
}

QTextStream & ControllerContext::output(void)
{
qDebug() << "ControllerContext::output()";
  assert(dm_reply);

  return dm_reply->output();
}

//
//
// Controller
//
//

Controller::Controller(void)
{
}

Controller::~Controller()
{
}

void Controller::handleControllerRequest(const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  qDebug() << "Controller::handleControllerRequest" << actionname;

  actionmap_t::iterator ii = dm_actions.find(actionname);

  if (ii != dm_actions.end()) {
    setupContext(actionname, req, reply);

    ii->second(this);
  } else
    throw ActionNotFoundException("Action not found: " + actionname);
}

