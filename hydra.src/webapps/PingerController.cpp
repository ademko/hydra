
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerController.h>

#include <wexus/HTMLString.h>

#include <webapps/PingHost.h>
#include <webapps/PingSite.h>

#include <QDebug>

using namespace wexus;
using namespace webapps;

//
//
// PingerController
//
//

PingerController::PingerController(void)
{ 
  registerAction<PingerController, &PingerController::index>("index");
}

void PingerController::index(void)
{
  if (cookies.contains("counter"))
    cookies["counter"] = cookies["counter"].toInt() + 1;
  else
    cookies["counter"] = 100;
  dm_counter = cookies["counter"].toInt();

  //reply()->setServerCookie("SESID", "VAL0", "", "", "/");
  //reply()->setServerCookie("A_COOKIE", "VAL1", "", "", "/pinger");
  //qDebug() << "ClientCookies" << request()->cookies()["SESID"] << request()->cookies()["A_COOKIE"];
  /*output() << "from within PingerController::index() via Context<p>\n"
    "and some encoded tags (viewsource): "
    << "<inline_encoded_tag>"
    << HTMLString::encode("<encoded_tag>")
    << HTMLString("<raw_tag>")
    << "<p>\n";*/

  // ActiveRecord test code:

  // this stuff should be moved into "migrations" or something
  {
    PingHost().activeClass()->createTable();
    PingSite().activeClass()->createTable();

    //PingHost().destroyRows();
    //PingSite().destroyRows();
  }

  PingHost hosts;

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

  hosts.order(PingHost::Id);
  //hosts.where(PingHost::Id >= 1000);
  hosts.all();

  qDebug() << "RESULTSET FOLLOWS:";
  while (hosts.next()) {
    qDebug() << "FOUND RECORD: " << hosts.toString();
  }

  qDebug() << "Total records in DB: " << hosts.count();
  qDebug() << "Total records in DB (index <1000: " << hosts.count(PingHost::Id < 1000);

  PingSite sites;

  if (!sites.exists(100)) {
    sites.sitename = "home network";
    sites.create(100);
  }
// controller

  PingHost sub = sites.pingHost();

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
}

