
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Context.h>

#include <assert.h>

#include <wexus/HTMLString.h>
#include <wexus/Application.h>
#include <wexus/Form.h>

#include <QThreadStorage>
#include <QDebug>

using namespace wexus;

typedef Context* ContextPtr;

static QThreadStorage<ContextPtr *> Storage;

Context::Context(wexus::Application *application, const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
  : dm_application(application),
    dm_actionname(actionname),
    dm_req(req), dm_reply(reply),
    params(ParamsParser::parse(&dm_req)),
    cookies(&dm_req, &dm_reply),
    dm_sessionlocker(dm_application->sessionManager().getDataByCookie(cookies)),
    session(dm_sessionlocker.map()),
    flash(session["flash"].toMap())
{
  // clear the flash
  session["flash"] = QVariant();

  // we need to store a dynamically allocated ptr-to-ptr
  // becase QThreadStorage insists on being able to call delete
  Storage.setLocalData(new ContextPtr(this));

  Form::testFlashValidators(params, flash, errors);
}

Context * Context::threadInstance(void)
{
  Context *ret = *(Storage.localData());

  assert(ret);

  return ret;
}

Context::~Context()
{
  // save the setFlast as the current flash for the next call
  session["flash"] = setFlash;

  Storage.setLocalData(0);
}

QTextStream & Context::output(void)
{
  Context *here = threadInstance();

  // flush the other textstreams
  if (here->dm_htmloutput.get())
    here->dm_htmloutput->flush();

  return here->dm_reply.output();
}

QTextStream & Context::htmlOutput(void)
{
  Context *here = threadInstance();

  // flush the other textstreams
  here->dm_reply.output().flush();

  if (here->dm_htmloutput.get())
    return *here->dm_htmloutput;

  here->dm_htmldevice.reset(new HTMLEncoderDevice(here->dm_reply.output().device()));
  here->dm_htmloutput.reset(new QTextStream(here->dm_htmldevice.get()));

  return *here->dm_htmloutput;
}

