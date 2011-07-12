
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_STRINGUTIL_H__
#define __INCLUDED_WEXUS_STRINGUTIL_H__

#include <QString>
#include <QStringList>

namespace wexus
{
  /**
   * Converts a raw :: filled C++ typename
   * to a __ encoded one.
   *
   * Basically, converts : to _
   *
   * @author Aleksander Demko
   */ 
  QString colonsToUnderscores(const QString &s);
  /**
   * Splits the string (a raw :: filled C++ typename)
   * into parts by ::
   *
   * @author Aleksander Demko
   */ 
  void colonsToParts(const QString &s, QStringList &out);
  /**
   * Splits a __ encoded string back to parts
   *
   * @author Aleksander Demko
   */ 
  void underscoresToParts(const QString &s, QStringList &out);
};

#endif

