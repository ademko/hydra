
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/App.h>

#include <wexus/Registry.h>

#include <pingapp/Sites.h>
#include <pingapp/Hosts.h>

using namespace wexus;
using namespace pingapp;

//
//
// App
//
//

App::App(void)
{
  registerController<Sites>("sites");
  registerController<Hosts>("hosts");
}

static hydra::Register<App> r1("pingapp::App");

static RegisterApp<App> r2("pingapp::App");

