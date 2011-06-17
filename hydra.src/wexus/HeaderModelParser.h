
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUSHEADERMODELPARSER_H__
#define __INCLUDED_WEXUSHEADERMODELPARSER_H__

#include <exception>

#include <QString>
#include <QIODevice>

#include <wexus/ModelTokenList.h>

namespace wexus
{
  class HeaderModelParser;
}

class wexus::HeaderModelParser
{
  public:
    class Exception : public std::exception
    {
      public:
        /// usermessage constructor
        Exception(const QString &_what) throw();
        virtual ~Exception() throw();

        virtual const char* what() const throw() { return dm_what; }

      private:
        // cant be a QString as then what() will return a * to a temporary
        QByteArray dm_what;
    };

  public:
    /// ctor
    HeaderModelParser(void);
    
    /**
     * Parsers the given input stream.
     *
     * @author Aleksander Demko
     */ 
    virtual void parse(QIODevice &input, wexus::ModelTokenList &outlist);

  private:
    static void tokenize(const QByteArray &ary, wexus::ModelTokenList &outlist);
};

#endif

