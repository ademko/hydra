
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/OpenDatabases.h>

#include <assert.h>

#include <QDebug>

#include <wexus/HTTPServer.h>

using namespace wexus;

OpenDatabases::Handle::Handle(void)
{
}

OpenDatabases::Handle::Handle(const std::shared_ptr<QSqlDatabase> &db)
  : dm_db(db)
{
}

OpenDatabases::Handle::~Handle()
{
  if (dm_db.get() && dm_db.unique() && dm_db->isOpen()) {
    QString connname(dm_db->connectionName());

    dm_db->close();

    dm_db.reset(); //dm_db = QSqlDatabase(); // we need to "null" dm_db otherwise removeDatabasew ill think its open

    QSqlDatabase::removeDatabase(connname);
  }
}

OpenDatabases * OpenDatabases::dm_instance;

OpenDatabases::OpenDatabases(void)
{
  assert(dm_instance == 0);
  dm_instance = this;
}

OpenDatabases::~OpenDatabases()
{
  assert(dm_instance == this);
  dm_instance = 0;
}

OpenDatabases * OpenDatabases::instance(void)
{
  return dm_instance;
}

static std::shared_ptr<QSqlDatabase> makeDB(const QString &filename)
{
  std::shared_ptr<QSqlDatabase> ret(new QSqlDatabase);

  *ret = QSqlDatabase::addDatabase("QSQLITE", filename);

  if (!ret->isValid())
    throw HTTPServer::Exception("OpenDatabases: can't addDatabase: " + filename);

  ret->setDatabaseName(filename);
  bool good = ret->open();

  if (!good)
    throw HTTPServer::Exception("OpenDatabases: can't open(): " + filename);

  return ret;
}

OpenDatabases::Handle OpenDatabases::database(const QString &filename)
{
  assert(dm_instance);

  if (!dm_instance->dm_map.contains(filename)) {
    std::shared_ptr<QSqlDatabase> db(makeDB(filename));
    dm_instance->dm_map[filename] = db;
    return Handle(db);
  } else {
    std::shared_ptr<QSqlDatabase> db(dm_instance->dm_map[filename]);

    if (!db.get()) {
      db = makeDB(filename);
      dm_instance->dm_map[filename] = db;
    }

    return Handle(db);
  }
}

