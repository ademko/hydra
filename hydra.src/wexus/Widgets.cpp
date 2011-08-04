
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Widgets.h>

#include <assert.h>

#include <wexus/Context.h>

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

void wexus::redirectTo(const QString &rawurl)
{
  if (rawurl.isEmpty())
    Context::reply().redirectTo(Context::request().request());
  else
    Context::reply().redirectTo(rawurl);
}

bool wexus::renderErrors(void)
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

  return haserrors;
}

