
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTEXT_H__
#define __INCLUDED_WEXUS_CONTEXT_H__

#include <QTextStream>

#include <hydra/TR1.h>

#include <wexus/HTTPRequest.h>
#include <wexus/HTTPReply.h>
#include <wexus/FormParams.h>
#include <wexus/Cookies.h>
#include <wexus/Session.h>

namespace wexus
{
  /**
   * Same as Context::instance().output().
   *
   * @author Aleksander Demko
   */ 
  QTextStream & output(void);

  /**
   * Same as Context::instance().htmlOutput().
   *
   * @author Aleksander Demko
   */ 
  QTextStream & htmlOutput(void);

  class Context;

  class Application;  // forward
}

/**
 * The context object sets up and maintains (during its lifetime)
 * various thread-local storage variables necesary for the other
 * global functions defined in this file.
 *
 * @author Aleksander Demko
 */ 
class wexus::Context
{
  public:
    /// constructor
    Context(wexus::Application *application, const QString &actionname, wexus::HTTPRequest &req, wexus::HTTPReply &reply);
    /// destructor
    ~Context();

    /// returns the static instance
    static Context *instance(void);

    // might remove these in the future, maybe, not sure

    wexus::Application * application(void) const { return dm_application; }
    const QString actionName(void) const { return dm_actionname; }
    wexus::HTTPRequest & request(void) const { return dm_req; }
    wexus::HTTPReply & reply(void) const { return dm_reply; }

    // should these be static with implied instance()??

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

  private:
    wexus::Application *dm_application;
    QString dm_actionname;
    wexus::HTTPRequest & dm_req;
    wexus::HTTPReply & dm_reply;

    std::shared_ptr<QIODevice> dm_htmldevice;
    std::shared_ptr<QTextStream> dm_htmloutput;

  public:
    // public things
    // placed her to maintain a proper initialization order

    /**
     * The form parameters.
     * Object directly accessible so operator[] works.
     *
     * @author Aleksander Demko
     */ 
    FormParams params;

    /**
     * The cookies.
     *
     * @author Aleksander Demko
     */ 
    Cookies cookies;

    /**
     * The session, if any
     *
     * @author Aleksander Demko
     */ 
    Session session;
};

#endif

