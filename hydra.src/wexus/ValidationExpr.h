
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_VALIDATIONEXPR_H__
#define __INCLUDED_WEXUS_VALIDATIONEXPR_H__

#include <wexus/TR1.h>

#include <QVariant>
#include <QStringList>

namespace wexus
{
  class ValidationExpr;

  class validate;
}

/**
 * A validation expression.
 *
 * Although there were plans to be able to create
 * nesting operators, compelte with ! || and other operators,
 * this was deemed to confusing with respect to how
 * to make meaningfull error messages. A future
 * project, perhaps.
 *
 * @author Aleksander Demko
 */ 
class wexus::ValidationExpr
{
  public:
    /**
     * Null expression, that doesn't require anything
     * This expression cannot be used or tested.
     *
     * @author Aleksander Demko
     */ 
    ValidationExpr(void);
    /// shallow copy ctor
    ValidationExpr(const ValidationExpr &rhs);

    /// is this the default, null expression
    bool isNull(void) const;

    /// tests to se if v meets the requiresments
    /// of the expressinall the tests
    /// this ValidationExpr cannt be null
    bool test(const QVariant &v, QStringList *outerrors = 0) const;

    // "serialization"

    /// package this expression into a variant
    QVariant toVariant(void) const;

    /// unpackage an expression from the variant
    static ValidationExpr fromVariant(const QVariant &v);

    // explicit "ctors"


    /// always returns true
    static ValidationExpr optional(void);

    /// the value cannot be invalid
    static ValidationExpr required(void);

    // note that all the follow checkers
    // only test valid variables.
    // that is, they all return true on invalid values
    //  (so as to allow the processing of optional fields, etc)

    // string checkers
    static ValidationExpr minLength(int l);
    static ValidationExpr maxLength(int l);
    static ValidationExpr notEmptyLength(void);    // similar to minLength(1), but with a different error message

    // int checkers
    static ValidationExpr minValue(double d);
    static ValidationExpr maxValue(double d);

    // fancy string checkers
    static ValidationExpr isEmail(void);

    // operators

    //ValidationExpr operator ! (void); // not operator

    /// (short circuit) and operator
    ValidationExpr operator && (const ValidationExpr &rhs);

  public:
    class ImpObject;
  private:
    class Imp;
    class Optional;
    class Required;
    class Length;
    //class UniOp;
    class BinOp;

    ValidationExpr(const std::shared_ptr<Imp> &imp);

  private:
    std::shared_ptr<Imp> dm_imp;
};

/**
 * This is a convience class. Rather than make a new validate namespace
 * and make a bunch of inline functions to bounce to the static methods
 * of ValidationExpr, we'll make a similarly named class that just
 * inherits all the static methods.
 * 
 * @author Aleksander Demko
 */ 
class wexus::validate : public ValidationExpr
{
  public:

  private:
    /// not implemented. this class is not instantiatable.
    validate(void);
};

#endif

