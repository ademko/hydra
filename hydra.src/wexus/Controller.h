
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTROLLER_H__
#define __INCLUDED_WEXUS_CONTROLLER_H__

#include <assert.h>

#include <map>

#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>

namespace wexus
{
  class Controller;
}

/**
 * Base class for all user defined controllers.
 * Controller is the C in MVC. They are instantied
 * per request, with the appripriate sub message being called.
 *
 * @author Aleksander Demko
 */ 
class wexus::Controller
{
  public:
    class ActionNotFoundException : public wexus::HTTPHandler::Exception
    {
      public:
        ActionNotFoundException(const QString &usermsg)
          : wexus::HTTPHandler::Exception(usermsg) { }
    };

  public:
    virtual ~Controller();

    virtual void handleControllerRequest(QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    /// constructor
    Controller(void);

    // see if this can be made nicery using
    // some C++ or macro magic?
    //template <void (C::*METH)(void), class C>
    template <class C, void (C::*METH)(void)>
      void registerAction(const char *actioname)
      { dm_actions[actioname] = methodToFunc<C, METH>; }

  private:
    template <class C, void (C::*LMETH)(void)>
      static void methodToFunc(Controller *c)
      {
        // bounce the function call to a method call
        C *here = dynamic_cast<C*>(c);
        assert(here);

        ((*here).*LMETH)();
      }

  private:
    typedef void (*actionfunc_t)(Controller *c);
    typedef std::map<QString, actionfunc_t> actionmap_t;

    actionmap_t dm_actions;
};

#endif

