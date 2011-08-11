
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INLCLUDED_WEXUS_ACTIVEEXPR_H__
#define __INLCLUDED_WEXUS_ACTIVEEXPR_H__

#include <wexus/TR1.h>

#include <QVariant>
#include <QSqlQuery>

namespace wexus
{
  class ActiveExpr;

  class ActiveClass;//fwd
  class ActiveRecord;//fwd
}

class wexus::ActiveExpr
{
  public:
    /// creates a null expression (empty)
    ActiveExpr(void);
    /// shallow copy ctor
    ActiveExpr(const ActiveExpr &rhs);
    /// Variable initer
    ActiveExpr(const QVariant &v);

    // we have these rather than having operators that take QVariant... good idea?
    ActiveExpr(int i);
    ActiveExpr(double d);
    ActiveExpr(const char *c);
    ActiveExpr(const QString &s);

    bool isNull(void) const;

    /**
     * Returns the column index if this expr was built with fromColumn.
     * throws if its not such an expression
     *
     * @author Aleksander Demko
     */ 
    int columnIndex(void) const;

    void buildString(ActiveClass &klass, QString &out) const;
    void buildBinds(ActiveClass &klass, ActiveRecord &recinst, QSqlQuery &out) const;

    // explicit "ctors"

    static ActiveExpr fromColumn(int index);

    // operators

    ActiveExpr operator == (const ActiveExpr &rhs);
    ActiveExpr operator != (const ActiveExpr &rhs);
    ActiveExpr operator <  (const ActiveExpr &rhs);
    ActiveExpr operator <= (const ActiveExpr &rhs);
    ActiveExpr operator >  (const ActiveExpr &rhs);
    ActiveExpr operator >= (const ActiveExpr &rhs);
    ActiveExpr operator && (const ActiveExpr &rhs);
    ActiveExpr operator || (const ActiveExpr &rhs);

  private:
    class Imp;  // internal class
    class ColumnIndex;
    class BinOp;
    class Var;

    std::shared_ptr<Imp> dm_imp;

  private:
    /// internal ctor for the explicit "ctors"
    ActiveExpr(const std::shared_ptr<Imp> &imp);

};

#endif

