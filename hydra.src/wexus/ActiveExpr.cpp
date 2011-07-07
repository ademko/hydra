
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveExpr.h>

#include <wexus/ActiveClass.h>

using namespace wexus;

//
// ActiveExpr::Imp
//

class ActiveExpr::Imp
{
  public:
    /// virtual dtor
    virtual ~Imp();

    virtual void buildString(ActiveClass &klass, QString &out) const = 0;
};

ActiveExpr::Imp::~Imp()
{
}

//
// ActiveExpr::ColumnIndex
//

class ActiveExpr::ColumnIndex : public ActiveExpr::Imp
{
  public:
    ColumnIndex(int index);

    virtual void buildString(ActiveClass &klass, QString &out) const;

  private:
    int dm_index;
};

ActiveExpr::ColumnIndex::ColumnIndex(int index)
  : dm_index(index)
{
}

void ActiveExpr::ColumnIndex::buildString(ActiveClass &klass, QString &out) const
{
  out += klass.fieldsVec()[dm_index]->fieldName();
}

//
//
// ActiveExpr
//
//

ActiveExpr::ActiveExpr(void)
{
}

ActiveExpr::ActiveExpr(const ActiveExpr &rhs)
  : dm_imp(rhs.dm_imp)
{
}

ActiveExpr::ActiveExpr(const std::shared_ptr<Imp> &imp)
  : dm_imp(imp)
{
}

bool ActiveExpr::isNull(void) const
{
  return dm_imp.get() == 0;
}

void ActiveExpr::buildString(ActiveClass &klass, QString &out) const
{
  assert(dm_imp.get());

  dm_imp->buildString(klass, out);
}

ActiveExpr ActiveExpr::fromColumn(int index)
{
  return ActiveExpr(std::shared_ptr<Imp>(new ColumnIndex(index)));
}

