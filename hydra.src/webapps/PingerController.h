
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGERCONTROLLER_H__
#define __INCLUDED_WEBAPPS_PINGERCONTROLLER_H__

#include <wexus/Controller.h>

namespace webapps
{
  class PingerController;
}

class webapps::PingerController : public wexus::Controller
{
  public:
    PingerController(void);

    void index(void);

    void renderHtml(void);
};

#endif

