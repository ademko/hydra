
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_IDABLE_H__
#define __INCLUDED_WEXUS_IDABLE_H__

namespace wexus
{
  class IDAble;
}

/**
 * A common class for ActiveFile and ActiveRecord
 * for linkTo.
 *
 * @author Aleksander Demko
 */ 
class wexus::IDAble
{
  public:
    virtual QVariant getIDAsVariant(void) = 0;
};

#endif

