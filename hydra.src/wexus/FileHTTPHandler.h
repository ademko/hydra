
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FILEHTTPHANDLER_H__
#define __INCLUDED_WEXUS_FILEHTTPHANDLER_H__

#include <wexus/HTTPHandler.h>
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
    enum FileHTTPHandlerFlags {
      /// shows the index.html or index.htm file if given a directory
      /// takes precedance over FileHTTPHandlerFlags::AutoDirIndex
      IndexHtml = 0x1,
      /// auto generates an index if given a directory
      AutoDirIndex = 0x2,
      /// only handle files for which there is a mime type (you usually don't want this)
      /// otherwise unkown types will be sent as application/octet-stream
      AllowAllMimeTypes = 0x4,
    };
  public:
    class FileException : public Exception
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
    FileHTTPHandler(const QString &docdir, int flags = 0);

    /// handler
    virtual void handleRequest(wexus::HTTPRequest &req, wexus::HTTPReply &reply);

    /**
     * The core handler itself, reusable in other places
     * (like FileApp) and without an FileHTTPHandler instance.
     *
     * @author Aleksander Demko
     */ 
    static void processRequest(int flags, const QString &docdir, const QString &relpath, wexus::HTTPReply &reply);

    /**
     * Sends the given file to the given device (often reply.output().device().
     *
     * Returns true on success, false on cant-open-file,
     * and throws on critical errors.
     *
     * @author Aleksander Demko
     */ 
    static bool sendFile(const QString &filename, QIODevice * outputdev);

  protected:
    static void generateDirIndex(const QString &fullpath, const QString &relpath, wexus::HTTPReply &reply);

  protected:
    QString dm_docdir;
    int dm_flags;
};

#endif

