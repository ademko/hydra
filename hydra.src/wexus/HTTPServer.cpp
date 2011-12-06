
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
  dm_numthreads = 4;
  dm_handler = 0;
}

void HTTPParams::setPort(int p)
{
  assert(p>0);
  dm_port = p;
}

void HTTPParams::setNumThreads(int t)
{
  assert(t>0);
  dm_numthreads = t;
}

void HTTPParams::setHandler(HTTPHandler *handler)
{
  dm_handler = handler;
}

//
// HTTPServer::Exception
//

/*HTTPServer::Exception::Exception(void)
{
}*/

HTTPServer::Exception::Exception(const QString &logmsg)
  : wexus::Exception(logmsg), dm_logmsg(logmsg)
{
}

HTTPServer::Exception::~Exception() throw ()
{
}

//
// HTTPServer::PortInUseException
//

HTTPServer::PortInUseException::PortInUseException(int port)
  : Exception("Server port in use: " + QString::number(port))
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

