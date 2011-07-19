
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FORMPARAMS_H__
#define __INCLUDED_WEXUS_FORMPARAMS_H__

#include <map>

#include <QString>
#include <QVariant>

#include <wexus/HTTPHandler.h>

namespace wexus
{
  class FormParams;
}

/**
 * Parses and holds all the form parameters,
 * which is essencially a QString->QVariant
 * map.
 *
 * @author Aleksander Demko
 */ 
class wexus::FormParams : public QVariantMap
{
  public:
    /*class ParamNotFoundException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        ParamNotFoundException(const QString &paramName);
    };*/
    class FormDecodeException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        FormDecodeException(void);
    };

  public:
    /**
     * Params the given req for fields and self fills.
     *
     * @author Aleksander Demko
     */ 
    FormParams(wexus::HTTPRequest *req);

  private:
    /// parse the linked HTTPRequest, if it hasnt already
    void parseRequest(wexus::HTTPRequest *req);

    template <class ITER>
      void decodeAndParse(ITER encodedBegin, ITER encodedEnd);

    static void nestedInsert(QVariantMap &m, const QString &key, const QString &val);
};

#endif


