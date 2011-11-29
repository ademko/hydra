
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
#include <QDateTime>
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

  std::ostringstream s;
  s << dm_opt.port();
  std::string port_s = s.str();

  s.str("");
  s << dm_opt.numThreads();
  std::string numthreads_s = s.str();

  const char * mgopts[] = {
    "listening_ports", port_s.c_str(),
    "num_threads", numthreads_s.c_str(),
    0
  };
qDebug() << "MongooseServer::start port=" << dm_opt.port() << " threads=" << dm_opt.numThreads(); 
  dm_ctx = mg_start(callback, this, mgopts);
  if (dm_ctx == 0) {
    throw PortInUseException(dm_opt.port());
  }
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
    MongooseRequest(const mg_request_info *request_info, QIODevice *inputdev);
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

MongooseRequest::MongooseRequest(const mg_request_info *request_info, QIODevice *inputdev)
{
  dm_request = request_info->uri;
  dm_query = request_info->query_string;

  const char *c;

  c = get_header(request_info, "Referer");
  if (c)
    dm_referer = c;
  else
    dm_referer.clear();
  c = get_header(request_info, "User-Agent");
  if (c)
    dm_useragent = c;
  else
    dm_useragent.clear();
  dm_clientcookies.clear();
  c = get_header(request_info, "Cookie");
  if (c)
    parseCookies(c);
  c = get_header(request_info, "Content-Length");
  if (c) {
    bool ok = false;
    dm_contentlength = QString(c).toLongLong(&ok);
    if (!ok)
      dm_contentlength = 0;
  } else
    dm_contentlength = 0;

  dm_inputdev = inputdev;
}

//
// OutputMongooseIODevice
//

class OutputMongooseIODevice : public QIODevice
{
  public:
    OutputMongooseIODevice(mg_connection *conn);

    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData(const char * data, qint64 maxSize);

  private:
    mg_connection *dm_conn;
};

OutputMongooseIODevice::OutputMongooseIODevice(mg_connection *conn)
  : dm_conn(conn)
{
  setOpenMode(QIODevice::WriteOnly);
}

qint64 OutputMongooseIODevice::readData(char * data, qint64 maxSize)
{
  assert(false);
  return 0;
}

qint64 OutputMongooseIODevice::writeData(const char * data, qint64 maxSize)
{
  return mg_write(dm_conn, data, maxSize);
}

//
// InputMongooseIODevice
//

// i didnt want to merge these to because I dont
// want users thinkin they can read from output()
// and write to input(), etc
class InputMongooseIODevice : public QIODevice
{
  public:
    InputMongooseIODevice(mg_connection *conn);

    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData(const char * data, qint64 maxSize);

  private:
    mg_connection *dm_conn;
};

InputMongooseIODevice::InputMongooseIODevice(mg_connection *conn)
  : dm_conn(conn)
{
  setOpenMode(QIODevice::ReadOnly);
}

qint64 InputMongooseIODevice::readData(char * data, qint64 maxSize)
{
  return mg_read(dm_conn, data, maxSize);
}

qint64 InputMongooseIODevice::writeData(const char * data, qint64 maxSize)
{
  assert(false);
  return 0;
}

void * MongooseServer::callback(enum mg_event event, struct mg_connection *conn,
        const struct mg_request_info *request_info)
{
  if (event != MG_NEW_REQUEST)
    return 0;

  MongooseServer *here = reinterpret_cast<MongooseServer*>(request_info->user_data);

  // build the request object
  InputMongooseIODevice input_device(conn);
  MongooseRequest req(request_info, &input_device);

  // build the reply object
  OutputMongooseIODevice output_device(conn);
  QTextStream output_stream(&output_device);
  HTTPReply rep(output_stream);

  // process the request
qDebug() << "MongooseServer::callback handling: " << req.request();

  // the followin will eventually be moved into the HTTPServer base
  try {
    if (here->dm_opt.handler())
      here->dm_opt.handler()->handleRequest(req, rep);

    if (!rep.hasReply())
      ErrorHTTPHandler("MongooseServer: unhandled request: " + req.request()).handleRequest(req, rep);
  }
  catch (HTTPHandler::Exception &e) {
    ErrorHTTPHandler(e.userMessage()).handleRequest(req, rep);
  }
  catch (std::exception &e) {
    QDateTime now(QDateTime::currentDateTime());

qDebug() << now.toString() << "what()" << e.what();
    ErrorHTTPHandler("MongooseServer: std::exception @" + now.toString()).handleRequest(req, rep);
  }

  return here;  // always returning non-null to show that ive handled the request
}

