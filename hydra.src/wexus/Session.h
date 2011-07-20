
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_SESSION_H__
#define __INCLUDED_WEXUS_SESSION_H__

#include <QMutex>
#include <QUuid>
#include <QMap>
#include <QVariant>

#include <hydra/TR1.h>
#include <wexus/Cookies.h>

namespace wexus
{
  class SessionLocker;
  class SessionManager;
}

/**
 * Parses and holds session parameters.
 * The 
 *
 * @author Aleksander Demko
 */ 
class wexus::SessionLocker
{
  public:
    class Data
    {
      public:
        // called by Session, used to protect fieldValues
        QMutex mutex;
        QVariantMap fieldValues;
    };

  public:
    /**
     * Creates a session bound to the given Data.
     * Future feature: disconnected sessions for when
     * a session is not needed??
     *
     * @author Aleksander Demko
     */ 
    SessionLocker(std::shared_ptr<Data> data);

    /// destructor
    ~SessionLocker();

    /// gets the linked QVariantMap of session values
    QVariantMap & map(void) const { return dm_data->fieldValues; }

  private:
    /// parse the linked HTTPRequest, if it hasnt already
    //void parseRequest(void);

  private:
    std::shared_ptr<Data> dm_data;
};

/**
 * A map of UUIDs to sessions.
 * rename this, perhaps?
 *
 * @author Aleksander Demko
 */ 
class wexus::SessionManager
{
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
    std::shared_ptr<SessionLocker::Data> getData(const QUuid &id);

    /**
     * Extra the session by the UUID stored in the session cookie,
     * if any.
     *
     * @author Aleksander Demko
     */ 
    std::shared_ptr<SessionLocker::Data> getDataByCookie(Cookies &cookies);

  private:
    QMap<QUuid, std::shared_ptr<SessionLocker::Data> > dm_map;
};

#endif

