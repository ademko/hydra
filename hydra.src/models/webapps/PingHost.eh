
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEBAPPS_PINGHOST_H__
#define __INCLUDED_WEBAPPS_PINGHOST_H__

#include <wexus/ActiveRecord.h>

namespace webapps
{
  class PingHost;
}

class webapps::PingHost : public wexus::ActiveRecord
{
  fields:   // begin parsing of fields, will do all replacement here too
    int id; //comment
    QString host;
};


#endif

