
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/SessionManager.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

//
//
// Session
//
//

SessionManager::Locker::Locker(void)
  : dm_sesman(0)
{
}

SessionManager::Locker::Locker(const Locker &rhs)
  : dm_sesman(0), dm_orphanId(rhs.dm_orphanId), dm_data(rhs.dm_data)
{
  // transfer by nuking the source ptr
  rhs.dm_data.reset();
}

SessionManager::Locker::Locker(std::shared_ptr<SessionManager::Data> data)
  : dm_sesman(0), dm_data(data)
{
  assert(dm_data.get());

  dm_data->mutex.lock();
}

SessionManager::Locker::Locker(SessionManager *sesman, const QUuid &orphanId)
  : dm_sesman(sesman), dm_orphanId(orphanId), dm_data(new Data)
{
  dm_data->mutex.lock();
}

SessionManager::Locker::~Locker()
{
  if (dm_data.get() == 0)
    return; // no data

  bool isEmpty = dm_data->fieldValues.isEmpty();

  dm_data->mutex.unlock();

  if (!isEmpty && dm_sesman) {
    assert(!dm_orphanId.isNull());
    // this an orphan, insert it
    assert(dm_sesman);
    dm_sesman->putData(dm_orphanId, dm_data);
  }
}

QVariantMap & SessionManager::Locker::map(void) const
{
  assert(dm_data.get());
  return dm_data->fieldValues;
}

//
//
// SessionManager
//
//

SessionManager::SessionManager(void)
{
}

SessionManager::Locker SessionManager::getData(const QUuid &id)
{
  QMutexLocker L(&dm_maplock);

  if (dm_map.contains(id))
    return Locker(dm_map[id]);

  // return a orphan-placeholder Locker
  return Locker(this, id);
}

SessionManager::Locker SessionManager::getDataByCookie(Cookies &cookies)
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

void SessionManager::putData(const QUuid &id, std::shared_ptr<Data> &dat)
{
  QMutexLocker L(&dm_maplock);

  dm_map[id] = dat;
}

