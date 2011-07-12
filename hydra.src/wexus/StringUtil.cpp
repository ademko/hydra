
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/StringUtil.h>

using namespace wexus;

QString wexus::colonsToUnderscores(const QString &s)
{
  QString r(s);

  for (QString::iterator ii=r.begin(); ii !=r.end(); ++ii)
    if (*ii == ':')
      *ii = '_';

  return r;
}

void wexus::colonsToParts(const QString &s, QStringList &out)
{
  out = s.split("::");
}

void wexus::underscoresToParts(const QString &s, QStringList &out)
{
  out = s.split("__");
}


