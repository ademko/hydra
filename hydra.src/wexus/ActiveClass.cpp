
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveClass.h>

#include <QSqlQuery>
#include <QDebug>

#include <wexus/ActiveRecord.h>

using namespace wexus;

//
// ActiveField
//

ActiveClass::ActiveField::ActiveField(int style, const QString &fieldName, const QString &fieldType)
  : dm_style(style), dm_fieldName(fieldName), dm_fieldType(fieldType)
{
}

//
//
// ActiveClass
//
//

QString ActiveClass::toSqlType(const QString &t)
{
  if (t == "int")
    return "INTEGER";
  if (t == "QString")
    return "TEXT";
  if (t == "double")
    return "REAL";
  throw ActiveRecord::Exception("Unkown C type: " + t);
}

ActiveClass::ActiveClass(const QString &_className)
  : dm_classname(_className), dm_tablename(_className)
{
  for (QString::iterator ii=dm_tablename.begin(); ii !=dm_tablename.end(); ++ii)
    if (*ii == ':')
      *ii = '_';
  dm_keycolumn = -1;
}

int ActiveClass::keyColumn(void) const
{
  assert(dm_keycolumn >= 0);
  return dm_keycolumn;
}

void ActiveClass::createTable(void) const
{
  QSqlDatabase &db = ActiveRecord::database();

  QString q = "CREATE TABLE " + tableName() + "(";

  for (int i=0; i<dm_vec.size(); ++i) {
    if (i>0)
      q += ",";

    q += dm_vec[i]->fieldName() + " " + dm_vec[i]->sqlFieldType();

    if (dm_vec[i]->style() != ActiveClass::fKeyStyle)
      q += " NOT NULL";   // non-fkeys cannot be null

    if (dm_vec[i]->style() == ActiveClass::keyStyle)
      q += " PRIMARY KEY";
  }

  q += ")";

qDebug() << q;
  //ActiveRecord::check(db.exec(q));

  // we don't check the results of this as it usual fails
  // (as the table often already exists)
  db.exec(q);
}

void ActiveClass::doneConstruction(void)
{
  int num = dm_vec.size();

  if (num == 0)
    throw ActiveRecord::Exception("ActiveClass::doneConstruction: no fields?");

  for (int i=0; i<num; ++i) {
    if (i>0) {
      dm_fieldsaslist += ", ";
      dm_fieldsaslistsanstable += ", ";
      dm_questionsaslist += ", ";
    }

    dm_fieldsaslist += tableName() + ".";

    dm_fieldsaslist += dm_vec[i]->fieldName();
    dm_fieldsaslistsanstable += dm_vec[i]->fieldName();
    dm_questionsaslist += "?";
  }
}

