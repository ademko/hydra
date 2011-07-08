
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveClass.h>

#include <QSqlQuery>

#include <wexus/ActiveRecord.h>

using namespace wexus;

//
// ActiveField
//

ActiveClass::ActiveField::ActiveField(const QString &fieldName, const QString &fieldType)
  : dm_fieldName(fieldName), dm_fieldType(fieldType)
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
}

void ActiveClass::createTable(void)
{
  QSqlDatabase &db = ActiveRecord::database();

  QString q = "CREATE TABLE " + tableName() + "(";

  for (int i=0; i<dm_vec.size(); ++i) {
    if (i>0)
      q += ",";

    q += dm_vec[i]->fieldName() + " " + dm_vec[i]->sqlFieldType();

    if (i == 0)
      q += " PRIMARY KEY";
  }

  q += ")";

//qDebug() << "creating table" << q;
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

