
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerApp.h>

#include <wexus/Context.h>

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
  output() << "from within PingerController::index() via Context";
}

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

