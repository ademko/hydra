
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_SESSIONMANAGER_H__
#define __INCLUDED_WEXUS_SESSIONMANAGER_H__

#include <QMutex>
#include <QUuid>
#include <QMap>
#include <QVariant>

#include <wexus/TR1.h>
#include <wexus/Cookies.h>

namespace wexus
{
  class SessionManager;
}

/**
 * A map of UUIDs to sessions.
 * rename this, perhaps?
 *
 * @author Aleksander Demko
 */ 
class wexus::SessionManager
{
  public:
    class Data
    {
      public:
        // called by Session, used to protect fieldValues
        QMutex mutex;
        QVariantMap fieldValues;
    };

  /**
   * Holds a lock on session parameters.
   *
   * @author Aleksander Demko
   */ 
  class Locker
  {
    public:
      /**
       * Empty ctor.
       *
       * @author Aleksander Demko
       */ 
      Locker(void);
      /**
       * Copy ctor: TRANSFERS ownership of the session.
       *
       * @author Aleksander Demko
       */ 
      Locker(const Locker &rhs);
      /**
       * Creates a session bound to the given Data.
       *
       * @author Aleksander Demko
       */ 
      Locker(std::shared_ptr<Data> data);
      /**
       * Creates an orphan session for the given id.
       *
       * @author Aleksander Demko
       */ 
      Locker(SessionManager *sesman, const QUuid &orphanId);

      /// destructor
      ~Locker();

      /// gets the linked QVariantMap of session values
      QVariantMap & map(void) const;

    private:
      /// parse the linked HTTPRequest, if it hasnt already
      //void parseRequest(void);

    private:
      SessionManager *dm_sesman;
      QUuid dm_orphanId;
      mutable std::shared_ptr<Data> dm_data;
  };

  public:
    /// ctor
    SessionManager(void);

    /**
     * Gets a session based on the given id.
     * If one doesn't exist, a new one will be created and
     * returned.
     *
     * @author Aleksander Demko
     */ 
    Locker getData(const QUuid &id);

    /**
     * Extra the session by the UUID stored in the session cookie,
     * if any.
     *
     * @author Aleksander Demko
     */ 
    Locker getDataByCookie(Cookies &cookies);

    /**
     * Stores the given Data at the given ID.
     * this is used by Locker.
     *
     * @author Aleksander Demko
     */ 
    void putData(const QUuid &id, std::shared_ptr<Data> &dat);

  private:

    QMutex dm_maplock;    // locks the following:
    QMap<QUuid, std::shared_ptr<Data> > dm_map; //shared RW
};

#endif

