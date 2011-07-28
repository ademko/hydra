
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Controller.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

//
//
// ControllerContext
//
//


ControllerContext::ControllerContext(void)
  : dm_context(*Context::threadInstance()),
  params(dm_context.params),
  cookies(dm_context.cookies),
  session(dm_context.session),
  flash(dm_context.flash),
  setFlash(dm_context.setFlash),
  errors(dm_context.errors)
{
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

void Controller::handleControllerRequest(const QString &actionname)
{
  qDebug() << "Controller::handleControllerRequest" << actionname;

  actionmap_t::iterator ii = dm_actions.find(actionname);

  if (ii != dm_actions.end()) {
    // call the found action
    ii->second(this);

    if (Context::reply().status() == 0)
      throw HTTPHandler::Exception("Controller called, but it didn't set any status (or send output)");
  } else
    throw ActionNotFoundException("wexus::Controller: Action not found: " + actionname);
}

