
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_PINGAPP_HOSTS_H__
#define __INCLUDED_PINGAPP_HOSTS_H__

#include <wexus/Controller.h>

#include <pingapp/Site.h>
#include <pingapp/Host.h>

namespace pingapp
{
  class Hosts;
}

class pingapp::Hosts : public wexus::Controller
{
  public:
    Hosts(void);

    void index(void);

    void indexHtml(void);

  protected:
    Host dm_host;
};

#endif

