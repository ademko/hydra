
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_HTTP_H__
#define __INCLUDED_WEXUS_HTTP_H__

#include <QString>
#include <QTextStream>

namespace wexus
{
  QString escapeForXML(const QString &s);

  class HTTPRequest;
  class HTTPReply;

  class HTTPHandler;
  class ErrorHTTPHandler;
}

/**
 * All the information in a HTTPRequest. The handler
 * can use this information to compose their HTTPReply.
 *
 * @author Aleksander Demko
 */ 
class wexus::HTTPRequest
{
  public:
    HTTPRequest(void);

    const QString & request(void) const { return dm_request; }
    const QString & query(void) const { return dm_query; }
    const QString & referer(void) const { return dm_referer; }
    const QString & userAgent(void) const { return dm_useragent; }

  protected:
    QString dm_request, dm_query, dm_referer, dm_useragent;
};

/**
 * A HTTP reply that is sent back to the client
 * as a result of an HTTP request.
 *
 * The status may be set via the constructor, and or changed
 * via setStatus. In eithercase, a reply with a 0 status
 * is invalid and cannot be commited (used).
 *
 * @author Aleksander Demko
 */ 
class wexus::HTTPReply
{
  public:
    /**
     * Constructs an http reply, using the given stream
     * and with the given status code.
     *
     * If your handler wants to respond to a request, must call output() (and
     * produce some output), setStatus() or both.
     *
     * @author Aleksander Demko
     */ 
    HTTPReply(QTextStream &outstream);
    /**
     * This calls the commitHeader method if status is >0 AND commitHeader() has not
     * yet been called.
     *
     * @author Aleksander Demko
     */ 
    ~HTTPReply();

    /**
     * Converts a status code to a description, null if this is an unkown status
     * code. Uses const char * to minimuze copies.
     *
     * @author Aleksander Demko
     */
    static const char * statusToString(int status);

    /**
     * Sets the current status code that will be returned. The default status
     * is 0, unless output() is used, in which case it is 200.
     *
     * @author Aleksander Demko
     */ 
    void setStatus(int status);

    /**
     * Gets the current return status code. If its 0, it means
     * unset (reply not set).
     *
     * @author Aleksander Demko
     */ 
    int status(void) const { return dm_status; }

    /**
     * True is a reply has been status (status is non-zero)
     *
     * @author Aleksander Demko
     */ 
    bool hasReply(void) const { return dm_status != 0; }

    /**
     * Sets the content type.
     * The default content type is text/html.
     *
     * @author Aleksander Demko
     */ 
    void setContentType(const QString &type);

    /**
     * Returns the currently set content type.
     * The default content type is text/html.
     *
     * @author Aleksander Demko
     */ 
    const QString & contentType(void) const { return dm_contenttype; }

    /**
     * Returns a stream that you can use to write the 
     * reply body. This called commitHeader() before hand, if it hasn't been called already.
     *
     * You may want to include wx/streamop.h
     *
     * @author Aleksander Demko
     */ 
    QTextStream & output(void);

  private:
    /**
     * Commits and flushes any data to the output stream.
     * The constructor will also automatically call this.
     * You can this repeadedly, only the first call does something.
     * Calling this with Status set to 0 is an error.
     *
     * @author Aleksander Demko
     */ 
    void commitHeader(void);

  protected:
    QTextStream &dm_outs;
    int dm_status;
    bool dm_calledcommit;
    QString dm_contenttype;

};

/**
 * A wexus::HTTPHandler handles events from a wexus::HTTPServer.
 *
 * @author Aleksander Demko
 */ 
class wexus::HTTPHandler
{
  public:
    /**
     * An Exception that handlers can throw. The
     * serverw ill catch them and display them to the user.
     *
     * @author Aleksander Demko
     */ 
    class Exception : public std::exception
    {
      public:
        /// no usermessage constructor
        Exception(void);
        /// usermessage constructor
        Exception(const QString &usermsg);

        virtual ~Exception() throw ();

        virtual const char* what() const throw ()
          { return "wexus::HTTPHandler::Exception"; }

        /**
         * The user-visable error message string.
         *
         * @author Aleksander Demko
         */ 
        const QString & userMessage(void) const { return dm_usermsg; }

      protected:
      protected:
        QString dm_usermsg;
    };

  public:
    /// destructor
    virtual ~HTTPHandler();

    /**
     * This is called when a request is to be processed.
     *
     * @param req the request
     * @return true if this consumer has processed this even and no further consumers should
     * receive this event. false if this consumer does not want to process this event.
     * @author Aleksander Demko
     */ 
    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply) = 0;
};

/**
 * A handler that does nothing but display a deliberatly cryptic error message.
 *
 * @author Aleksander Demko
 */ 
class wexus::ErrorHTTPHandler : public wexus::HTTPHandler
{
  public:
    /// no userMessage constructor
    ErrorHTTPHandler(void);
    /// with userMessage constructor
    ErrorHTTPHandler(const QString &usermsg);

  public:
    void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    QString dm_usermsg;
};

#endif

