
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Site.h>

#include <wexus/MimeTypes.h>

using namespace wexus;

//
//
// Site
//
//

Site::Site(const QString &siteDir, const wexus::HTTPParams &params)
  : dm_siteDir(siteDir), dm_httpparms(params),
    dm_filehandler(siteDir, FileHTTPHandler::IndexHtml|FileHTTPHandler::AutoDirIndex)
{
  dm_madeMimeTypes = MimeTypes::instance() == 0;
  if (dm_madeMimeTypes)
    new MimeTypes();    // instance not lost, its a singlton!

  dm_httpparms.setHandler(this);
  dm_httpserver = HTTPServer::factoryNew(dm_httpparms);
}

Site::~Site()
{
  // shutdown the server explicitly, just to be verbose
  dm_httpserver.reset();

  if (dm_madeMimeTypes)
    delete MimeTypes::instance();
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

void Site::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &rep)
{
  dm_filehandler.handleRequest(req, rep);
}

