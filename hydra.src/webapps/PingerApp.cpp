
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerApp.h>

using namespace webapps;

//
//
// PingerHome
//
//

PingerHome::PingerHome(void)
{
}

//
//
// PingerApp
//
//

PingerApp::PingerApp(void)
{
  registerController<PingerHome>("home");
}

static hydra::Register<PingerApp> r1("webapps::PingerApp");

