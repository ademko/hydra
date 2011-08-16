
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <wexus/Site.h>
#include <pingapp/PingApp.h>

int main(int argc, char **argv)
{
  wexus::Site s(".");

  s.addApplication("/pinger/", std::shared_ptr<pingapp::PingApp>(new pingapp::PingApp));

  s.start();
  s.wait();
}

