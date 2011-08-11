
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MODELTOKENLIST_H__
#define __INCLUDED_WEXUS_MODELTOKENLIST_H__

#include <wexus/TR1.h>

#include <wexus/ModelToken.h>

#include <QList>

namespace wexus
{
  typedef QList<std::shared_ptr<wexus::ModelToken> > ModelTokenList;
}


#endif

