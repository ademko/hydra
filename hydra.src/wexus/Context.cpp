
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
#include <wexus/FileHTTPHandler.h>    // for sendFile

#include <QThreadStorage>
#include <QDebug>

using namespace wexus;

typedef Context* ContextPtr;

static QThreadStorage<ContextPtr *> Storage;

static QVariantMap extractFlash(const QVariantMap &session)
{
  if (session.contains("flash"))
    return session["flash"].toMap();
  else
    return QVariantMap();
}

Context::Context(wexus::Application *application, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
  : dm_application(application),
    dm_req(req), dm_reply(reply),
    dm_sentHeader(false),
    params(ParamsParser::parse(&dm_req)),
    cookies(&dm_req, &dm_reply),
    dm_sessionlocker(dm_application->sessionManager().getDataByCookie(cookies)),
    session(dm_sessionlocker.map()),
    flash(extractFlash(session))
{
  // clear the flash
  if (session.contains("flash"))
    session["flash"] = QVariant();

  // we need to store a dynamically allocated ptr-to-ptr
  // becase QThreadStorage insists on being able to call delete
  Storage.setLocalData(new ContextPtr(this));

  Form::testFlashValidators(params, flash, errors);
}

Context::~Context()
{
  if (dm_sentHeader)
    sendFooter();

  // save the setFlast as the current flash for the next call
  if ( !(!session.contains("flash") && setFlash.isEmpty()) )
    session["flash"] = setFlash;

  Storage.setLocalData(0);
}

Context * Context::threadInstance(void)
{
  Context *ret = *(Storage.localData());

  assert(ret);

  return ret;
}

QTextStream & Context::output(void)
{
  Context *here = threadInstance();

  if (!here->dm_sentHeader)
    here->sendHeader();

  // flush the other textstreams
  if (here->dm_htmloutput.get())
    here->dm_htmloutput->flush();

  return here->dm_reply.output();
}

QTextStream & Context::htmlOutput(void)
{
  Context *here = threadInstance();

  if (!here->dm_sentHeader)
    here->sendHeader();

  // flush the other textstreams
  here->dm_reply.output().flush();

  if (here->dm_htmloutput.get())
    return *here->dm_htmloutput;

  here->dm_htmldevice.reset(new HTMLEncoderDevice(here->dm_reply.output().device()));
  here->dm_htmloutput.reset(new QTextStream(here->dm_htmldevice.get()));

  return *here->dm_htmloutput;
}

void Context::sendHeader(void)
{
  assert(!dm_sentHeader);

  dm_sentHeader = true;

  if (!dm_application->settings().contains("headerdir"))
    return;

  FileHTTPHandler::sendFile(dm_application->settings()["headerdir"].toString()
      + "/header.html", dm_reply.output().device());
}

void Context::sendFooter(void)
{
  assert(dm_sentHeader);

  // flush everything
  output();
  dm_reply.output().flush();

  FileHTTPHandler::sendFile(dm_application->settings()["headerdir"].toString()
      + "/footer.html", dm_reply.output().device());
}

