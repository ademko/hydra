
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <blogapp/PostsController.h>

#include <QDebug>

#include <wexus/Registry.h>

#include <blogapp/BlogApp.h>

using namespace blogapp;

void PostsController::index(void)
{
  dm_post.all(true);

  indexHtml();
}

void PostsController::show(void)
{
  dm_post.find(params["id"]);
  showHtml();
}

static wexus::RegisterController<BlogApp, PostsController> r1("posts");

static wexus::RegisterAction<PostsController, &PostsController::index> r100("index");
static wexus::RegisterAction<PostsController, &PostsController::show> r101("show");

