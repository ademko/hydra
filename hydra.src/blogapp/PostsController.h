
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_BLOGAPP_POSTSCONTROLLER_H__
#define __INCLUDED_BLOGAPP_POSTSCONTROLLER_H__

#include <wexus/Controller.h>
#include <wexus/MarkDown.h>

#include <blogapp/Post.h>

namespace blogapp
{
  class PostsController;
}

class blogapp::PostsController : public wexus::Controller
{
  public:
    void index(void);
    void indexHtml(void);

    void show(void);
    void showHtml(void);

  protected:
    Post dm_post;
};

#endif

