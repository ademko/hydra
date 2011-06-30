
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveRecord.h>

#include <assert.h>

#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <wexus/Context.h>
#include <wexus/Application.h>

using namespace wexus;

//
// ActiveRecord::Exception
//

ActiveRecord::Exception::Exception(const QString &_what) throw()
  : dm_what(_what.toUtf8())
{
}

ActiveRecord::Exception::~Exception() throw()
{
}

//
//
// ActiveRecord
//
//

ActiveRecord::ActiveClassMap * ActiveRecord::dm_manager;

ActiveRecord::ActiveRecord(void)
{
}

QSqlDatabase & ActiveRecord::database(void)
{
  QSqlDatabase &db = Context::instance()->application()->database();

  if (!db.isOpen())
    throw Exception("DB is closed");

  return db;
}

void ActiveRecord::check(const QSqlQuery &qy)
{
  if (qy.lastError().isValid())
    throw Exception("query error: " + qy.lastError().text());
}

std::shared_ptr<ActiveClass> ActiveRecord::activeClass(void)
{
  if (!dm_class.get())
    initClass();

  assert(dm_class.get());

  return dm_class;
}

void ActiveRecord::setActiveClass(const QString &className, bool &hadToCreate)
{
  assert(!dm_class.get());

  if (!dm_manager)
    dm_manager = new ActiveClassMap;
  assert(dm_manager);

  ActiveClassMap::iterator ii = dm_manager->find(className);

  hadToCreate = ii == dm_manager->end();

  if (hadToCreate)
    ii = dm_manager->insert(className, std::shared_ptr<ActiveClass>(new ActiveClass(className)));

  dm_class = *ii;

  assert(dm_class.get());
}

void ActiveRecord::all(void)
{
  /*qDebug() << "RUNNING QUERY, drivers: " << QSqlDatabase::drivers();

  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "C1");

    assert(db.isValid());

    db.setDatabaseName("/tmp/one.sqlite");

    bool good = db.open();

    assert(good);

    db.close();
  }

  qDebug() << "STILL open dbs: " << QSqlDatabase::connectionNames();

  QSqlDatabase::removeDatabase("C1");

  qDebug() << "STILL open dbs (after close): " << QSqlDatabase::connectionNames();*/
}

void ActiveRecord::insert(void)
{
  QSqlQuery q(database());
  std::shared_ptr<ActiveClass> klass = activeClass();
  QString s;

  s = "INSERT INTO " + klass->tableName() + " ("
    + klass->fieldsAsList() + ") VALUES ("
    + klass->questionsAsList() + ")";

  //qDebug() << s;
  q.prepare(s);

  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    q.bindValue(i, klass->fieldsVec()[i]->toVariant(this));
  }

  q.exec();
  check(q);
}

