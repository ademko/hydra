
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Cookies.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

Cookies::CookieNotFoundException::CookieNotFoundException(const QString &cookieName)
  : wexus::HTTPHandler::Exception("cookie not found: " + cookieName)
{
}

//
//
// Cookies
//
//

Cookies::Cookies(wexus::HTTPRequest *req, wexus::HTTPReply *rep)
{
  dm_req = req;
  dm_reply = rep;
}

bool Cookies::contains(const QString &cookieName)
{
  assert(dm_req);

  return dm_req->cookies().find(cookieName) != dm_req->cookies().end();
}

const QVariant & Cookies::operator[](const QString &cookieName) const
{
  assert(dm_req);
  assert(dm_reply);

  HTTPReply::ServerCookies::iterator jj = dm_reply->cookies().find(cookieName);

  if (jj == dm_reply->cookies().end()) {
    // check the client set too
    HTTPRequest::ClientCookies::const_iterator ii = dm_req->cookies().find(cookieName);

    if (ii == dm_req->cookies().end())
      throw CookieNotFoundException(cookieName);
    else
      return *ii;
  } else
    return jj->value;
}

QVariant & Cookies::operator[](const QString &cookieName)
{
  assert(dm_req);
  assert(dm_reply);

  HTTPReply::ServerCookies::iterator jj = dm_reply->cookies().find(cookieName);
  
  if (jj == dm_reply->cookies().end()) {
    // create a new server cookie
    HTTPReply::ServerCookie newcook;

    // do we have anything to initialize it with?
    HTTPRequest::ClientCookies::const_iterator ii = dm_req->cookies().find(cookieName);
    if (ii != dm_req->cookies().end())
      newcook.value = *ii;

    jj = dm_reply->cookies().insert(cookieName, newcook);
  }

  return jj->value;
}

