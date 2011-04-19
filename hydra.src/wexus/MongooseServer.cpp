
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <assert.h>

#include <string>
#include <sstream>

#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

#include <wexus/HTTP.h>

using namespace wexus;

MongooseServer::MongooseServer(const wexus::HTTPParams &opt)
{
  dm_ctx = 0;
  dm_opt = opt;
}

MongooseServer::~MongooseServer(void)
{
  if (isRunning()) {
    quit();
    wait();
  }
}

bool MongooseServer::isRunning(void) const
{
  return dm_ctx != 0;
}

void MongooseServer::start(void)
{
  if (isRunning())
    return;

qDebug() << "MongooseServer::start";
  std::ostringstream s;
  s << dm_opt.port();
  std::string port_s = s.str();

  const char * mgopts[] = {
    "listening_ports", port_s.c_str(),
    0
  };
  dm_ctx = mg_start(callback, this, mgopts);
  if (dm_ctx == 0)
    throw HTTPServerException();
}

void MongooseServer::quit(void)
{
  if (!isRunning())
    return;

qDebug() << "MongooseServer::quit";
  mg_stop(dm_ctx);
  dm_ctx = 0;
}

void MongooseServer::wait(void)
{
qDebug() << "MongooseServer::wait";
  // not needed, as quit() blocks
  if (isRunning()) {
    // definate block... figure something about better in the future
    QMutex m;
    QWaitCondition wc;

    // indefinate wait :)
    wc.wait(&m);
  }
}

//
// MongooseRequest
//

class MongooseRequest : public HTTPRequest
{
  public:
    MongooseRequest(const mg_request_info *request_info);
};

// Return HTTP header value, or NULL if not found.
static const char *get_header(const struct mg_request_info *ri,
                              const char *name) {
  int i;

  for (i = 0; i < ri->num_headers; i++)
    if (!strcmp(name, ri->http_headers[i].name))
      return ri->http_headers[i].value;
  return NULL;
}

MongooseRequest::MongooseRequest(const mg_request_info *request_info)
{
  dm_request = request_info->uri;
  dm_query = request_info->query_string;

  const char *c;

  c = get_header(request_info, "Referer");
  if (c)
    dm_referer = c;
  c = get_header(request_info, "User-Agent");
  if (c)
    dm_useragent = c;
}

//
// MongooseIODevice
//

class MongooseIODevice : public QIODevice
{
  public:
    MongooseIODevice(mg_connection *conn);

    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData(const char * data, qint64 maxSize);

  private:
    mg_connection *dm_conn;
};

MongooseIODevice::MongooseIODevice(mg_connection *conn)
  : dm_conn(conn)
{
  setOpenMode(QIODevice::WriteOnly);
}

qint64 MongooseIODevice::readData(char * data, qint64 maxSize)
{
  assert(false);
  return 0;
}

qint64 MongooseIODevice::writeData(const char * data, qint64 maxSize)
{
  return mg_write(dm_conn, data, maxSize);
}

void * MongooseServer::callback(enum mg_event event, struct mg_connection *conn,
        const struct mg_request_info *request_info)
{
  if (event != MG_NEW_REQUEST)
    return 0;

  MongooseServer *here = reinterpret_cast<MongooseServer*>(request_info->user_data);

  // build the request object
  MongooseRequest req(request_info);

  // build the reply object
  MongooseIODevice output_device(conn);
  QTextStream output_stream(&output_device);
  HTTPReply rep(output_stream);

  // process the request
qDebug() << "MongooseServer::callback handling: " << req.request();

  // the followin will eventually be moved into the HTTPServer base
  try {
    if (here->dm_opt.handler())
      here->dm_opt.handler()->handleRequest(req, rep);

    if (!rep.hasReply())
      ErrorHTTPHandler("Object not found: " + req.request()).handleRequest(req, rep);
  }
  catch (HTTPHandler::Exception &e) {
    ErrorHTTPHandler(e.userMessage()).handleRequest(req, rep);
  }

  return here;  // always returning non-null to show that ive handled the request
}

