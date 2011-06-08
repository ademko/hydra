
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

Session::Session(std::shared_ptr<Session::Data> data)
  : dm_data(data)
{
  assert(dm_data.get());

  dm_data->mutex.lock();
}

Session::~Session()
{
  dm_data->mutex.unlock();
}

bool Session::has(const QString &fieldName)
{
  return dm_data->fieldValues.contains(fieldName);
}

const QVariant & Session::operator[](const QString &fieldName) const
{
  Data::FieldValues::const_iterator ii = dm_data->fieldValues.find(fieldName);

  if (ii == dm_data->fieldValues.end())
    return dm_emptyvariant;
  else
    return *ii;
}

QVariant & Session::operator[](const QString &fieldName)
{
  return dm_data->fieldValues[fieldName];
}

//
//
// SessionManager
//
//

SessionManager::SessionManager(void)
{
}

std::shared_ptr<Session::Data> SessionManager::getData(const QUuid &id)
{
  if (dm_map.contains(id))
    return dm_map[id];

  // create it
  std::shared_ptr<Session::Data> ptr(new Session::Data);
  dm_map[id] = ptr;
  return ptr;
}

std::shared_ptr<Session::Data> SessionManager::getDataByCookie(Cookies &cookies)
{
  QUuid id;

  // get the cookie
  if (cookies.has("WEXUS_SESSION_UUID"))
    id = QUuid(cookies["WEXUS_SESSION_UUID"].toString());

  // make a new id if needed
  if (id.isNull()) {
    id = QUuid::createUuid();
    cookies["WEXUS_SESSION_UUID"] = id.toString();
  }

  // finally, extract the session
  return getData(id);
}

