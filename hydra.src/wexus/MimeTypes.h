
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MIMETYPES_H__
#define __INCLUDED_WEXUS_MIMETYPES_H__

#include <map>

#include <wexus/HTTPServer.h>

namespace wexus
{
  class MimeTypes;
}

/**
 * A databaset, well map, of mime types and their file extensions.
 *
 * @author Aleksander Demko
 */ 
class wexus::MimeTypes
{
  public:
    class MimeTypesException : public wexus::HTTPException
    {
      public:
        MimeTypesException(const QString &usermsg);
    };

  public:
    /// destructor
    ~MimeTypes();

    /**
     * Returns the singleton instance, if any.
     *
     * @author Aleksander Demko
     */ 
    static MimeTypes *instance(void);

    /**
     * Returns the singleton instance, and creates it
     * if need be.
     *
     * @author Aleksander Demko
     */ 
    static MimeTypes *instanceCreate(void);

    /**
     * Is there a mime type of this extension?
     * ext must already be all lower case and shouldn't include
     * any periods.
     *
     * @author Aleksander Demko
     */
    bool hasMimeType(const QString &ext);

    /**
     * Returns the mime type for the given extension.
     * Throws an exception if it doesn't exist.
     * ext must already be all lower case and shouldn't include
     * any periods.
     *
     * @author Aleksander Demko
     */ 
    const QString & mimeType(const QString &ext);

    static QString binaryMimeType(void) { return "application/octet-stream"; }

  protected:
    /**
     * Constructor.
     *
     * @author Aleksander Demko
     */ 
    MimeTypes(void);

  private:
    void addMimeType(const QString &type, const QString &ext);
    void addMimeType(const QString &type, const QStringList &extlist);

    void initData(void);

  private:
    typedef std::shared_ptr<QString> data_t;
    typedef std::map<QString, data_t> map_t;

    static MimeTypes *dm_instance;
    
    // the extension to mime type map
    map_t dm_ext;
};

#endif
