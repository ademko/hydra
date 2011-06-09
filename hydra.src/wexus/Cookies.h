
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_COOKIES_H__
#define __INCLUDED_WEXUS_COOKIES_H__

#include <wexus/HTTP.h>

namespace wexus
{
  class Cookies;
}

/**
 * This is collection of cookies, string values
 * mapped by string keys.
 * Internally, it uses the ClientCookies and ServerCookies
 * of HTTPRequest and HTTPReply.
 *
 * @author Aleksander Demko
 */ 
class wexus::Cookies
{
  public:
    class CookieNotFoundException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        CookieNotFoundException(const QString &cookieName);
    };

  public:
    /**
     * Constructor
     * TODO future, add Site so the domain/expires/path
     * can be set.
     *
     * @author Aleksander Demko
     */ 
    Cookies(wexus::HTTPRequest *req, wexus::HTTPReply *rep);

    /**
     * Does this have a cookie with the given name?
     *
     * @author Aleksander Demko
     */ 
    bool contains(const QString &cookieName);

    /**
     * Gets a cookie.
     * Throws CookieNotFoundException on not found.
     *
     * @author Aleksander Demko
     */ 
    const QVariant & operator[](const QString &cookieName) const;

    /**
     * Gets a cookie.
     * Never fails, as it'll create the cookie if need be.
     *
     * @author Aleksander Demko
     */ 
    QVariant & operator[](const QString &cookieName);

  private:
    wexus::HTTPRequest *dm_req;
    wexus::HTTPReply *dm_reply;
};

#endif

