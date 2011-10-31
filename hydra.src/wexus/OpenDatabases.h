
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_OPENDATABASES_H__
#define __INCLUDED_WEXUS_OPENDATABASES_H__

#include <QSqlDatabase>
#include <QMap>

#include <wexus/TR1.h>

namespace wexus
{
  class OpenDatabases;
}

/**
 * A singleton class that manages all the open databases.
 *
 * @author Aleksander Demko
 */ 
class wexus::OpenDatabases
{
  public:
    /**
     * Speciall wrapper around the smart pointer because
     * QSqlDatabase needs some extra closing logic.
     *
     * @author Aleksander Demko
     */ 
    class Handle
    {
      public:
        Handle(void);
        Handle(const std::shared_ptr<QSqlDatabase> &db);
        ~Handle();

        QSqlDatabase & database(void) { return *dm_db; }

      private:
        std::shared_ptr<QSqlDatabase> dm_db;
    };

  public:
    /// constructor
    OpenDatabases(void);
    /// destructor
    ~OpenDatabases();

    /**
     * Returns the singleton instance. May return null
     * if there is none.
     *
     * @author Aleksander Demko
     */ 
    static OpenDatabases *instance(void);

    static Handle database(const QString &filename);

  private:
    typedef std::weak_ptr<QSqlDatabase> data_t;
    typedef QMap<QString, data_t> map_t;

    static OpenDatabases *dm_instance;
    
    map_t dm_map;
};

#endif

