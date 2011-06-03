
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
    FormParams(void);

    /**
     * Sets the wexus::HTTPRequest that will be used
     * if any params are requested.
     *
     * @author Aleksander Demko
     */ 
    void setupRequest(wexus::HTTPRequest *req);

    /**
     * Does this form set have a particular field?
     *
     * @author Aleksander Demko
     */ 
    bool has(const QString &paramName);

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

    void decodeAndParse(QString::const_iterator encodedBegin, QString::const_iterator encodedEnd);

  private:
    wexus::HTTPRequest *dm_req;
    bool dm_processedreq;
    typedef std::map<QString, QVariant> params_t;
    params_t dm_params;
    QVariant dm_emptyvariant;
};

#endif


