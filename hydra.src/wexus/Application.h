
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_APPLICATION_H__
#define __INCLUDED_WEXUS_APPLICATION_H__

#include <hydra/Registry.h>

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
    //void registerController<>("shortname");
};

#endif

