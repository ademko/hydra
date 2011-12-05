
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/StringUtil.h>

#include <assert.h>

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


QString wexus::partsToCPP(const QStringList &parts)
{
  return parts.join("::");
}

QString wexus::lowerFirstChar(const QString &s)
{
  assert(!s.isEmpty());

  QString r(s);

  r[0] = r[0].toLower();

  return r;
}

QString wexus::upperFirstChar(const QString &s)
{
  assert(!s.isEmpty());

  QString r(s);

  r[0] = r[0].toUpper();

  return r;
}

QString wexus::checkNull(const QString &s, const QString &def)
{
  if (s.isEmpty())
    return def;
  else
    return s;
}
HTMLString wexus::checkNull(const HTMLString &s, const QString &def)
{
  if (s.isEmpty())
    return HTMLString(def);
  else
    return s;
}
