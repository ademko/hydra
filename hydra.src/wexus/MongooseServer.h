
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MONGOOSESERVER_H__
#define __INCLUDED_WEXUS_MONGOOSESERVER_H__

#include <wexus/HTTPServer.h>

#include <stddef.h>   // so size_t is available for mongoose.h
#include "mongoose.h"

namespace wexus
{
  class MongooseServer;
}

/**
 * A mongoose based wexus::HTTPServer implementation.
 *
 * @author Aleksander Demko
 */ 
class wexus::MongooseServer : public wexus::HTTPServer
{
  public:
    /**
     * Creates a new web server. This doesn't actually start it thougth,
     * start() does that.
     *
     * @author Aleksander Demko
     */ 
    MongooseServer(const wexus::HTTPParams &opt);
    /**
     * Destructor. If the server is running, this will also call
     * quit() and then wait()
     *
     * @author Aleksander Demko
     */ 
    virtual ~MongooseServer(void);

    /// is the web server currently running?
    virtual bool isRunning(void) const;
    /// starts the web server
    virtual void start(void);
    /// notifies the web server to stop. you should still do a wait()
    /// this function may or may not block
    virtual void quit(void);
    /// waits until the server finishes
    virtual void wait(void);

  private:
    static void *callback(enum mg_event event, struct mg_connection *conn,
        const struct mg_request_info *request_info);

  private:
    wexus::HTTPParams dm_opt;
    mg_context *dm_ctx;
};

#endif

