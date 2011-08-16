
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_APPLICATION_H__
#define __INCLUDED_WEXUS_APPLICATION_H__

#include <wexus/Controller.h>
#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>
#include <wexus/Registry.h>

#include <QSqlDatabase>

namespace wexus
{
  class Application;
}

/**
 * Controllers are groupped into applications. Applications
 * are the installed, at a certain mount point (prefix)
 * into a Site.
 *
 * Each user application decends from this class,
 * registers the application subclass via.
 * The application then registers controllers in 
 * its constructor via registerController.
 *
 * @author Aleksander Demko
 */ 
class wexus::Application
{
  public:
    /// destructor
    virtual ~Application();

    /**
     * Called by wexus::Site during addApplication
     *
     * @author Aleksander Demko
     */
    void setMountPoint(const QString &mountPoint);

    /**
     * Returns the mount point.
     * Mount points always end in /
     *
     * @author Aleksander Demko
     */
    const QString &mountPoint(void) const { return dm_mountpoint; }

    /**
     * A enhanced handleApplicationRequest() call from the wexus::Site to wexus::Application.
     * filteredRequest contains just the action call. It always starts with atleast a /
     *
     * This method is not typically overriden.
     *
     * @author Aleksander Demko
     */ 
    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

    SessionManager & sessionManager(void) { return dm_sessionmanager; }

    QSqlDatabase & database(void) { return dm_db; }

    std::shared_ptr<Registry::AppInfo> appInfo(void);

  protected:
    /// inherited constructor
    Application(void);

  private:
    QString dm_mountpoint;

    SessionManager dm_sessionmanager;
    QSqlDatabase dm_db;

    std::shared_ptr<Registry::AppInfo> dm_appinfo;

    class Route;
    class MatchingRoute;
    typedef QList<std::shared_ptr<Route> > RouteList;

    RouteList dm_routes;
};

#endif

