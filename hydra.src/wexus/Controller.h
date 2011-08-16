
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
    QVariantMap &params;

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

    /**
     * Errors.
     *
     * @author Aleksander Demko
     */
    Context::Errors &errors;
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
    virtual ~Controller();

  protected:
    /// constructor
    Controller(void);
};

#endif

