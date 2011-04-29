
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <wexus/Site.h>
#include <webapps/PingerApp.h>

int main(void)
{
  wexus::Site s(".");

  s.addApplication("/pinger/", std::shared_ptr<webapps::PingerApp>(new webapps::PingerApp));

  s.start();
  s.wait();
}

