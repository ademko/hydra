
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTEXT_H__
#define __INCLUDED_WEXUS_CONTEXT_H__

#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>

namespace wexus
{
  /**
   * Returns the current output stream.
   *
   * @author Aleksander Demko
   */ 
  QTextStream & output(void);

  class Context;
}

/**
 * The context object sets up and maintains (during its lifetime)
 * various thread-local storage variables necesary for the other
 * global functions defined in this file.
 *
 * @author Aleksander Demko
 */ 
class wexus::Context
{
  public:
    /// constructor
    Context(wexus::HTTPRequest &req, wexus::HTTPReply &reply);
    /// destructor
    ~Context();

    static Context *instance(void);

    wexus::HTTPRequest & request(void) const { return dm_req; }
    wexus::HTTPReply & reply(void) const { return dm_reply; }

  private:
    wexus::HTTPRequest & dm_req;
    wexus::HTTPReply & dm_reply;
};

#endif

