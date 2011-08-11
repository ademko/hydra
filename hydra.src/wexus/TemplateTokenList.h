
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_TEMPLATETOKENLIST_H__
#define __INCLUDED_WEXUS_TEMPLATETOKENLIST_H__

#include <QList>

#include <wexus/TR1.h>

#include <wexus/TemplateToken.h>

namespace wexus
{
  typedef QList<std::shared_ptr<wexus::TemplateToken> > TemplateTokenList;

  void compressList(TemplateTokenList &list);
}

#endif

