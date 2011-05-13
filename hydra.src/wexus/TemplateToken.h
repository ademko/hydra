
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_TEMPLATETOKEN_H__
#define __INCLUDED_WEXUS_TEMPLATETOKEN_H__

#include <QByteArray>

namespace wexus
{
  class TemplateToken;
}

/**
 * The base class of all the elements in a parsed
 * render node.
 *
 * @author Aleksander Demko
 */ 
class wexus::TemplateToken
{
  public:
    TemplateToken(int lineno, char typ, const QByteArray &cod);

    int lineno(void) const { return dm_lineno; }
    char type(void) const { return dm_type; }
    const QByteArray &data(void) const { return dm_data; }
    QByteArray &data(void) { return dm_data; }

  private:
    int dm_lineno;
    char dm_type;    // one of ' ' '=' 'L' (literal)
    QByteArray dm_data;
};

#endif

