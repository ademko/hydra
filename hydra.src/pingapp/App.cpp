
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
}

static RegisterApp<App> r2("pingapp::App");

