
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUSHEADERMODELPARSER_H__
#define __INCLUDED_WEXUSHEADERMODELPARSER_H__

#include <QString>
#include <QIODevice>

#include <wexus/ModelTokenList.h>
#include <wexus/Exception.h>


namespace wexus
{
  class HeaderModelParser;
}

class wexus::HeaderModelParser
{
  public:
    class Exception : public wexus::Exception
    {
      public:
        /// usermessage constructor
        Exception(const QString &_what) throw();
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

