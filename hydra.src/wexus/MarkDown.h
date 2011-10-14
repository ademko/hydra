
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MARKDOWN_H__
#define __INCLUDED_WEXUS_MARKDOWN_H__

#include <QByteArray>

#include <wexus/HTMLString.h>

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
    };

    /**
     * Format the given markdown code as HTML markup.
     *
     * As flags, either choose Format_Basic, Format_Post Format_Wiki, or if you
     * want total controler, the indiviual type flags.
     *
     * @author Aleksander Demko
     */ 
    static QByteArray process(const QByteArray &markdown, int flags = Format_Basic);

    /**
     * Returns the title, already rendered as an HTML string from the givenmarkdown code.
     *
     * @author Aleksander Demko
     */ 
    static HTMLString title(const QByteArray &markdown);

    /**
     * Returns the first paragraph, already rendered as an HTML string from the givenmarkdown code.
     *
     * @author Aleksander Demko
     */ 
    static HTMLString firstPara(const QByteArray &markdown);
};

#endif

