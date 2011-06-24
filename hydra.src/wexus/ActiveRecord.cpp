
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveRecord.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

//
//
// ActiveClass
//
//

ActiveClass::ActiveField::ActiveField(const QString &fieldName, const QString &fieldType)
  : dm_fieldName(fieldName), dm_fieldType(fieldType)
{
}

ActiveClass::ActiveClass(const QString &_className)
  : dm_classname(_className)
{
}

void ActiveClass::addField(const QString &fieldName, const QString &fieldType)
{
  std::shared_ptr<ActiveField> f(new ActiveField(fieldName, fieldType));

  dm_vec.push_back(f);
  dm_map[fieldName] = f;
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

