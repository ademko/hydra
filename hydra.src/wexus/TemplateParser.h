
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_TEMPLATEPARSER_H__
#define __INCLUDED_WEXUS_TEMPLATEPARSER_H__

#include <QString>
#include <QIODevice>

#include <wexus/TemplateTokenList.h>
#include <wexus/Exception.h>

namespace wexus
{
  class TemplateParser;
}

class wexus::TemplateParser
{
  public:
    class Exception : public wexus::Exception
    {
      public:
        /// usermessage constructor
        Exception(const QString &_what) throw();
    };

  public:
    virtual ~TemplateParser();
    /**
     * Parses the given input stream and append found TemplateTokens
     * to the TemplateTokenList
     *
     * @author Aleksander Demko
     */ 
    virtual void parse(QIODevice &input, wexus::TemplateTokenList &outlist) = 0;
};

#endif
