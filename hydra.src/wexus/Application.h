
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_APPLICATION_H__
#define __INCLUDED_WEXUS_APPLICATION_H__

#include <hydra/Registry.h>

#include <wexus/Controller.h>
#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>

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

    class ControllerNotFoundException : public wexus::HTTPHandler::Exception
    {
      public:
        ControllerNotFoundException(const QString &usermsg)
          : wexus::HTTPHandler::Exception(usermsg) { }
    };

  public:
    /// destructor
    virtual ~Application();

    /**
     * A enhanced handleApplicationRequest() call from the wexus::Site to wexus::Application.
     * filteredRequest contains just the action call. It always starts with atleast a /
     *
     * This method is not typically overriden.
     *
     * @author Aleksander Demko
     */ 
    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    /// inherited constructor
    Application(void);

    /**
     * Registers a controller.
     *
     * @param shortname the name of the controller, as seen in url. This should not have
     * any slashes or other punctutation. Example "home" "users" etc
     * @author Aleksander Demko
     */ 
    template <class T>
      void registerController(const char *shortname)
      { dm_controllers.appendFunc(&hydra::loadfunc_impl<Controller,T>, shortname); }

  private:
    // TODO in the future, replace this registry with something that
    // uses a map rather than vector?
    hydra::Registry<Controller> dm_controllers;
};

#endif

