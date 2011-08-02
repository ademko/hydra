
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ValidationExpr.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

//
// ImpObject
//

class ValidationExpr::ImpObject
{
  public:
    std::shared_ptr<Imp> imp;
};

Q_DECLARE_METATYPE(ValidationExpr::ImpObject)

//
// Imp
//

class ValidationExpr::Imp
{
  public:
    /// virtual dtor
    virtual ~Imp();

    virtual bool test(const QVariant &v, QStringList *outerrors) const = 0;
};

ValidationExpr::Imp::~Imp()
{
}

class ValidationExpr::Optional : public ValidationExpr::Imp
{
  public:
    virtual bool test(const QVariant &v, QStringList *outerrors) const;
};

bool ValidationExpr::Optional::test(const QVariant &v, QStringList *outerrors) const
{
  return true;
}

class ValidationExpr::Required : public ValidationExpr::Imp
{
  public:
    virtual bool test(const QVariant &v, QStringList *outerrors) const;
};

bool ValidationExpr::Required::test(const QVariant &v, QStringList *outerrors) const
{
  bool r = v.isValid();

  if (!r && outerrors)
    outerrors->push_back("is required");

  return r;
}

class ValidationExpr::Length : public ValidationExpr::Imp
{
  public:
    enum {
      GTE,
      LTE,
    };
    Length(int op, int val, const char * errormsg = 0);

    virtual bool test(const QVariant &v, QStringList *outerrors) const;

  private:
    int dm_op, dm_val;
    const char * dm_errormesg;
};

ValidationExpr::Length::Length(int op, int val, const char * errormsg)
  : dm_op(op), dm_val(val), dm_errormesg(errormsg)
{
}

bool ValidationExpr::Length::test(const QVariant &v, QStringList *outerrors) const
{
  bool r;

  if (!v.isValid())
    return true;

//qDebug() << "dm_op" << dm_op << "dm_val" << dm_val << "toInt" << v.toInt();
  if (dm_op == GTE) {
    r = v.toString().size() >= dm_val;
    if (!r && outerrors) {
      if (dm_errormesg)
        outerrors->push_back(dm_errormesg);
      else
        outerrors->push_back("too short (must be " + QString::number(dm_val) + " characters)");
    }
  } else { //LTE
    r = v.toString().size() <= dm_val;
    if (!r && outerrors) {
      if (dm_errormesg)
        outerrors->push_back(dm_errormesg);
      else
        outerrors->push_back("too long (must be at most " + QString::number(dm_val) + " characters)");
    }
  }

  return r;
}

/*class ValidationExpr::UniOp : public ValidationExpr::Imp
{
  public:
    enum {
      NOT,
    };

  public:
    UniOp(int op, const std::shared_ptr<Imp> &lhs);

    virtual bool test(const QVariant &v, QStringList *outerrors) const;
  private:
    int dm_op;
    std::shared_ptr<Imp> dm_left;
};

ValidationExpr::UniOp::UniOp(int op, const std::shared_ptr<Imp> &lhs)
  : dm_op(op), dm_left(lhs)
{
}

bool ValidationExpr::UniOp::test(const QVariant &v, QStringList *outerrors) const
{
  // dm_op == NOT

  return !dm_left->test(v, outerrors);
}*/

class ValidationExpr::BinOp : public ValidationExpr::Imp
{
  public:
    enum {
      AND,
    };

  public:
    BinOp(int op, const std::shared_ptr<Imp> &lhs,
        const std::shared_ptr<Imp> &rhs);

    virtual bool test(const QVariant &v, QStringList *outerrors) const;

  private:
    int dm_op;
    std::shared_ptr<Imp> dm_left, dm_right;
};

ValidationExpr::BinOp::BinOp(int op, const std::shared_ptr<Imp> &lhs,
        const std::shared_ptr<Imp> &rhs)
  : dm_op(op), dm_left(lhs), dm_right(rhs)
{
}

bool ValidationExpr::BinOp::test(const QVariant &v, QStringList *outerrors) const
{
  return
    dm_left->test(v, outerrors) &&
    dm_right->test(v, outerrors);
}

//
//
// ValidationExpr
//
//

ValidationExpr::ValidationExpr(void)
{
}

ValidationExpr::ValidationExpr(const ValidationExpr &rhs)
  : dm_imp(rhs.dm_imp)
{
}

ValidationExpr::ValidationExpr(const std::shared_ptr<Imp> &imp)
  : dm_imp(imp)
{
}

bool ValidationExpr::isNull(void) const
{
  return dm_imp.get() == 0;
}

bool ValidationExpr::test(const QVariant &v, QStringList *outerrors) const
{
  assert(dm_imp.get());

  return dm_imp->test(v, outerrors);
}

QVariant ValidationExpr::toVariant(void) const
{
  ImpObject o;

  o.imp = dm_imp;

  return QVariant::fromValue(o);
}

ValidationExpr ValidationExpr::fromVariant(const QVariant &v)
{
  // check the type of v here?
  ImpObject o = v.value<ImpObject>();

  return ValidationExpr(o.imp);
}

ValidationExpr ValidationExpr::optional(void)
{
  return ValidationExpr(std::shared_ptr<Imp>(new Optional));
}

ValidationExpr ValidationExpr::required(void)
{
  return ValidationExpr(std::shared_ptr<Imp>(new Required));
}

ValidationExpr ValidationExpr::minLength(int l)
{
  return ValidationExpr(std::shared_ptr<Imp>(new Length(Length::GTE, l)));
}

ValidationExpr ValidationExpr::maxLength(int l)
{
  return ValidationExpr(std::shared_ptr<Imp>(new Length(Length::LTE, l)));
}

ValidationExpr ValidationExpr::notEmptyLength(void)
{
  return ValidationExpr(std::shared_ptr<Imp>(new Length(Length::GTE, 1, "cannot be empty")));
}

// removed the NOT operator
/*ValidationExpr ValidationExpr::operator ! (void)
{
  return ValidationExpr(std::shared_ptr<Imp>(new UniOp(UniOp::NOT, dm_imp)));
}*/

ValidationExpr ValidationExpr::operator && (const ValidationExpr &rhs) { return ValidationExpr(std::shared_ptr<Imp>(new BinOp(BinOp::AND, dm_imp, rhs.dm_imp))); }
