
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerApp.h>

#include <webapps/PingerController.h>

using namespace wexus;
using namespace webapps;

//
//
// PingerApp
//
//

PingerApp::PingerApp(void)
{
  registerController<PingerController>("home");
}

static hydra::Register<PingerApp> r1("webapps::PingerApp");

