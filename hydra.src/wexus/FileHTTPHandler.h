
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FILEHTTPHANDLER_H__
#define __INCLUDED_WEXUS_FILEHTTPHANDLER_H__

#include <wexus/HTTP.h>
#include <wexus/HTTPServer.h>

namespace wexus
{
  class FileHTTPHandler;
}

/**
 * A HTTP handler that servces up statis files.
 *
 * @author Aleksander Demko
 */ 
class wexus::FileHTTPHandler : public wexus::HTTPHandler
{
  public:
    class FileException : public wexus::HTTPException
    {
      public:
        FileException(const QString &usermsg);
    };
  public:
    /**
     * Constructor.
     * dir is the dir to serve files from. Subdirectories will be included.
     *
     * @author Aleksander Demko
     */ 
    FileHTTPHandler(const QString &docdir);

    /// handler
    void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  private:
    QString dm_docdir;
};

#endif

