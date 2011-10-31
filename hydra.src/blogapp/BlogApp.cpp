
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <blogapp/BlogApp.h>

#include <wexus/VarPath.h>

using namespace wexus;

blogapp::BlogApp::BlogApp(void)
{
  RouteBuilder b(*this);

  b.addStandardRoutes();
  b.addMatch("/", key("controller","posts") + key("action","index"));
}

static wexus::RegisterApp<blogapp::BlogApp> r1("ca.demko.blog");

