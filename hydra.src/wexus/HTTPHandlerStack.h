
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_HTTPHANDLERSTACK_H__
#define __INCLUDED_WEXUS_HTTPHANDLERSTACK_H__

#include <vector>

#include <wexus/TR1.h>
#include <wexus/HTTPHandler.h>

namespace wexus
{
  class HTTPHandlerStack;
}

/**
 * A handler that maintains a list of other handlers.
 * When it receives a request, it goes down this list of handlers
 * until one of them processes the event.
 *
 * @author Aleksander Demko
 */ 
class wexus::HTTPHandlerStack : public wexus::HTTPHandler
{
  public:
    /// constructor
    HTTPHandlerStack(void);

    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

    /**
     * Adds a handler to the end of the handler stack.
     *
     * The stack is still sorted by priority, with lower priority
     * handlers.
     *
     * priorities should usually be between [1..99].
     * 
     * @author Aleksander Demko
     */ 
    void addHandler(std::shared_ptr<wexus::HTTPHandler> handler, int prio = 50);

  private:
    typedef std::pair<int, std::shared_ptr<wexus::HTTPHandler> > priohandler_t;
    static bool lessthan(const priohandler_t &left, const priohandler_t &right)
      { return left.first < right.first; }

    typedef std::vector<priohandler_t> handlers_t;

    handlers_t dm_handlers;
};

#endif

