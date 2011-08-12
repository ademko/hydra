
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
#include <wexus/VarPath.h>

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

  for (; ii != endii; ++ii)
    if ((*ii)->mfn == mfn) // found it
      return Context::application()->mountPoint() + cinfo->name + "/" + (*ii)->actionname;

  // didnt find anything
  throw AssertException("Trying to reference unregistered action func in class: " + cinfo->name);
}

struct prefix {
  const QString *name;
  const prefix *prev;

  prefix(void) : name(0), prev(0) { }
};

static void fillString(const prefix *p, const QVariantMap &_map, QString &outs)
{
  QVariantMap::const_iterator ii=_map.begin(), endii=_map.end();

  for (; ii != endii; ++ii) {
    if (ii->type() == QVariant::Map) {
      // map, recurse
      prefix subp;

      subp.name = &ii.key();
      subp.prev = p;

      fillString(&subp, asVariantMap(*ii), outs);
    } else {
      // not a map, insert it using the prefix stack to build proper [] keys
      outs += '&';  // the first & will be removed by the caller
      if (p) {
        // slow case
        QString builtkey(ii.key());

        const prefix *curp = p;
        while (curp) {
          builtkey = *curp->name + '[' + builtkey + ']';
          curp = curp->prev;
        }
      } else {
        // fast case
        outs += ii.key();
      }
      outs += '=';
      outs += ii->toString(); // TODO ENCODE
    }
  }
}

QString wexus::variantParamsToUrl(const QVariant &_params)
{
  // TODO add string encoding here
  if (_params.type() != QVariant::Map)
    return "?id= " + _params.toString();

  // else returning a map
  QString r;

  fillString(0, asVariantMap(_params), r);

  // convert the first & to a ? (sneaky :)
  if (!r.isEmpty())
    r[0] = '?';

  return r;
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

