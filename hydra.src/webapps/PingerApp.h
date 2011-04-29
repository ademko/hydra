
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGERAPP_H__
#define __INCLUDED_WEBAPPS_PINGERAPP_H__

#include <wexus/Application.h>

namespace webapps
{
  class PingerController;
  class PingerApp;
}

class webapps::PingerController : public wexus::Controller
{
  public:
    PingerController(void);

    void index(void);
};

/**
 * The first wexus application - a uptime/ping monitor.
 *
 * @author Aleksander Demko
 */ 
class webapps::PingerApp : public wexus::Application
{
  public:
    PingerApp(void);
};

#endif

