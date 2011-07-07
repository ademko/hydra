
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INLCLUDED_WEXUS_ACTIVEEXPR_H__
#define __INLCLUDED_WEXUS_ACTIVEEXPR_H__

#include <hydra/TR1.h>

namespace wexus
{
  class ActiveExpr;

  class ActiveClass;//fwd
}

class wexus::ActiveExpr
{
  public:
    /// creates a null expression (empty)
    ActiveExpr(void);
    /// shallow copy ctor
    ActiveExpr(const ActiveExpr &rhs);

    bool isNull(void) const;

    void buildString(ActiveClass &klass, QString &out) const;

    // explicit "ctors"

    static ActiveExpr fromNull(void);

    static ActiveExpr fromColumn(int index);

  private:
    class Imp;  // internal class
    class ColumnIndex;

    std::shared_ptr<Imp> dm_imp;

  private:
    /// internal ctor for the explicit "ctors"
    ActiveExpr(const std::shared_ptr<Imp> &imp);

};

#endif

