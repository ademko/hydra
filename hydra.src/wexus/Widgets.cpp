
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Widgets.h>

using namespace wexus;

HTMLString wexus::linkTo(const QString &desc, const QString &rawurl)
{
  return HTMLString("<A HREF=\"" + rawurl + "\">" + HTMLString::encode(desc) + "</A>");
}

