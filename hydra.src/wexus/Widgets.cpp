
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Widgets.h>

#include <assert.h>

#include <wexus/Context.h>
#include <wexus/Registry.h>
#include <wexus/Application.h>
#include <wexus/Assert.h>

#include <QDebug>

using namespace wexus;

QString wexus::urlTo(void)
{
  return Context::request().request();
}

HTMLString wexus::linkTo(const QString &desc, const QString &rawurl)
{
  return HTMLString("<A HREF=\"" + rawurl + "\">" + HTMLString::encode(desc) + "</A>");
}

QString wexus::memberFunctionToUrl(const QString controllertype, const MemberFunction &mfn)
{
  if (!Registry::controllersByType().contains(controllertype))
    assertThrow(false);

  // first, find the controllertype
  std::shared_ptr<Registry::ControllerInfo> cinfo =
    Registry::controllersByType()[controllertype];

  assert(cinfo.get());

  // verify that that this controller type is inded part of my application
  assertThrow(Context::application()->appInfo() == cinfo->application);

  // find the MemberFunction within this controllers member functions
  Registry::ControllerInfo::ActionMap::const_iterator ii, endii;

  ii = cinfo->actions.begin();
  endii = cinfo->actions.end();

  // TODO fix this function
  // make sure MemberFunction stuf works

  for (; ii != endii; ++ii)
    if ((*ii)->mfn == mfn) // found it
      return Context::application()->mountPoint() + cinfo->name + "/" + (*ii)->actionname;

  // didnt find anything
  assertThrow(false);
  return "";  // will never reach here
}

void wexus::redirectTo(const QString &rawurl)
{
  if (rawurl.isEmpty())
    Context::reply().redirectTo(Context::request().request());
  else
    Context::reply().redirectTo(rawurl);
}

void wexus::renderErrors(void)
{
  Context *ctx = Context::threadInstance();

  assert(ctx);

  bool haserrors = !ctx->errors.isEmpty();

  if (haserrors) {
    Context::output() << "<h3>Form Errors!</h3>\n<ul>\n";
    for (Context::Errors::const_iterator ii=ctx->errors.begin(); ii != ctx->errors.end(); ++ii)
      for (QStringList::const_iterator ee=ii->begin(); ee != ii->end(); ++ee)
        Context::output() << "<li><i>" << HTMLString::encode(ii.key()) << "</i> "
          << HTMLString::encode(*ee) << "</li>\n";
    Context::output() << "</ul>\n";
  }

  //return haserrors;
}

void wexus::renderNotice(void)
{
  Context::output() << "<p> " << HTMLString::encode(
      Context::threadInstance()->flash["notice"].toString()) << "</p>\n";
}

