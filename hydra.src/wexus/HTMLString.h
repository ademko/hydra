
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_HTMLSTRING_H__
#define __INCLUDED_WEXUS_HTMLSTRING_H__

#include <QString>

namespace wexus
{
  class HTMLString;
}

/**
 * This is a QString that is safe for emiting directly to the
 * user (that is, it has been properly escaped)
 *
 * @author Aleksander Demko
 */ 
class wexus::HTMLString : public QString
{
  public:
    HTMLString(void);

    /**
     * HTML encode the given string and return
     * the encoded string.
     *
     * @author Aleksander Demko
     */ 
    static HTMLString encode(const QString &s);

    /**
     * Return the current string, but as a HTMLString.
     * It is assumed the string is already encoded
     * for HTML rendering.
     *
     * @author Aleksander Demko
     */ 
    static HTMLString raw(const QString &s) { return HTMLString(s); }

  protected:
    explicit HTMLString(const QString &s);
};

#endif

