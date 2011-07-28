
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_PARAMSPARSER_H__
#define __INCLUDED_WEXUS_PARAMSPARSER_H__

#include <map>

#include <QString>
#include <QVariant>

#include <wexus/HTTPHandler.h>

namespace wexus
{
  class ParamsParser;
}

/**
 * Parsers form params.
 *
 * @author Aleksander Demko
 */ 
class wexus::ParamsParser
{
  public:
    class ParamsDecodeException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        ParamsDecodeException(void);
    };

  public:
    /**
     * Parse the form params out of the given wexus::HTTPRequest
     *
     * @author Aleksander Demko
     */ 
    static QVariantMap parse(wexus::HTTPRequest *req);

    /**
     * Parse the form params out of the given wexus::HTTPRequest
     *
     * @author Aleksander Demko
     */ 
    static void parse(wexus::HTTPRequest *req, QVariantMap &out);

  private:
    template <class ITER>
      static void decodeAndParse(ITER encodedBegin, ITER encodedEnd, QVariantMap &out);

    static void nestedInsert(QVariantMap &m, const QString &key, const QString &val);
};

#endif


