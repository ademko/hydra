
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTTPServer.h>

#include <assert.h>

#include <wexus/MongooseServer.h>

//
//
// HTTPParams
//
//

wexus::HTTPParams::HTTPParams(void)
{
  dm_port = 8080;
  dm_handler = 0;
}

void wexus::HTTPParams::setPort(int p)
{
  assert(p>0);
  dm_port = p;
}

void wexus::HTTPParams::setHandler(HTTPHandler *handler)
{
  dm_handler = handler;
}

//
//
// HTTPServer
//
//

wexus::HTTPServer::HTTPServer(void)
{
}

wexus::HTTPServer::~HTTPServer()
{
}

std::shared_ptr<wexus::HTTPServer> wexus::HTTPServer::factoryNew(const wexus::HTTPParams &params)
{
  return std::shared_ptr<wexus::HTTPServer>(new wexus::MongooseServer(params));
}

