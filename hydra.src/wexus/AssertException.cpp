
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/AssertException.h>

#include <assert.h>

using namespace wexus;

// global

AssertThrower wexus::required;

//
// AssertException
//

AssertException::AssertException(const char *msg)
  : Exception(msg)
{
  assert(msg);
}

AssertException::AssertException(const QString &msg)
  : Exception(msg)
{
}

//
// functions
//

void wexus::assertThrowMsg(bool b, const char *msg)
{
  assert(msg);
  if (!b)
    throw AssertException(msg);
}

//
// AssertThrower
//

bool AssertThrower::operator = (bool b)
{
  assertThrowMsg(b, "AssertThrower(bool)");
  return b;
}

const QVariant & AssertThrower::operator = (const QVariant &v)
{
  assertThrowMsg(v.isValid(), "AssertThrower(QVariant)");
  return v;
}

const QString & AssertThrower::operator = (const QString &s)
{
  assertThrowMsg(!s.isEmpty(), "AssertThrower(QString)");
  return s;
}

