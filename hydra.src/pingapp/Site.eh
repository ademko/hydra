
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGSITE_H__
#define __INCLUDED_WEBAPPS_PINGSITE_H__

#include <wexus/ActiveRecord.h>

namespace pingapp
{
  class Site;
}

class pingapp::Site : public wexus::ActiveRecord
{
  fields:   // begin parsing of fields, will do all replacement here too
    pingapp::Host has_many;
    QString name(validate::notEmptyLength());
    QString desc = "default description";
    int id; //comment normally not listed last, just checking if it works
};


#endif

