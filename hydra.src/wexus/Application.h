
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
#include <wexus/OpenDatabases.h>

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
     * Decendants may overide this to add additional functionality.
     *
     * @author Aleksander Demko
     */
    virtual void setMountPoint(const QString &mountPoint);

    /**
     * Returns the mount point.
     * Mount points always end in /
     *
     * @author Aleksander Demko
     */
    const QString &mountPoint(void) const { return dm_mountpoint; }

    /**
     * Called shortly after creation. This saves a copy
     * of the settings reference (acceisble via settings())
     * and also opens the database.
     *
     * Decendants may overide this to add additional functionality.
     * They should call this version, first, however.
     *
     * @author Aleksander Demko
     */
    virtual void init(const QVariantMap &settings);

    /**
     * This minimalist version of init() can be called by decendant's init functions
     * if instead of Application::init(). This version simply sets
     * the settings variable and does not perform any other
     * functions, like database initialization.
     *
     * @author Aleksander Demko
     */ 
    void initBasic(const QVariantMap &settings);

    /**
     * Returns the current settings
     *
     * Some interesting fields, assigned at boot:
     *   app= the appname of the started class
     *   mountpoint= the url mount point, same as mountPoint()
     *   sitedir= the directory (on disk) of the site
     *   appdir= the directory (on disk) of the app within the site
     *
     * @author Aleksander Demko
     */ 
    const QVariantMap & settings(void) const { return dm_settings; }

    /**
     * A enhanced handleApplicationRequest() call from the wexus::Site to wexus::Application.
     * filteredRequest contains just the action call. It always starts with atleast a /
     *
     * This method is not typically overriden, but can be if you want to layer over the
     * default controller-handler system.
     *
     * @author Aleksander Demko
     */ 
    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

    SessionManager & sessionManager(void) { return dm_sessionmanager; }

    QSqlDatabase & database(void) { return dm_db.database(); }

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
        void addStandardRoutes(void);

      protected:
        Application &dm_app;
    };

    /// inherited constructor
    Application(void);

    /// called by initSettings to open (or reopen) the db
    void openDB(void);

  private:
    // all members here are RO (readonly), unless otherwise noted
    QString dm_mountpoint;
    QVariantMap dm_settings;

    SessionManager dm_sessionmanager;
    OpenDatabases::Handle dm_db;

    std::shared_ptr<Registry::AppInfo> dm_appinfo;

    class Route;
    class MatchingRoute;
    typedef QList<std::shared_ptr<Route> > RouteList;

    RouteList dm_routes;
};

#endif

