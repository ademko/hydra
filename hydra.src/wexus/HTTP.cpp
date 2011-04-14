
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTTP.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

//
//
// HTTPRequest
//
//

HTTPRequest::HTTPRequest(void)
{
}

//
//
// HTTPReply
//
//

HTTPReply::HTTPReply(QTextStream &outstream, int status)
  : dm_outs(outstream), dm_status(status),
  dm_calledcommit(false), dm_contenttype("text/html")
{
}

HTTPReply::~HTTPReply()
{
  if (dm_status != 0 && !dm_calledcommit)
    commitHeader();
}

const char * HTTPReply::statusToString(int status)
{
  switch (status) {
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 204: return "No Content";
    case 301: return "Moved Permanently";
    case 302: return "Moved Temporarily";
    case 304: return "Not Modified";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    default: return 0;
  }
}

void HTTPReply::setStatus(int status)
{
  dm_status = status;
}

void HTTPReply::setContentType(const QString &type)
{
  assert(!type.isEmpty());

  dm_contenttype = type;
}

void HTTPReply::commitHeader(void)
{
  assert(dm_status>0);
  assert(dm_status>=100 && dm_status<1000);

  const char *statstring = statusToString(dm_status);

  if (dm_calledcommit)
    return;

  dm_calledcommit = true;

  // in the futre, do proper output with meaningfull description streams
  dm_outs << "HTTP/1.0 " << dm_status << " ";

  if (statstring)
    dm_outs << statstring << "\r\n";
  else
    dm_outs << "\r\n";

  dm_outs <<
    "Content-type: " << dm_contenttype << "\r\n"
    "Server: wexus/1.9.0\r\n"
    "\r\n";   // end of header, now the body
}

QTextStream & HTTPReply::output(void)
{
  assert(dm_status > 0);
  if (!dm_calledcommit)
    commitHeader();
  return dm_outs;
}

//
//
// HTTPHandler
//
//

HTTPHandler::~HTTPHandler()
{
}

//
//
// ErrorHTTPHandler
//
//

bool ErrorHTTPHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  reply.output() << "Error 432: ErrorHTTPHandler called.";

  return true;
}

