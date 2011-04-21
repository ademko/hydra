
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_APPLICATION_H__
#define __INCLUDED_WEXUS_APPLICATION_H__

#include <hydra/Registry.h>

#include <wexus/Controller.h>

namespace wexus
{
  class Application;
}

/**
 * Controllers are groupped into applications. Applications
 * are the installed, at a certain mount point (prefix)
 * into a Site.
 *
 * Each user application decends from this class,
 * registers the application subclass via.
 * The application then registers controllers in 
 * its constructor via registerController.
 *
 * @author Aleksander Demko
 */ 
class wexus::Application
{
  public:
    typedef hydra::Registry<Application> registry_type;
    static registry_type registry;

  public:
    /// destructor
    virtual ~Application();

  protected:
    /// inherited constructor
    Application(void);

    /// controller regisistration function
    template <class T>
      void registerController(const char *shortname)
      { dm_registry.appendFunc(&hydra::loadfunc_impl<Controller,T>, shortname); }

  private:
    // TODO in the future, replace this registry with something that
    // uses a map rather than vector?
    hydra::Registry<Controller> dm_registry;
};

#endif

