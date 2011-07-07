
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerController.h>

#include <wexus/HTMLString.h>

#include <webapps/PingHost.h>

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

  PingHost hosts;

  //hosts.activeClass();

  /*hosts.id = 501;
  hosts.host = "google501.com";
  hosts.insert();*/

  //hosts.all(PingHost::Id);
  hosts.order(PingHost::Host);
  if (hosts.last())
    qDebug() << "last RECORD" << hosts.id << hosts.host;

  /*if (hosts.next()) {
    hosts.host = "first one";
    hosts.save();
  }*/

  //int x = 100;
  while (hosts.next()) {
    qDebug() << "FOUND RECORD" << hosts.id << hosts.host;
    //hosts.host = "index " + QString::number(x);
    //hosts.save();
    //++x;
  }

  indexHtml();
}

