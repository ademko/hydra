
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/Sites.h>

#include <wexus/HTMLString.h>

#include <pingapp/Host.h>

#include <QDebug>

using namespace wexus;
using namespace pingapp;

//
//
// Sites
//
//

Sites::Sites(void)
{ 
  registerAction<Sites, &Sites::index>("index");
  registerAction<Sites, &Sites::create>("create");
}

void Sites::index(void)
{
  // need to put this someplace better!
  // atleast before the migration stuff too
  Host().activeClass()->createTable();
  Site().activeClass()->createTable();

  dm_site.all();

  indexHtml();
}

void Sites::create(void)
{
  createHtml();
}

  /*
{
  if (cookies.contains("counter"))
    cookies["counter"] = cookies["counter"].toInt() + 1;
  else
    cookies["counter"] = 100;
  dm_counter = cookies["counter"].toInt();

  //reply()->setServerCookie("SESID", "VAL0", "", "", "/");
  //reply()->setServerCookie("A_COOKIE", "VAL1", "", "", "/pinger");
  //qDebug() << "ClientCookies" << request()->cookies()["SESID"] << request()->cookies()["A_COOKIE"];
  //output() << "from within Site::index() via Context<p>\n"
  //  "and some encoded tags (viewsource): "
  //  << "<inline_encoded_tag>"
  //  << HTMLString::encode("<encoded_tag>")
  //  << HTMLString("<raw_tag>")
  //  << "<p>\n";

  // ActiveRecord test code:

  // this stuff should be moved into "migrations" or something
  {
    Host().activeClass()->createTable();
    Site().activeClass()->createTable();

    //Host().destroyRows();
    //Site().destroyRows();
  }

  Host hosts;

  if (hosts.exists(501))
    hosts.destroy();
  hosts.destroy(502);

  hosts.hostname = "test record";
  hosts.create(501);
  hosts.create(502);

  hosts.find(501);
  hosts.hostname = "updated string";
  hosts.save();

  hosts.find(502);
  hosts.destroy();

  hosts.order(Host::Id);
  //hosts.where(Host::Id >= 1000);
  hosts.all();

  qDebug() << "RESULTSET FOLLOWS:";
  while (hosts.next()) {
    qDebug() << "FOUND RECORD: " << hosts.toString();
  }

  qDebug() << "Total records in DB: " << hosts.count();
  qDebug() << "Total records in DB (index <1000: " << hosts.count(Host::Id < 1000);

  Site sites;

  if (!sites.exists(100)) {
    sites.sitename = "home network";
    sites.create(100);
  }
// controller

  Host sub = sites.host();

  sub.destroyRows();

  sub.hostname = "sub host name";
  sub.create();
  sub.hostname = "another host name";
  sub.create();

qDebug() << sub.toString();
  sub.all();
  qDebug() << "RESULTSET FOLLOWS:";
  while (sub.next()) {
    qDebug() << "FOUND RECORD: " << sub.toString();
  }

  indexHtml();
}*/

