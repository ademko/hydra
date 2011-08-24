
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <blogapp/Post.h>

#include <wexus/Application.h>
#include <wexus/Context.h>

blogapp::Post::Post(void)
  : ActiveFile(
      wexus::Context::application()->settings()["appdir"].toString(),
      QRegExp("*.txt", Qt::CaseInsensitive, QRegExp::Wildcard))
{
}

