
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Context.h>

#include <assert.h>

#include <wexus/HTMLString.h>
#include <wexus/Application.h>

#include <QThreadStorage>

using namespace wexus;

typedef Context* ContextPtr;

static QThreadStorage<ContextPtr *> Storage;

QTextStream & wexus::output(void)
{
  Context *c = Context::instance();

  assert(c);

  return c->output();
}

QTextStream & wexus::htmlOutput(void)
{
  Context *c = Context::instance();

  assert(c);

  return c->htmlOutput();
}

Context::Context(wexus::Application *application, const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
  : dm_application(application),
    dm_actionname(actionname),
    dm_req(req), dm_reply(reply),
    params(&dm_req),
    cookies(&dm_req, &dm_reply),
    dm_sessionlocker(dm_application->sessionManager().getDataByCookie(cookies)),
    session(dm_sessionlocker.map())
{
  // we need to store a dynamically allocated ptr-to-ptr
  // becase QThreadStorage insists on being able to call delete
  Storage.setLocalData(new ContextPtr(this));
}

Context * Context::instance(void)
{
  return *(Storage.localData());
}

Context::~Context()
{
  Storage.setLocalData(0);
}

QTextStream & Context::output(void)
{
  // flush the other textstreams
  if (dm_htmloutput.get())
    dm_htmloutput->flush();

  return dm_reply.output();
}

QTextStream & Context::htmlOutput(void)
{
  // flush the other textstreams
  dm_reply.output().flush();

  if (dm_htmloutput.get())
    return *dm_htmloutput;

  dm_htmldevice.reset(new HTMLEncoderDevice(dm_reply.output().device()));
  dm_htmloutput.reset(new QTextStream(dm_htmldevice.get()));

  return *dm_htmloutput;
}

