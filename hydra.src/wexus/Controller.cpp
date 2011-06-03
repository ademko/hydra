
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Controller.h>

#include <wexus/HTMLString.h>

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

  params.setupRequest(dm_req);
}

QTextStream & ControllerContext::output(void)
{
  assert(dm_reply);

  // flush the other textstreams
  if (dm_htmloutput.get())
    dm_htmloutput->flush();

  return dm_reply->output();
}

QTextStream & ControllerContext::htmlOutput(void)
{
  // flush the other textstreams
  dm_reply->output().flush();

  if (dm_htmloutput.get())
    return *dm_htmloutput;

  dm_htmldevice.reset(new HTMLEncoderDevice(dm_reply->output().device()));
  dm_htmloutput.reset(new QTextStream(dm_htmldevice.get()));

  return *dm_htmloutput;
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

