
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
class wexus::FormParams
{
  public:
    class ParamNotFoundException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        ParamNotFoundException(const QString &paramName);
    };
    class FormDecodeException : public wexus::HTTPHandler::Exception   // TODO should this really decend from HTTPHandler::Exception?
    {
      public:
        FormDecodeException(void);
    };

  public:
    /// constructor
    FormParams(wexus::HTTPRequest *req);

    /**
     * Does this form set have a particular field?
     *
     * @author Aleksander Demko
     */ 
    bool contains(const QString &paramName);

    /**
     * Gets a field.
     * Returns an invalid QVariant on not found.
     *
     * what to do with this function?
     * rename it to get(), operator() or something else?
     *
     * @author Aleksander Demko
     */ 
    //const QVariant & param(const QString &paramName);

    /**
     * Gets a field.
     * Throws ParamNotFoundException on not found.
     *
     * @author Aleksander Demko
     */ 
    const QVariant & operator[](const QString &paramName);

  private:
    /// parse the linked HTTPRequest, if it hasnt already
    void parseRequest(void);

    template <class ITER>
      void decodeAndParse(ITER encodedBegin, ITER encodedEnd);

  private:
    wexus::HTTPRequest *dm_req;
    bool dm_processedreq;
    typedef std::map<QString, QVariant> params_t;
    params_t dm_params;
};

#endif


