
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_BLOGAPP_POST_H__
#define __INCLUDED_BLOGAPP_POST_H__

#include <wexus/ActiveFile.h>

namespace blogapp
{
  class Post;
}

class blogapp::Post : public wexus::ActiveFile
{
  public:
    Post(void);
};

#endif

