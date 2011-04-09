
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_REGiSTRY_H__
#define __INCLUDED_HYDRA_REGiSTRY_H__

#include <hydra/TR1.h>

#include <QDebug>

namespace hydra
{
  template <class BASE, class SUB> BASE* loadfunc_impl(void) { return new SUB(); }
  template <class BASE> class Registry;
  template <class SUB> class Register;
}

/**
 * This allows you to setup a load-time registering plugin system
 * that registers objects for any purpose.
 *
 * You typically make a BASE class for your plugins. In that base class, you:
 *  - typedef Registry<ThatBase> registry_type
 *  - static registry_type registry
 *  - make sure your dtor is virtual
 *
 * The Regisiter objects will then use BASE::registry to add types.
 * You can then inspect and instatiate registered objects via
 * the members of this class.
 *
 * @author Aleksander Demko
 */ 
template <class BASE> class hydra::Registry
{
  public:
    typedef BASE base_type;
    typedef BASE* (*loadfunc_t)(void);
    template <class SUB> BASE* loadfunc_impl(void) { return new SUB(); }

    class payload_t {
      public:
        loadfunc_t loader;
      public:
        payload_t(loadfunc_t _loader) : loader(_loader) { }
    };

  public:
    //Registry(void) { qDebug() << __FUNCTION__; }
    //Registry<BASE> & instance(Registry<BASE> & *ptr);

    void appendFunc(loadfunc_t func) { instance(); /*qDebug() << __FUNCTION__;*/ dm_funcs->push_back(func); }

    //template <class SUB>
      //void appendType(void) { } //appendFunc(&loadfunc_impl<SUB>); }

    size_t size(void) { instance(); return dm_funcs->size(); }

    std::shared_ptr<BASE> create(size_t index) { instance(); return std::shared_ptr<BASE>((*dm_funcs)[index].loader()); }

  private:
    void instance(void);

  private:
    // this is a pointer rather than direct for cleaner start up initialization
    // resolution (ie. we can control it somewhat)
    std::vector<payload_t> *dm_funcs;
};

/*template <class BASE>
hydra::Registry<BASE> & hydra::Registry<BASE>::instance(Registry<BASE> & *ptr)
{
  if (ptr == 0)
    ptr = new Registry<BASE>();
  return *ptr;
}*/

template <class BASE>
void hydra::Registry<BASE>::instance(void)
{
  //qDebug() << __FUNCTION__ << (dm_funcs !=0);
  if (!dm_funcs)
    dm_funcs = new std::vector<payload_t>;
}

/**
 * This is the helper class that does the actual registration.
 *
 * Embeed a static-global instance of one of these per-object
 * to be registered. The constructor will then registery the type
 * SUB with the registry.
 *
 * The type SUB should have a registry_type typedef and a register
 * variable. You should implement this via a common BASE class
 * (see Registry)
 *
 * @author Aleksander Demko
 */ 
template <class SUB> class hydra::Register
{
  public:
    /**
     * Registering constructor.
     *
     * @author Aleksander Demko
     */ 
    Register(void)
      {
        //if (!SUB::registry)
          //SUB::registry = new typename SUB::registry_type;
        SUB::registry.appendFunc(&hydra::loadfunc_impl<typename SUB::registry_type::base_type, SUB>);
      }
};

#endif

