
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_STRINGUTIL_H__
#define __INCLUDED_WEXUS_STRINGUTIL_H__

#include <QString>
#include <QStringList>

#include <wexus/HTMLString.h>

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

  /**
   * Converts parts back into a CPP name
   *
   * @author Aleksander Demko
   */
  QString partsToCPP(const QStringList &parts);

  /**
   * Returns the same string with the first
   * letter lowercased.
   *
   * @author Aleksander Demko
   */ 
  QString lowerFirstChar(const QString &s);

  /**
   * Returns the same string with the first
   * letter lowercased.
   *
   * @author Aleksander Demko
   */ 
  QString upperFirstChar(const QString &s);

  /**
   * If the given string is null, return the default string instead.
   *
   * @author Aleksander Demko
   */
  QString checkNull(const QString &s, const QString &def);

  /**
   * If the given string is null, return the default string instead.
   *
   * Will we be needing HTMLString versions of everything?
   *
   * @author Aleksander Demko
   */
  HTMLString checkNull(const HTMLString &s, const QString &def);
};

#endif

