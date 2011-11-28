
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Site.h>

#include <assert.h>

#include <wexus/Application.h>
#include <wexus/MimeTypes.h>
//#include <wexus/Context.h>

using namespace wexus;

//
// Site::ApplicationHandler
//

class Site::ApplicationHandler : public HTTPHandler
{
  public:
    ApplicationHandler(const QString &mountpoint, std::shared_ptr<Application> app);

    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  private:
    QString dm_mountpoint;
    QString dm_shorted; //dm_mountpoint but without the trail /, for speed :)
    std::shared_ptr<Application> dm_app;
};

Site::ApplicationHandler::ApplicationHandler(const QString &mountpoint, std::shared_ptr<Application> app)
  : dm_mountpoint(mountpoint), dm_app(app)
{
  assert(!dm_mountpoint.isEmpty());
  assert(dm_mountpoint[dm_mountpoint.size()-1] == '/');

  if (!dm_mountpoint.isEmpty())
    dm_shorted = dm_mountpoint.left(dm_mountpoint.size()-1);
}

void Site::ApplicationHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  if (req.request().startsWith(dm_mountpoint) || req.request() == dm_shorted) {
    // clip off the mount point in the request
    //Context ctx(req, reply);

    QString filteredReq = "/" + req.request().mid(dm_mountpoint.size());

    dm_app->handleApplicationRequest(filteredReq, req, reply);
  }
}

//
//
// Site
//
//

Site::Site(const QString &siteDir, const wexus::HTTPParams &params)
  : dm_siteDir(siteDir), dm_httpparms(params)
{
  dm_madeMimeTypes = MimeTypes::instance() == 0;
  if (dm_madeMimeTypes)
    new MimeTypes();    // instance not lost, its a singlton!

  dm_httpparms.setHandler(this);
  dm_httpserver = HTTPServer::factoryNew(dm_httpparms);

  //dont do this as a default anymore, its unsafe... let the frontend demand it:
  //dm_filehandler.reset(new FileHTTPHandler(siteDir, FileHTTPHandler::IndexHtml|FileHTTPHandler::AutoDirIndex));
  //addHandler(dm_filehandler, 100);
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

void Site::addApplication(const QString &mountpoint, std::shared_ptr<Application> app)
{
  assert(!mountpoint.isEmpty());
  assert(mountpoint[mountpoint.size()-1] == '/');

  app->setMountPoint(mountpoint);

  addHandler(std::shared_ptr<ApplicationHandler>(new ApplicationHandler(mountpoint, app)));
}

