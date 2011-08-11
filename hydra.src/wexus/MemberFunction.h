
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __WEXUS_MEMBERFUNCTION_H__
#define __WEXUS_MEMBERFUNCTION_H__

#include <assert.h>

#include <wexus/TR1.h>

#include <typeinfo>

#include <QString>

namespace wexus
{
  /**
   * Given a class T, returns a unique string for
   * that class.
   *
   * @author Aleksander Demko
   */ 
  template <class T> QString typeToString(void);

  class MemberFunction;
  template <class T> class MemberFunctionType;
}

template <class T>
  QString wexus::typeToString(void)
{
  return QString(typeid(T).name());
}

/**
 * wexus::MemberFunction represents a pointer-to-member
 * as a comprable (well, only =) object.
 * This is used for wexus::Registry work
 *
 * @author Aleksander Demko
 */ 
class wexus::MemberFunction
{
  private:
    class Imp {
      public:
        virtual ~Imp() { }
          
        virtual bool equals(const Imp &rhs) const = 0;
    };
    template <class T> class ImpType : public Imp
    {
      public:
        ImpType<T>(T _mfn)
          : dm_mfn(_mfn)
        {
        }
        virtual bool equals(const Imp &rhs) const;

      protected:
        T dm_mfn;
    };

  public:
    /// constructs a null one
    MemberFunction(void);

    /// normal ctor
    template <class MFN>
      MemberFunction(MFN _mfn)
        : dm_typestring(wexus::typeToString<MFN>()), dm_imp(new ImpType<MFN>(_mfn)) { }

    virtual ~MemberFunction();

    /// cant do < because the basic pointer-to-member type doesnt do <
    inline bool operator ==(const MemberFunction &rhs) const { return equals(rhs); }

  private:
    bool equals(const MemberFunction &rhs) const;

  private:
    QString dm_typestring;
    std::shared_ptr<Imp> dm_imp;
};

template <class T>
bool wexus::MemberFunction::ImpType<T>::equals(const wexus::MemberFunction::Imp &rhs) const
{
  const MemberFunction::ImpType<T> *there = dynamic_cast<const MemberFunction::ImpType<T> * >(&rhs);

  assert(there);

  return (dm_mfn == there->dm_mfn);
}

#endif

