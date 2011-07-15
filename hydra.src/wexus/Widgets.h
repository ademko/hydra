
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_WIDGETS_H__
#define __INCLUDED_WEXUS_WIDGETS_H__

#include <wexus/HTMLString.h>

namespace wexus
{
  /**
   * Returns the HTML code for a link to rawurl with the given
   * description.
   *
   * @author Aleksander Demko
   */ 
  wexus::HTMLString linkTo(const QString &desc, const QString &rawurl);
}

#endif

