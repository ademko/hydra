
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
 * @author Aleksander Demko
 */ 
class wexus::MarkDown
{
  public:
    enum {
      Format_Lists = 1,    // numeric and non-numeric lists
      Format_Quotes = 2,   // post quoting and code quote

      Format_Links = 4,   // normal and wiki links

      Format_Titles = 8,   // headings/titles

      Format_None = 0,      // html encoding and paragraph coding is ALWAYS ON
      Format_Basic = Format_Lists|Format_Quotes,
      Format_BlogPost = 0x7F, // ALL flags
      Format_Comment = Format_Basic,
      Format_WikiPage = Format_BlogPost, // ALL flags

      // FUTURE TODO
      // Return_All = 0
      // Return_FirstTitle =
      // Return_FirstPara
    };

    /**
     * Format the given string as user markup.
     *
     * As flags, either choose Format_Basic, Format_Post Format_Wiki, or if you
     * want total controler, the indiviual type flags.
     *
     * @author Aleksander Demko
     */ 
    static QByteArray process(const QByteArray &input, int flags = Format_Basic);
};

#endif

