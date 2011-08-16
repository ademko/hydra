
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGERAPP_H__
#define __INCLUDED_WEBAPPS_PINGERAPP_H__

#include <wexus/Application.h>

namespace pingapp
{
  class PingApp;
}

/**
 * The first wexus application - a uptime/ping monitor.
 *
 * @author Aleksander Demko
 */ 
class pingapp::PingApp : public wexus::Application
{
  public:
    PingApp(void);
};

#endif

