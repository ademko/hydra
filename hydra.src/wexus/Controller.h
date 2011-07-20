
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTROLLER_H__
#define __INCLUDED_WEXUS_CONTROLLER_H__

#include <assert.h>

#include <map>

#include <QVariant>

#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>
#include <wexus/Context.h>

namespace wexus
{
  class ControllerContext;
  class Controller;
}

// helper operator for QTextStream ouput of variants
inline QTextStream & operator <<(QTextStream &o, const QVariant &v) { return o << v.toString(); }

/**
 * This contains useful input/output functions that are part of
 * Controller.
 *
 * It basically reflects a lot of the methods of the Context TLS global
 * into the Controller itself.
 *
 * @author Aleksander Demko
 */ 
class wexus::ControllerContext
{
  private:
    wexus::Context &dm_context;   // at the top as this should be initialized before cookies, etc

  public:
    /**
     * Constructor.
     *
     * @author Aleksander Demko
     */ 
    ControllerContext(void);

    /**
     * Returns the raw output stream.
     *
     * @author Aleksander Demko
     */ 
    QTextStream & output(void) { return dm_context.output(); }

    /**
     * Returns a stream that HTML escapes all output
     * (except HTMLString).
     *
     * @author Aleksander Demko
     */ 
    QTextStream & htmlOutput(void) { return dm_context.htmlOutput(); }

    /**
     * The form parameters.
     * Object directly accessible so operator[] works.
     *
     * @author Aleksander Demko
     */ 
    FormParams &params;

    /**
     * The cookies.
     *
     * @author Aleksander Demko
     */ 
    Cookies &cookies;

    /**
     * The session, if any.
     *
     * @author Aleksander Demko
     */ 
    QVariantMap &session;

    /**
     * The incoming flash sent by the previous calls.
     *
     * @author Aleksander Demko
     */ 
    const QVariantMap &flash;


    /**
     * The flash that will be preseved for the next calls.
     * In the next call, this will be the contents of inFlash.
     *
     * @author Aleksander Demko
     */ 
    QVariantMap &setFlash;
};

/**
 * Base class for all user defined controllers.
 * Controller is the C in MVC. They are instantied
 * per request, with the appripriate sub message being called.
 *
 * @author Aleksander Demko
 */ 
class wexus::Controller : public wexus::ControllerContext
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

    virtual void handleControllerRequest(const QString &actionname);

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

