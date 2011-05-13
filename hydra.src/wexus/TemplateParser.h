
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_TEMPLATEPARSER_H__
#define __INCLUDED_WEXUS_TEMPLATEPARSER_H__

#include <exception>

#include <QString>
#include <QIODevice>

#include <wexus/TemplateTokenList.h>

namespace wexus
{
  class TemplateParser;
}

class wexus::TemplateParser
{
  public:
    class Exception : public std::exception
    {
      public:
        /// usermessage constructor
        Exception(const QString &_what) throw();
        virtual ~Exception() throw();

        virtual const char* what() const throw() { return dm_what.toUtf8(); }

      private:
        QString dm_what;
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
