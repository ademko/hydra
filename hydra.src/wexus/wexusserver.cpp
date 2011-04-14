
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <wexus/HTTP.h>

class TestHandler : public wexus::HTTPHandler
{
  public:
    virtual bool handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

};

bool TestHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  reply.setContentType("text/plain");
  reply.output() << "hello, world!\n\n"
    << "request = " << req.request() << "\n"
    << "query = " << req.query() << "\n"
    << "referer = " << req.referer() << "\n"
    << "userAgent = " << req.userAgent() << "\n";

  return true;
}

int main(void)
{
  TestHandler h;

  wexus::HTTPParams params;
  params.setHandler(&h);

  wexus::MongooseServer server(params);

  server.start();
  server.wait();
}

