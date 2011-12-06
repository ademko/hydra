
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_HTTPSERVER_H__
#define __INCLUDED_WEXUS_HTTPSERVER_H__

#include <QString>

#include <wexus/TR1.h>
#include <wexus/Exception.h>

namespace wexus
{
  class HTTPParams;
  class HTTPServer;

  class HTTPHandler;  //fwd
}

/**
 * Options that can be passed to the HTTPServer upon bootup.
 *
 * @author Aleksander Demko
 */
class wexus::HTTPParams
{
  public:
    /// constructor
    HTTPParams(void);

    /// port to listen to, 8080 by default.
    int port(void) const { return dm_port; }

    int numThreads(void) const { return dm_numthreads; }

    void setPort(int p);

    void setNumThreads(int t);

    /**
     * Sets the handeler that will receive
     * all the http events. By default, it
     * is null, for no handler.
     *
     * @author Aleksander Demko
     */ 
    void setHandler(HTTPHandler *handler);

    HTTPHandler * handler(void) const { return dm_handler; }

  private:
    int dm_port;
    int dm_numthreads;
    HTTPHandler *dm_handler;
};

/**
 * Base class/interface for all HTTP servers.
 *
 * @author Aleksander Demko
 */ 
class wexus::HTTPServer
{
  public:
    class Exception : public wexus::Exception
    {
      public:
        /// no log message constructor... is this version needed?
        //Exception(void);
        /// log message constructor
        Exception(const QString &logmsg);

        virtual ~Exception() throw ();

        const QString & logMessage(void) const { return dm_logmsg; }

      protected:
        QString dm_logmsg;
    };
    class PortInUseException : public Exception
    {
      public:
        PortInUseException(int port);
    };

  public:
    /// destructor - will stop the server is needed
    virtual ~HTTPServer();

    /**
     * Create a new HTTPServer implementation, based on some decendant.
     * This is a factory method. In the future, this could take a string
     * or perhaps be moved to a whole factory class system.
     *
     * Exceptions might be thrown.
     *
     * @author Aleksander Demko
     */ 
    static std::shared_ptr<wexus::HTTPServer> factoryNew(const wexus::HTTPParams &params);

    /// is the web server currently running?
    virtual bool isRunning(void) const = 0;
    /// starts the web server
    virtual void start(void) = 0;
    /// notifies the web server to stop. you should still do a wait()
    /// this function may or may not block
    virtual void quit(void) = 0;
    /// waits until the server finishes
    virtual void wait(void) = 0;

  protected:
    HTTPServer(void);
};

#endif

