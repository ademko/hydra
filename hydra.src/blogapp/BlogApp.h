
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_BLOGAPP_H__
#define __INCLUDED_WEBAPPS_BLOGAPP_H__

#include <wexus/Application.h>

namespace blogapp
{
  class BlogApp;
}

/**
 * A basic blogging application.
 *
 * @author Aleksander Demko
 */ 
class blogapp::BlogApp : public wexus::Application
{
  public:
    BlogApp(void);
};

#endif

