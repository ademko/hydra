
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveExpr.h>

#include <wexus/ActiveClass.h>
#include <wexus/AssertException.h>

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

    int columnIndex(void) const { return dm_index; }

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
// ActiveExpr::BinOp
//

class ActiveExpr::BinOp : public ActiveExpr::Imp
{
  public:
    BinOp(const QString &op, const std::shared_ptr<ActiveExpr::Imp> &lhs,
            const std::shared_ptr<ActiveExpr::Imp> &rhs);

    virtual void buildString(ActiveClass &klass, QString &out) const;
    virtual void buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const;

  private:
    QString dm_op;
    std::shared_ptr<ActiveExpr::Imp> dm_left, dm_right;
};

ActiveExpr::BinOp::BinOp(const QString &op,
    const std::shared_ptr<ActiveExpr::Imp> &lhs,
    const std::shared_ptr<ActiveExpr::Imp> &rhs)
  : dm_op(op), dm_left(lhs), dm_right(rhs)
{
}

void ActiveExpr::BinOp::buildString(ActiveClass &klass, QString &out) const
{
  dm_left->buildString(klass, out);
  out += " " + dm_op + " ";
  dm_right->buildString(klass, out);
}

void ActiveExpr::BinOp::buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const
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

ActiveExpr::ActiveExpr(const QVariant &v)
  : dm_imp(new Var(v))
{
}

ActiveExpr::ActiveExpr(int i) : dm_imp(new Var(i)) { }
ActiveExpr::ActiveExpr(double d) : dm_imp(new Var(d)) { }
ActiveExpr::ActiveExpr(const char *c) : dm_imp(new Var(c)) { }
ActiveExpr::ActiveExpr(const QString &s) : dm_imp(new Var(s)) { }

bool ActiveExpr::isNull(void) const
{
  return dm_imp.get() == 0;
}

int ActiveExpr::columnIndex(void) const
{
  assert(dm_imp);
  ColumnIndex *c = dynamic_cast<ColumnIndex*>(dm_imp.get());

  if (!c)
    throw AssertException("ActiveExpr::columnIndex() called on a non-columnIndex expression");

  return c->columnIndex();
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

ActiveExpr ActiveExpr::operator == (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("=",   dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator != (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("<>",  dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator <  (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("<",   dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator <= (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("<=",  dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator >  (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp(">",   dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator >= (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp(">=",  dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator && (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("AND", dm_imp, rhs.dm_imp))); }
ActiveExpr ActiveExpr::operator || (const ActiveExpr &rhs) { return ActiveExpr(std::shared_ptr<Imp>(new BinOp("OR",  dm_imp, rhs.dm_imp))); }

