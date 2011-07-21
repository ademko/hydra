
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_VALIDATIONEXPR_H__
#define __INCLUDED_WEXUS_VALIDATIONEXPR_H__

#include <hydra/TR1.h>

#include <QVariant>
#include <QStringList>

namespace wexus
{
  class ValidationExpr;
}

class wexus::ValidationExpr
{
  public:
    /// null expression, that doesn't require anything
    ValidationExpr(void);
    /// shallow copy ctor
    ValidationExpr(const ValidationExpr &rhs);

    bool isNull(void) const;

    /// tests to se if v meets the requiresments
    /// of the expressinall the tests
    /// this ValidationExpr cannt be null
    bool test(const QVariant &v, QStringList *outerrors = 0) const;

    // "serialization"

    /// package this expression into a variant
    QVariant toVariant(void);

    /// unpackage an expression from the variant
    ValidationExpr fromVariant(const QVariant &v);

    // explicit "ctors"

    // the value cannot be invalid
    static ValidationExpr required(void);

    // note that all the follow checkers
    // only test valid variables.
    // that is, they all return true on invalid values
    //  (so as to allow the processing of optional fields, etc)

    // string checkers
    static ValidationExpr minLength(int l);
    static ValidationExpr maxLength(int l);
    static ValidationExpr zeroLength(void);    // same as maxLength(0)

    // int checkers
    static ValidationExpr minValue(double d);
    static ValidationExpr maxValue(double d);

    // fancy string checkers
    static ValidationExpr isEmail(void);

    // operators

    ValidationExpr operator ! (void); // not operator
    ValidationExpr operator && (const ValidationExpr &rhs);

  private:
    class Imp;
    class Required;
    class Length;
    class UniOp;
    class BinOp;

    ValidationExpr(const std::shared_ptr<Imp> &imp);

  private:
    std::shared_ptr<Imp> dm_imp;
};

#endif

