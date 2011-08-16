
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
    /**
     * Decendants can instantite this in their constructors
     * to build a routing table.
     *
     * Route will be matched in order, with the first ones
     * given first chance/priority.
     *
     * @author Aleksander Demko
     */ 
    class RouteBuilder
    {
      public:
        /**
         * Start building a new route for the given app.
         * Any previous routing table will be destroyed.
         *
         * @author Aleksander Demko
         */ 
        RouteBuilder(Application &app);

        /// destructor
        ~RouteBuilder();

        /**
         * Adds a basic match route.
         *
         * It typically looks like /blah/:var/?:id
         * Where blah would be matched directly, :var will be assigned to
         * a variable and ? is optional.
         *
         * @param matchString the match string
         * @param defaults the default values for any params. This will be
         * overriden with matched vars, ofcourse.
         *
         * @author Aleksander Demko
         */ 
        void addMatch(const QString &matchString, const QVariantMap &defaults = QVariantMap());

        /**
         * Adds a bunch of default routes.
         *
         * @author Aleksander Demko
         */
        void addDefault(void);

      protected:
        Application &dm_app;
    };

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

