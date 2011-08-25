
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MARKDOWN_H__
#define __INCLUDED_WEXUS_MARKDOWN_H__

#include <QByteArray>

namespace wexus
{
  class MarkDown;
}

/**
 * A Markdown like formatter.
 *
 * This needs work.
 * Currently supports:
 *
 * *bold* _italics_ =title=
 * [wikilink] [[reallink]]
 * *list item
 *
 * TODO reclean this function, support more MarkDown, etc
 *
 * @author Aleksander Demko
 */ 
class wexus::MarkDown
{
  public:
    enum {
      Format_Links= 1,   // normal links
      Format_Wikilinks= 2,
      Format_Bold_italics= 4,
      Format_Titles= 8,   // headings/titles
      Format_Lists= 16,    // numeric and non-numeric lists
      Format_Quotes= 32,   // post quoting and code quote
      Format_Codequotes= 64,   // newline+space == fixed width quotes

      Format_Basic= 0,      // html encoding and paragraph coding is ALWAYS ON
      Format_Post= Format_Links|Format_Bold_italics|Format_Quotes,
      Format_Wiki= 0x7F, // ALL flags
    };

    /**
     * Format the given string as user markup.
     *
     * As flags, either choose Format_Basic, Format_Post Format_Wiki, or if you
     * want total controler, the indiviual type flags.
     *
     * @author Aleksander Demko
     */ 
    static QByteArray process(const QByteArray &input, unsigned int flags = Format_Basic);
};
#endif

