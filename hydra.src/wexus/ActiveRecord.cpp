
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
// ActiveRecord::RecordNotFound
//

ActiveRecord::RecordNotFound::RecordNotFound(void)
  : Exception("Record not found")
{
}

//
//
// ActiveRecord
//
//

ActiveRecord::ActiveRecord(ActiveClass *klass)
  : dm_class(klass)
{
  assert(dm_class);
}

ActiveRecord::~ActiveRecord()
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
  if (qy.lastError().isValid()) {
//assert(false);
    throw Exception("query error: " + qy.lastError().text());
  }
}

void ActiveRecord::order(const ActiveExpr & orderByExpr)
{
  dm_orderByExpr = orderByExpr;
}

void ActiveRecord::resetQuery(void)
{
  dm_query.reset();
}

void ActiveRecord::setQuery(std::shared_ptr<QSqlQuery> qry)
{
  dm_query = qry;
}

void ActiveRecord::all(void)
{
  where(ActiveExpr());
}

void ActiveRecord::where(const ActiveExpr & whereExpr)
{
  internalWhere(whereExpr, 0);
}

void ActiveRecord::find(const QVariant &keyVal)
{
  internalWhere(ActiveExpr::fromColumn(0) == keyVal, 1);
  if (!next())
    throw RecordNotFound();
}

bool ActiveRecord::exists(const QVariant &keyVal)
{
  internalWhere(ActiveExpr::fromColumn(0) == keyVal, 1);
  return next();
}

bool ActiveRecord::first(const ActiveExpr & whereExpr)
{
  internalWhere(whereExpr, 1);
  return next();
}

bool ActiveRecord::last(const ActiveExpr & whereExpr)
{
  internalWhere(whereExpr, -1);
  return next();
}

void ActiveRecord::create(void)
{
  resetQuery();

  QSqlQuery q(database());
  ActiveClass * klass = activeClass();
  QString s;

  s = "INSERT INTO " + klass->tableName() + " ("
    + klass->fieldsAsListSansTable() + ") VALUES ("
    + klass->questionsAsList() + ")";

qDebug() << s;
  q.prepare(s);

  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    q.bindValue(i, klass->fieldsVec()[i]->toVariant(this));
  }

  q.exec();
  check(q);
}

void ActiveRecord::save(void)
{
  //resetQuery();

  QSqlQuery q(database());
  ActiveClass * klass = activeClass();
  QString s;

  s = "UPDATE " + klass->tableName() + " SET ";

  for (int i=1; i<klass->fieldsVec().size(); ++i) {
    if (i>1)
      s += ", ";
    s += klass->fieldsVec()[i]->fieldName() + " = ?";
  }

  s += " WHERE " + klass->fieldsVec()[0]->fieldName() + " = ?";

qDebug() << s;
  q.prepare(s);

  for (int i=1; i<klass->fieldsVec().size(); ++i) {
    q.bindValue(i-1, klass->fieldsVec()[i]->toVariant(this));
  }
  q.bindValue(klass->fieldsVec().size()-1, klass->fieldsVec()[0]->toVariant(this));

  q.exec();
  check(q);
}

void ActiveRecord::destroy(void)
{
  QSqlQuery q(database());
  ActiveClass * klass = activeClass();
  QString s;

  s = "DELETE FROM " + klass->tableName() + " WHERE  "
      + klass->fieldsVec()[0]->fieldName() + " = ?";

  qDebug() << s;
  q.prepare(s);

  q.bindValue(0, klass->fieldsVec()[0]->toVariant(this));

  q.exec();
  check(q);
}

bool ActiveRecord::next(void)
{
  if (!dm_query.get())
    return false;

  if (!dm_query->next()) {
    dm_query.reset();
    return false;
  }

  ActiveClass * klass = activeClass();
  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    klass->fieldsVec()[i]->setVariant(this, dm_query->value(i));
  }

  return true;
}

void ActiveRecord::internalWhere(const ActiveExpr & whereExpr, int limit)
{
  resetQuery();

  std::shared_ptr<QSqlQuery> q(new QSqlQuery(database()));
  ActiveClass * klass = activeClass();
  QString s;

  s = "SELECT " + klass->fieldsAsList() + " FROM " + klass->tableName();

  if (!whereExpr.isNull()) {
    s += " WHERE ";
    whereExpr.buildString(*klass, s);
  }

  assert((limit >= 0 || !dm_orderByExpr.isNull()) && "[first() and last() require an order to be set]");

  if (!dm_orderByExpr.isNull()) {
    s += " ORDER BY ";
    dm_orderByExpr.buildString(*klass, s);

    if (limit == 1)
      s += " LIMIT 1";
    else if (limit == -1)
      s += " DESC LIMIT 1";
  }

qDebug() << s;

  q->prepare(s);

  if (!whereExpr.isNull())
    whereExpr.buildBinds(*klass, *this, *q);

  q->exec();
  check(*q);

  setQuery(q);
}

