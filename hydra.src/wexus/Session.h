
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
  class Session;
  class SessionManager;
}

/**
 * Parses and holds session parameters.
 * The 
 *
 * @author Aleksander Demko
 */ 
class wexus::Session
{
  public:
    class Data
    {
      public:
        // called by Session, used to protect fieldValues
        QMutex mutex;
        typedef QMap<QString, QVariant> FieldValues;
        FieldValues fieldValues;
    };

  public:
    /**
     * Creates a session bound to the given Data.
     * Future feature: disconnected sessions for when
     * a session is not needed??
     *
     * @author Aleksander Demko
     */ 
    Session(std::shared_ptr<Data> data);

    /// destructor
    ~Session();

    /**
     * Does this session set have a particular field?
     *
     * @author Aleksander Demko
     */ 
    bool contains(const QString &fieldName);

    /**
     * Gets a field, const-version
     * Returns an invalid QVariant on not found
     * (perhaps it should throw an exception instead?)
     *
     * @author Aleksander Demko
     */ 
    const QVariant & operator[](const QString &fieldName) const;

    /**
     * Gets a field.
     * Returns a new variable if not found.
     * Returns an invalid QVariant on not found
     * (perhaps it should throw an exception instead?)
     *
     * @author Aleksander Demko
     */ 
    QVariant & operator[](const QString &fieldName);

  private:
    /// parse the linked HTTPRequest, if it hasnt already
    void parseRequest(void);

  private:
    std::shared_ptr<Data> dm_data;
    QVariant dm_emptyvariant;
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
    std::shared_ptr<Session::Data> getData(const QUuid &id);

    /**
     * Extra the session by the UUID stored in the session cookie,
     * if any.
     *
     * @author Aleksander Demko
     */ 
    std::shared_ptr<Session::Data> getDataByCookie(Cookies &cookies);

  private:
    QMap<QUuid, std::shared_ptr<Session::Data> > dm_map;
};

#endif

