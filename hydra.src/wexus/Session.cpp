
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Session.h>

#include <assert.h>

using namespace wexus;

//
//
// Session
//
//

SessionLocker::SessionLocker(std::shared_ptr<SessionLocker::Data> data)
  : dm_data(data)
{
  assert(dm_data.get());

  dm_data->mutex.lock();
}

SessionLocker::~SessionLocker()
{
  dm_data->mutex.unlock();
}

//
//
// SessionManager
//
//

SessionManager::SessionManager(void)
{
}

std::shared_ptr<SessionLocker::Data> SessionManager::getData(const QUuid &id)
{
  if (dm_map.contains(id))
    return dm_map[id];

  // create it
  std::shared_ptr<SessionLocker::Data> ptr(new SessionLocker::Data);
  dm_map[id] = ptr;
  return ptr;
}

std::shared_ptr<SessionLocker::Data> SessionManager::getDataByCookie(Cookies &cookies)
{
  QUuid id;

  // get the cookie
  if (cookies.contains("WEXUS_SESSION_UUID"))
    id = QUuid(cookies["WEXUS_SESSION_UUID"].toString());

  // make a new id if needed
  if (id.isNull()) {
    id = QUuid::createUuid();
    cookies["WEXUS_SESSION_UUID"] = id.toString();
  }

  // finally, extract the session
  return getData(id);
}

