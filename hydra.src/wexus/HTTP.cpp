
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTTP.h>

#include <assert.h>

#include <QStringList>
#include <QDebug>

using namespace wexus;

QString wexus::escapeForXML(const QString &s)
{
  QString ret;

  ret.reserve(s.size() + 10);

  bool lastblank = false;

  for (QString::const_iterator ii=s.begin(); ii != s.end(); ++ii) {
    if (*ii == ' ') {
      // blank gets extra work,
      // this solves the problem you get if you replace all
      // blanks with &nbsp;, if you do that you loss 
      // word breaking
      if (lastblank) {
        lastblank = false;
        ret.append("&nbsp;");
      } else {
        lastblank = true;
        ret.append(' ');
      }
    } else {
      lastblank = false;
      //
      // HTML Special Chars
      if (*ii == '"')
        ret.append("&quot;");
      else if (*ii == '&')
        ret.append("&amp;");
      else if (*ii == '<')
        ret.append("&lt;");
      else if (*ii == '>')
        ret.append("&gt;");
      else if (*ii == '\n')
        // Handle Newline
        ret.append("&lt;br/&gt;");
      else {
        int ci = 0xffff & (*ii).unicode();
        if (ci < 160 )
          // nothing special only 7 Bit
          ret.append(*ii);
        else {
          // Not 7 Bit use the unicode system
          ret.append("&#");
          ret.append(QString::number(ci));
          ret.append(';');
        }
      }
    }//else
  }//for

  return ret;
}

//
//
// HTTPRequest
//
//

HTTPRequest::HTTPRequest(void)
{
  dm_contentlength = 0;
  dm_inputdev = 0;
}

int HTTPRequest::parseCookies(const QString &raw_cookie_str)
{
  int count = 0;
  QString::const_iterator ii, endii;
  QString name, value;
  enum {
    building_name,
    building_value,
    starting_name,
  } state = starting_name;

  ii = raw_cookie_str.begin();
  endii = raw_cookie_str.end();

//qDebug() << "WORKING ON" << raw_cookie_str;
  while (true) {
    switch (ii == endii ? '\0' : ii->toAscii()) {
      case '\0': // end of string case
      case ';':
        if (state == building_value) {
          // done a pair
//qDebug() << "DECODE COOKIE" << name << value;
          dm_clientcookies[name] = value;   // TODO decode
          name.clear();
          value.clear();
          state = starting_name;
        } else if (state == building_name) {
          // weird case, no value
          // so its a "" name and the value
//qDebug() << "DECODE COOKIE (valueonly)" << name;
          dm_clientcookies[""] = name;   // TODO decode
          name.clear();
          state = starting_name;
        } else
          return count; // fail case
        break;
      case '=':
        if (state == building_name) {
          state = building_value;
          break;
        } //else, fall through?
      default:
        if (state == starting_name) {
          if (*ii != ' ') // skip a leading space if need be
            name += *ii;
          state = building_name;
        } else if (state == building_name)
          name += *ii;
        else if (state == building_value)
          value += *ii;
        break;
    }//switch

    if (ii == endii)
      break;
    else
      ++ii;
  }//while true

  return count;
}

//
//
// HTTPReply
//
//

HTTPReply::HTTPReply(QTextStream &outstream)
  : dm_outs(outstream), dm_status(0),
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
  assert(!dm_calledcommit && "[setStatus() must be called BEFORE any output() calls]");
  dm_status = status;
}

void HTTPReply::setContentType(const QString &type)
{
  assert(!dm_calledcommit && "[setContentType() must be called BEFORE any output() calls]");
  assert(!type.isEmpty());

  dm_contenttype = type;
}

QTextStream & HTTPReply::output(void)
{
   if (dm_status == 0)
     dm_status = 200;
  if (!dm_calledcommit)
    commitHeader();
  return dm_outs;
}

void HTTPReply::redirectTo(const QString &rawurl)
{
  assert(!dm_calledcommit && "[setStatus() must be called BEFORE any output() calls]");
  assert(!rawurl.isEmpty());
  // Reply status really should be 303 (I think), but
  // netscape won're redirect if the status code is 303.
  // Look into cause later.
  setStatus(302);

  QStringList l("Location: " + rawurl);

  commitHeader(&l);
}

/*void HTTPReply::setServerCookie(const QString &name, const QString &value,
        const QString &expires, const QString &domain, const QString &path)
{
  assert(!dm_calledcommit && "[cannot call setServerCookie after commitHeader]");

  cookie_t c;

  c.name = name;
  c.value = value;
  c.expires = expires;
  c.domain = domain;
  c.path = path;

  dm_servercookies.push_back(c);
}*/

void HTTPReply::commitHeader(const QStringList *additionalHeaders)
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
    "Server: wexus/1.9.0\r\n";

  commitCookieHeader();

  if (additionalHeaders)
    for (QStringList::const_iterator ii=additionalHeaders->begin(); ii != additionalHeaders->end(); ++ii)
      dm_outs << *ii << "\r\n";

  dm_outs <<
    "\r\n";   // end of header, now the body

  // helpful incase the caller is calling device() and other
  // such magic
  dm_outs.flush();
}

void HTTPReply::commitCookieHeader(void)
{
  ServerCookies::const_iterator ii, endii;

  ii = dm_servercookies.begin();
  endii = dm_servercookies.end();
  QString cookie_hdr;

  cookie_hdr.reserve(128);

  for (; ii != endii; ++ii) {
//qDebug() << "SETTING COOKIE: " << ii.key() << ii->value;
    cookie_hdr = "Set-Cookie: ";
    cookie_hdr += ii.key() + "=" + ii->value.toString() + "; path=" + ii->path;

    if (!ii->domain.isEmpty())
      cookie_hdr += "; domain=" + ii->domain;

    if (!ii->expires.isEmpty())
      cookie_hdr += "; expires=" + ii->expires;

    cookie_hdr += "\r\n";

    // finally, send it out over the socket
    dm_outs << cookie_hdr;
  }

}

//
// HTTPHandler::Exception
//

/*HTTPHandler::Exception::Exception(void)
{
}*/

HTTPHandler::Exception::Exception(const QString &usermsg)
  : wexus::Exception(usermsg), dm_usermsg(usermsg)
{
}

HTTPHandler::Exception::~Exception() throw ()
{
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

ErrorHTTPHandler::ErrorHTTPHandler(void)
{
}

ErrorHTTPHandler::ErrorHTTPHandler(const QString &usermsg)
  : dm_usermsg(usermsg)
{
}

void ErrorHTTPHandler::handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  if (dm_usermsg.isEmpty())
    reply.output() << "Error 432: ErrorHTTPHandler called.";
  else
    reply.output() << "ErrorHTTPHandler: " << dm_usermsg;
}

