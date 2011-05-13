
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEUXS_HTMLTEMPLATEPARSER_H__
#define __INCLUDED_WEUXS_HTMLTEMPLATEPARSER_H__

#include <wexus/TemplateParser.h>

namespace wexus
{
  class HTMLTemplateParser;
}

class wexus::HTMLTemplateParser : public wexus::TemplateParser
{
  public:
    /**
     * Parses the given input stream and append found TemplateTokens
     * to the TemplateTokenList
     *
     * @author Aleksander Demko
     */ 
    virtual void parse(QIODevice &input, TemplateTokenList &outlist);
};

#endif

