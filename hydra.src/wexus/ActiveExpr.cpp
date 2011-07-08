
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
    virtual void buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const;
};

ActiveExpr::Imp::~Imp()
{
}

void ActiveExpr::Imp::buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const
{
  // does nothing by default
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
  out += klass.tableName() + "." + klass.fieldsVec()[dm_index]->fieldName();
}

//
// ActiveExpr::EqualOp
//

class ActiveExpr::EqualOp : public ActiveExpr::Imp
{
  public:
    EqualOp(const std::shared_ptr<ActiveExpr::Imp> &lhs,
            const std::shared_ptr<ActiveExpr::Imp> &rhs);

    virtual void buildString(ActiveClass &klass, QString &out) const;
    virtual void buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const;

  private:
    std::shared_ptr<ActiveExpr::Imp> dm_left, dm_right;
};

ActiveExpr::EqualOp::EqualOp(const std::shared_ptr<ActiveExpr::Imp> &lhs,
            const std::shared_ptr<ActiveExpr::Imp> &rhs)
  : dm_left(lhs), dm_right(rhs)
{
}

void ActiveExpr::EqualOp::buildString(ActiveClass &klass, QString &out) const
{
  dm_left->buildString(klass, out);
  out += " = ";
  dm_right->buildString(klass, out);
}

void ActiveExpr::EqualOp::buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const
{
  dm_left->buildBinds(klass, recinst, out);
  dm_right->buildBinds(klass, recinst, out);
}

//
// ActiveExpr::var
//

class ActiveExpr::Var : public ActiveExpr::Imp
{
  public:
    Var(const QVariant &var);

    virtual void buildString(ActiveClass &klass, QString &out) const;
    virtual void buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const;

  private:
    QVariant dm_var;
};

ActiveExpr::Var::Var(const QVariant &var)
  : dm_var(var)
{
}

void ActiveExpr::Var::buildString(ActiveClass &klass, QString &out) const
{
  out += "?";
}

void ActiveExpr::Var::buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const
{
  out.addBindValue(dm_var);
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

void ActiveExpr::buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const
{
  assert(dm_imp);

  dm_imp->buildBinds(klass, recinst, out);
}

ActiveExpr ActiveExpr::fromColumn(int index)
{
  return ActiveExpr(std::shared_ptr<Imp>(new ColumnIndex(index)));
}

ActiveExpr ActiveExpr::operator == (const ActiveExpr &rhs)
{
  return ActiveExpr(std::shared_ptr<Imp>(new EqualOp(dm_imp, rhs.dm_imp)));
}

ActiveExpr ActiveExpr::operator == (QVariant v)
{
  return ActiveExpr(std::shared_ptr<Imp>(new EqualOp(dm_imp,
          std::shared_ptr<Imp>(new Var(v)))));
}

