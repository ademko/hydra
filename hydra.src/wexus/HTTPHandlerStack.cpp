
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTTPHandlerStack.h>

#include <algorithm>

#include <QDebug>

using namespace wexus;

HTTPHandlerStack::HTTPHandlerStack(void)
{
}

void HTTPHandlerStack::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  // im going to use i rather than iterators just incase someone, somewhere
  // decides to modify the stack from within a sub-handler (a nono)
  for (int i=0; i<dm_handlers.size(); ++i) {
    dm_handlers[i].second->handleRequest(req, reply);
    if (reply.hasReply())
      return; //this handler handled the request, stop scanning the stack
  }
}

void HTTPHandlerStack::addHandler(std::shared_ptr<wexus::HTTPHandler> handler, int prio)
{
  dm_handlers.push_back(priohandler_t(prio, handler));
  // BUG stable_sort (atleast under linux) seems to require this check:
  if (dm_handlers.size() > 1)
    std::stable_sort(dm_handlers.begin(), dm_handlers.end(), lessthan);
}

