
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
 * A per-thread class that managers all the open QSqlDatabase
 * connections held by a thread.
 *
 * We need to use something like this because QSqlDatabase objects
 * can only be used in the threads that created them, hense each thread
 * needs its own set.
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
     * This class is copyable.
     *
     * @author Aleksander Demko
     */ 
    class Handle
    {
      public:
        Handle(void);
        Handle(const QString &filename);

        /**
         * Gets the QSqlDatabase from this threads's TLS.
         *
         * @author Aleksander Demko
         */ 
        QSqlDatabase & database(void);

      private:
        QString dm_filename;
    };

  public:
    /// destructor
    ~OpenDatabases();

    /**
     * Returns the "static-like" instance for this thread.
     *
     * @author Aleksander Demko
     */ 
    static OpenDatabases *threadInstance(void);

    /// uses threadInstance
    static QSqlDatabase & database(const QString &filename);

  protected:
    /// constructor, called by threadInstance when needed
    OpenDatabases(void);

  private:
    typedef std::shared_ptr<QSqlDatabase> data_t;
    typedef QMap<QString, data_t> map_t;

    map_t dm_map;
};

#endif

