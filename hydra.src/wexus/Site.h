
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_SITE_H__
#define __INCLUDED_WEXUS_SITE_H__

#include <QString>

#include <wexus/HTTPHandlerStack.h>
#include <wexus/HTTPServer.h>
#include <wexus/FileHTTPHandler.h>

namespace wexus
{
  class Site;

  class Application; //fwd
}

/**
 * This is the core engine class of wexus.
 * A wexus::Site represents 1 or more running wexus::Application 
 * instanced groupped with a wexus::HTTPServer.
 *
 * This will instante certain singltons too, if needed:
 * MimeTypes.
 *
 * @author Aleksander Demko
 */ 
class wexus::Site : public wexus::HTTPHandlerStack
{
  public:
    // exceptions
    class error {};
  public:
    /**
     * Creates site instance based on the given siteDir.
     * The siteDir can be empty or already populated with a site.
     *
     * Exceptions are thrown on errors.
     *
     * You should call start() when you want to start the internal httpserver.
     *
     * @author Aleksander Demko
     */ 
    Site(const QString &siteDir, const wexus::HTTPParams &params = wexus::HTTPParams());
    /// destructor
    ~Site();

    /// is the web server currently running?
    bool isRunning(void) const;
    /// starts the web server
    void start(void);
    /// notifies the web server to stop. you should still do a wait()
    /// this function may or may not block
    void quit(void);
    /// waits until the server finishes
    void wait(void);

    /**
     * Adds an already instantiated application to the given mount point.
     * The mountpoint must end in a '/'. The root mountpoint, "/" is fine too.
     *
     * @author Aleksander Demko
     */ 
    void addApplication(const QString &mountpoint, std::shared_ptr<wexus::Application> app);

  private:
    // all members here are RO (readonly)
    // that is, set during startup in the main thread
    bool dm_madeMimeTypes;

    QString dm_siteDir;
    wexus::HTTPParams dm_httpparms;
    std::shared_ptr<wexus::HTTPServer> dm_httpserver;

    // helper handlers
    std::shared_ptr<wexus::FileHTTPHandler> dm_filehandler;

    class ApplicationHandler;
};

#endif

