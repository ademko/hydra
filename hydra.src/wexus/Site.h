
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_SITE_H__
#define __INCLUDED_WEXUS_SITE_H__

#include <QString>

#include <wexus/HTTP.h>
#include <wexus/HTTPServer.h>

namespace wexus
{
  class Site;
}

/**
 * This is the core engine class of wexus.
 * A wexus::Site represents 1 or more running wexus::Application 
 * instanced groupped with a wexus::HTTPServer.
 *
 * @author Aleksander Demko
 */ 
class wexus::Site : public wexus::HTTPHandler
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

    // implemented for HTTPHandler
    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  private:
    QString dm_siteDir;
    wexus::HTTPParams dm_httpparms;
    std::shared_ptr<wexus::HTTPServer> dm_httpserver;
};

#endif

