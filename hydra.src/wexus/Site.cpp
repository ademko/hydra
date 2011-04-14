
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Site.h>

using namespace wexus;

//
//
// Site
//
//

Site::Site(const QString &siteDir, const wexus::HTTPParams &params)
  : dm_siteDir(siteDir), dm_httpparms(params)
{
  dm_httpparms.setHandler(this);
  dm_httpserver = HTTPServer::factoryNew(dm_httpparms);
}

Site::~Site()
{
  // shutdown the server explicitly, just to be verbose
  dm_httpserver.reset();
}

bool Site::isRunning(void) const
{
  return dm_httpserver->isRunning();
}

void Site::start(void)
{
  dm_httpserver->start();
}

void Site::quit(void)
{
  dm_httpserver->quit();
}

void Site::wait(void)
{
  dm_httpserver->wait();
}

bool Site::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  reply.output() << "Hello, from Site";
  return true;
}

