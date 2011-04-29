
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Controller.h>

#include <QDebug>

using namespace wexus;

Controller::Controller(void)
{
}

Controller::~Controller()
{
}

void Controller::handleControllerRequest(QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  qDebug() << "Controller::handleControllerRequest" << actionname;

  actionmap_t::iterator ii = dm_actions.find(actionname);

  if (ii != dm_actions.end())
    ii->second(this);
  else
    throw ActionNotFoundException("Action not found: " + actionname);
}

