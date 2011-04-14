
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
     * The actual headers and output will be written when
     * commitHeader() is called.
     * You often don't need to call commitHeader() yourself,
     * as output() will do it for you, as needed.
     *
     * @author Aleksander Demko
     */ 
    HTTPReply(QTextStream &outstream, int status = 200);
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
     * Sets the current status code that will be returned.
     *
     * @author Aleksander Demko
     */ 
    void setStatus(int status);

    /**
     * Gets the current return status code.
     *
     * @author Aleksander Demko
     */ 
    int status(void) const { return dm_status; }

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
     * Commits and flushes any data to the output stream.
     * The constructor will also automatically call this.
     * You can this repeadedly, only the first call does something.
     * Calling this with Status set to 0 is an error.
     *
     * @author Aleksander Demko
     */ 
    void commitHeader(void);

    /**
     * Returns a stream that you can use to write the 
     * reply body. This called commitHeader() before hand, if it hasn't been called already.
     *
     * You may want to include wx/streamop.h
     *
     * @author Aleksander Demko
     */ 
    QTextStream & output(void);

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
    virtual bool handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply) = 0;
};

/**
 * A handler that does nothing but display a deliberatly cryptic error message.
 *
 * @author Aleksander Demko
 */ 
class wexus::ErrorHTTPHandler
{
  public:
    bool handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);
};

#endif

