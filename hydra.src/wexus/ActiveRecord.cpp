
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
#include <wexus/VarPath.h>
#include <wexus/AssertException.h>

using namespace wexus;

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
  QSqlDatabase &db = Context::application()->database();

  if (!db.isOpen())
    throw AssertException("DB is closed");

  return db;
}

void ActiveRecord::check(const QSqlQuery &qy)
{
  if (qy.lastError().isValid()) {
//assert(false);
    throw AssertException("query error: " + qy.lastError().text());
  }
}

QVariant ActiveRecord::getIDAsVariant(void)
{
  return activeClass()->keyField()->toVariant(this);
}

void ActiveRecord::clear(void)
{
  ActiveClass * klass = activeClass();

  for (int i=0; i<klass->fieldsVec().size(); ++i)
    if (i != dm_filtercol)
      klass->fieldsVec()[i]->setVariant(this, klass->fieldsVec()[i]->initVal());
}

void ActiveRecord::test(Context::Errors *outerrors) const
{
  ActiveClass * klass = activeClass();
  QStringList errorlist;

  for (int i=0; i<klass->fieldsVec().size(); ++i)
    if (!klass->fieldsVec()[i]->validationExpr().isNull()) {
      klass->fieldsVec()[i]->validationExpr().test(
          klass->fieldsVec()[i]->toVariant(this),
          outerrors ? &errorlist : 0);
      if (outerrors && !errorlist.isEmpty()) {
        (*outerrors)[klass->fieldsVec()[i]->fieldName()] = errorlist;
        errorlist.clear();
      }
    }
}

bool ActiveRecord::fromForm(const QVariant &v)
{
  // reset the record first
  clear();

  QVariant tableNameParams;

  if (!v.isValid()) {
    tableNameParams = Context::threadInstance()->params[activeClass()->tableName()];
    if (!tableNameParams.isValid())
      return false;
  }

  Context *c = Context::threadInstance();
  ActiveClass * klass = activeClass();

  const QVariantMap &m = asVariantMap(v.isValid()? v : tableNameParams);   // will throw if its not a map

  for (QVariantMap::const_iterator ii = m.begin(); ii != m.end(); ++ii) {
    if (!klass->fieldsMap().contains(ii.key()))
      throw HTTPHandler::Exception("unkown ActiveRecord field: " + ii.key());
    std::shared_ptr<ActiveClass::ActiveField> f = *klass->fieldsMap().find(ii.key());
    if (f->style() != ActiveClass::varStyle)
      throw HTTPHandler::Exception("not varStyle field: " + ii.key());    // dont overwrite any of the keys
    // finally, assign the form value to this DB record
    f->setVariant(this, *ii);
  }

  // finally, test them all
  test(&c->errors);

  return c->errors.isEmpty();
}

QString ActiveRecord::toString(void) const
{
  ActiveClass * klass = activeClass();
  QString r("[");

  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    if (i>0)
      r += ",";
    r += "\"" + klass->fieldsVec()[i]->toVariant(this).toString() + "\"";
  }

  r += "]";

  return r;
}

void ActiveRecord::setFilterColumn(int colindex)
{
  assert(colindex>=0 && colindex<activeClass()->fieldsVec().size());

  dm_filtercol = colindex;
}

void ActiveRecord::check(bool b, const QString &exceptionMsg)
{
  if (!b) {
    throw AssertException(exceptionMsg);
  }
}

void ActiveRecord::order(const ActiveExpr & orderByExpr)
{
  dm_orderByExpr = orderByExpr;
}

ActiveExpr ActiveRecord::filterExpr(const ActiveExpr &e)
{
  if (dm_filtercol == -1)
    return e;

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
    throw AssertException("Record not found: " + keyVal.toString());
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

void ActiveRecord::create(const QVariant &v)
{
  ActiveClass * klass = activeClass();
  QString s;

  resetQuery();

  bool autoinc = !v.isValid();
  QVariant next_id(v);
  int tries = 0;

  if (autoinc) {
    QSqlQuery a(database());
    s = "SELECT MAX(" + klass->fieldsVec()[klass->keyColumn()]->fieldName()
      + ") FROM " + klass->tableName();
qDebug() << s;

    a.exec(s);
    check(a);

    if (a.next())
      next_id = a.value(0);

    if (next_id.isNull())
      next_id = 0;
    next_id = next_id.toInt() + 1;
  }

  QSqlQuery q(database());

  s = "INSERT INTO " + klass->tableName() + " ("
    + klass->fieldsAsListSansTable() + ") VALUES ("
    + klass->questionsAsList() + ")";

qDebug() << s << toString();
  check( q.prepare(s) , "ActiveRecord::create() prepare() failed");

  for (int i=0; i<klass->fieldsVec().size(); ++i) {
    q.bindValue(i, klass->fieldsVec()[i]->toVariant(this));
  }

  bool good;
  do {
    klass->fieldsVec()[klass->keyColumn()]->setVariant(this, next_id);
    q.bindValue(klass->keyColumn(), next_id);

    good = q.exec();
    tries++;

    if (autoinc)
      next_id = next_id.toInt() + 1;  //prep if the next iteration, if any
  } while (autoinc && !good && tries < 5);

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

qDebug() << s << toString();
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

  destroyAll(ActiveExpr::fromColumn(klass->keyColumn()) == klass->fieldsVec()[klass->keyColumn()]->toVariant(this));
}

void ActiveRecord::destroy(const QVariant &keyVal)
{
  ActiveClass * klass = activeClass();

  destroyAll(ActiveExpr::fromColumn(klass->keyColumn()) == keyVal);
}

void ActiveRecord::destroyAll(const ActiveExpr & _whereExpr)
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
  check( q.prepare(s) , "ActiveRecord::destroyAll() prepare() failed");

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
    throw AssertException("ActiveRecord::count next() called failed");

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

qDebug() << s << toString();

  check( q->prepare(s) , "ActiveRecord::internalWhere() prepare() failed");

  if (!whereExpr.isNull())
    whereExpr.buildBinds(*klass, *this, *q);

  q->exec();
  check(*q);

  setQuery(q);
}

