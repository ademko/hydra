
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/PingApp.h>

#include <wexus/Registry.h>
#include <wexus/VarPath.h>

#include <pingapp/SitesController.h>
#include <pingapp/HostsController.h>

using namespace wexus;
using namespace pingapp;

//
//
// PingApp
//
//

PingApp::PingApp(void)
{
  RouteBuilder b(*this);
  b.addStandardRoutes();
  b.addMatch("/", key("controller","sites") + key("action","index"));
}

static RegisterApp<PingApp> r2("ca.demko.pingapp");

