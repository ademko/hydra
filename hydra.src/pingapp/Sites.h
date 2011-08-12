
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGERCONTROLLER_H__
#define __INCLUDED_WEBAPPS_PINGERCONTROLLER_H__

#include <wexus/Controller.h>

#include <pingapp/Site.h>
#include <pingapp/Host.h>

namespace pingapp
{
  class Sites;
}

class pingapp::Sites : public wexus::Controller
{
  public:
    Sites(void);

    void index(void);
    void create(void);
    void edit(void);
    void destroy(void);

    void indexHtml(void);
    void createHtml(void);
    void editHtml(void);
    void destroyHtml(void);

  protected:
    //QVariant dm_counter;
    Site dm_site;
};

#endif

