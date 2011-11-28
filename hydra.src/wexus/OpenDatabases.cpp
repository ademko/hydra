
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/OpenDatabases.h>

#include <assert.h>

#include <QThread>
#include <QThreadStorage>
#include <QDebug>

#include <wexus/HTTPServer.h>

using namespace wexus;

static QThreadStorage<OpenDatabases *> Storage;

OpenDatabases::Handle::Handle(void)
{
}

OpenDatabases::Handle::Handle(const QString &filename)
  : dm_filename(filename)
{
}

QSqlDatabase & OpenDatabases::Handle::database(void)
{
  return OpenDatabases::threadInstance()->database(dm_filename);
}

OpenDatabases::OpenDatabases(void)
{
}

OpenDatabases::~OpenDatabases()
{
  // manually remove all the open databases
  for (map_t::iterator ii=dm_map.begin(); ii != dm_map.end(); ++ii) {
    QString connname((*ii)->connectionName());

    (*ii)->close();

    (*ii).reset(); //dm_db = QSqlDatabase(); // we need to "null" dm_db otherwise removeDatabasew ill think its open

    QSqlDatabase::removeDatabase(connname);
  }
}

OpenDatabases * OpenDatabases::threadInstance(void)
{
  OpenDatabases *ret = Storage.localData();

  if (!ret) {
    ret = new OpenDatabases;
    Storage.setLocalData(ret);
  }

  return ret;
}

static std::shared_ptr<QSqlDatabase> makeDB(const QString &filename)
{
  std::shared_ptr<QSqlDatabase> ret(new QSqlDatabase);
  // make the connection name a combination of threadid + filename
  QString connname = QString::number(reinterpret_cast<quintptr>(
        QThread::currentThread())) + " " + filename;

  *ret = QSqlDatabase::addDatabase("QSQLITE", connname);

  if (!ret->isValid())
    throw HTTPServer::Exception("OpenDatabases: can't addDatabase: " + filename);

  ret->setDatabaseName(filename);
  bool good = ret->open();

  if (!good)
    throw HTTPServer::Exception("OpenDatabases: can't open(): " + filename);

  return ret;
}

QSqlDatabase & OpenDatabases::database(const QString &filename)
{
  OpenDatabases *instance = threadInstance();

  assert(instance);

  if (!instance->dm_map.contains(filename)) {
    std::shared_ptr<QSqlDatabase> db(makeDB(filename));
    instance->dm_map[filename] = db;

    return *db;
  } else {
    std::shared_ptr<QSqlDatabase> db(instance->dm_map[filename]);

    if (!db.get()) {
      db = makeDB(filename);
      instance->dm_map[filename] = db;
    }

    return *db;
  }
}

