
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
  : dm_class(klass), dm_filtercol(-1)
{
  assert(dm_class);

  //clear();    // this must be called by decendants
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

void ActiveRecord::clear(void)
{
  ActiveClass * klass = activeClass();

  for (int i=0; i<klass->fieldsVec().size(); ++i)
    klass->fieldsVec()[i]->setVariant(this, QVariant());
}

void ActiveRecord::check(bool b, const QString &exceptionMsg)
{
  if (!b) {
    throw Exception(exceptionMsg);
  }
}

void ActiveRecord::order(const ActiveExpr & orderByExpr)
{
  dm_orderByExpr = orderByExpr;
}

void ActiveRecord::setFilterColumn(int colindex)
{
  assert(colindex>=0 && colindex<activeClass()->fieldsVec().size());

  dm_filtercol = colindex;
}

ActiveExpr ActiveRecord::filterExpr(const ActiveExpr &e)
{
  if (dm_filtercol == -1)
    return e;
assert(false);

  ActiveClass * klass = activeClass();

  ActiveExpr fexp(ActiveExpr::fromColumn(dm_filtercol) == klass->fieldsVec()[dm_filtercol]->toVariant(this));

  if (e.isNull())
    return fexp;
  else
    return fexp && e;
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
  ActiveClass * klass = activeClass();

  internalWhere(ActiveExpr::fromColumn(klass->keyColumn()) == keyVal, 1);

  if (!next())
    throw RecordNotFound();
}

bool ActiveRecord::exists(const QVariant &keyVal)
{
  ActiveClass * klass = activeClass();

  internalWhere(ActiveExpr::fromColumn(klass->keyColumn()) == keyVal, 1);

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
  check( q.prepare(s) , "ActiveRecord::create() prepare() failed");

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
  bool doneone;

  s = "UPDATE " + klass->tableName() + " SET ";

  doneone = false;
  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    if (klass->fieldsVec()[i]->style() == ActiveClass::keyStyle)
      continue;
    if (doneone)
      s += ", ";
    else
      doneone = true;
    s += klass->fieldsVec()[i]->fieldName() + " = ?";
  }

  s += " WHERE ";

  doneone = false;
  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    if (klass->fieldsVec()[i]->style() != ActiveClass::keyStyle)
      continue;
    if (doneone)
      s += "AND ";  // future multi-key support?
    else
      doneone = true;
    s += klass->fieldsVec()[0]->fieldName() + " = ?";
  }

qDebug() << s;
  check( q.prepare(s) , "ActiveRecord::save() prepare() failed");

  for (int i=0; i<klass->fieldsVec().size(); ++i)
    if (klass->fieldsVec()[i]->style() != ActiveClass::keyStyle)
      q.addBindValue(klass->fieldsVec()[i]->toVariant(this));
  for (int i=0; i<klass->fieldsVec().size(); ++i)
    if (klass->fieldsVec()[i]->style() == ActiveClass::keyStyle)
      q.addBindValue(klass->fieldsVec()[i]->toVariant(this));

  q.exec();
  check(q);
}

void ActiveRecord::destroy(void)
{
  ActiveClass * klass = activeClass();

  deleteRows(ActiveExpr::fromColumn(klass->keyColumn()) == klass->fieldsVec()[0]->toVariant(this));
}

void ActiveRecord::destroy(const QVariant &keyVal)
{
  ActiveClass * klass = activeClass();

  deleteRows(ActiveExpr::fromColumn(klass->keyColumn()) == keyVal);
}

void ActiveRecord::deleteRows(const ActiveExpr & _whereExpr)
{
  QSqlQuery q(database());
  ActiveClass * klass = activeClass();
  QString s;
  ActiveExpr whereExpr(filterExpr(_whereExpr));

  s = "DELETE FROM " + klass->tableName();
  
  if (!whereExpr.isNull()) {
    s += " WHERE  ";
    whereExpr.buildString(*klass, s);
  }

qDebug() << s;
  check( q.prepare(s) , "ActiveRecord::deleteRows() prepare() failed");

  if (!whereExpr.isNull())
    whereExpr.buildBinds(*klass, *this, q);

  q.exec();
  check(q);
}

int ActiveRecord::count(const ActiveExpr & _whereExpr)
{
  QSqlQuery q(database());
  ActiveClass * klass = activeClass();
  QString s;
  ActiveExpr whereExpr(filterExpr(_whereExpr));

  s = "SELECT COUNT(*) FROM " + klass->tableName();
  
  if (!whereExpr.isNull()) {
    s += " WHERE  ";
    whereExpr.buildString(*klass, s);
  }

qDebug() << s;
  check( q.prepare(s) , "ActiveRecord::count() prepare() failed");

  if (!whereExpr.isNull())
    whereExpr.buildBinds(*klass, *this, q);

  q.exec();
  check(q);

  if (!q.next())
    throw Exception("ActiveRecord::count next() called failed");

  return q.value(0).toInt();
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

void ActiveRecord::internalWhere(const ActiveExpr & _whereExpr, int limit)
{
  resetQuery();

  std::shared_ptr<QSqlQuery> q(new QSqlQuery(database()));
  ActiveClass * klass = activeClass();
  QString s;
  ActiveExpr whereExpr(filterExpr(_whereExpr));

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

  check( q->prepare(s) , "ActiveRecord::internalWhere() prepare() failed");

  if (!whereExpr.isNull())
    whereExpr.buildBinds(*klass, *this, *q);

  q->exec();
  check(*q);

  setQuery(q);
}

