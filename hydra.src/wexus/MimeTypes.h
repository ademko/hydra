
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MIMETYPES_H__
#define __INCLUDED_WEXUS_MIMETYPES_H__

#include <map>

#include <wexus/TR1.h>
#include <wexus/HTTPHandler.h>

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
    class MimeTypesException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        MimeTypesException(const QString &usermsg);
    };

  public:
    /**
     * Constructor.
     *
     * @author Aleksander Demko
     */ 
    MimeTypes(void);
    /// destructor
    ~MimeTypes();

    /**
     * Returns the singleton instance. May return null
     * if there is none.
     *
     * @author Aleksander Demko
     */ 
    static MimeTypes *instance(void);

    /**
     * Is there a mime type of this extension?
     * ext must already be all lower case and shouldn't include
     * any periods.
     *
     * @author Aleksander Demko
     */
    static bool containsMimeType(const QString &ext);

    /**
     * Returns the mime type for the given extension.
     * Throws an exception if it doesn't exist.
     * ext must already be all lower case and shouldn't include
     * any periods.
     *
     * @author Aleksander Demko
     */ 
    static const QString & mimeType(const QString &ext);

    static QString binaryMimeType(void) { return "application/octet-stream"; }

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

