
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTROLLER_H__
#define __INCLUDED_WEXUS_CONTROLLER_H__

#include <assert.h>

#include <map>

#include <QTextStream>
#include <QVariant>

#include <hydra/TR1.h>

#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>
#include <wexus/FormParams.h>

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
 * This class doesn't really need to exist seperatly, but in
 * the future this concept might be merged with the TLS based
 * Context concept.
 *
 * @author Aleksander Demko
 */ 
class wexus::ControllerContext
{
  public:
    /// ctor
    ControllerContext(void);

  protected:
    /// called by handleControllerRequest
    virtual void setupContext(const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

    /**
     * Returns the raw output stream.
     *
     * @author Aleksander Demko
     */ 
    QTextStream & output(void);

    /**
     * Returns a stream that HTML escapes all output
     * (except HTMLString).
     *
     * @author Aleksander Demko
     */ 
    QTextStream & htmlOutput(void);

    /**
     * The form parameters.
     * Object directly accessible so operator[]
     * works.
     *
     * @author Aleksander Demko
     */ 
    FormParams params;

  private:
    QString dm_actionname;
    wexus::HTTPRequest *dm_req;
    wexus::HTTPReply *dm_reply;

    std::shared_ptr<QIODevice> dm_htmldevice;
    std::shared_ptr<QTextStream> dm_htmloutput;
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

    virtual void handleControllerRequest(const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

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

