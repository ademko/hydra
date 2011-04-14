
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <wexus/HTTP.h>
#include <wexus/Site.h>

class TestHandler : public wexus::HTTPHandler
{
  public:
    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

};

void TestHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  reply.setContentType("text/plain");
  reply.output() << "hello, world!\n\n"
    << "request = " << req.request() << "\n"
    << "query = " << req.query() << "\n"
    << "referer = " << req.referer() << "\n"
    << "userAgent = " << req.userAgent() << "\n";
}

int main(void)
{
  TestHandler h;

 /* wexus::HTTPParams params;
  params.setHandler(&h);

  wexus::MongooseServer server(params);*/
  wexus::Site s(".");

  s.start();
  s.wait();
}

