
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTTPServer.h>

#include <assert.h>

#include <wexus/MongooseServer.h>

using namespace wexus;

//
//
// HTTPParams
//
//

HTTPParams::HTTPParams(void)
{
  dm_port = 8080;
  dm_handler = 0;
}

void HTTPParams::setPort(int p)
{
  assert(p>0);
  dm_port = p;
}

void HTTPParams::setHandler(HTTPHandler *handler)
{
  dm_handler = handler;
}

//
// HTTPException
//

HTTPException::HTTPException(void)
{
}

HTTPException::HTTPException(const QString &usermsg)
  : dm_usermsg(usermsg)
{
}

HTTPException::~HTTPException() throw ()
{
}

//
//
// HTTPServer
//
//

HTTPServer::HTTPServer(void)
{
}

HTTPServer::~HTTPServer()
{
}

std::shared_ptr<wexus::HTTPServer> wexus::HTTPServer::factoryNew(const wexus::HTTPParams &params)
{
  return std::shared_ptr<wexus::HTTPServer>(new wexus::MongooseServer(params));
}

