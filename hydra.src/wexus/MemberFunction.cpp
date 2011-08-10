
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MemberFunction.h>

#include <QDebug>

wexus::MemberFunction::MemberFunction(void)
{
}

wexus::MemberFunction::~MemberFunction()
{
}

bool wexus::MemberFunction::equals(const MemberFunction &rhs) const
{
  if (dm_typestring < rhs.dm_typestring)
    return true;
  if (dm_typestring > rhs.dm_typestring)
    return false;

  if (!dm_imp.get())
    return false;   // cant compare the null MemberFunction

  // else, theyre equal and we need to compare dm_mfn
  // equality means we can also do the following cast:

  return dm_imp->equals(*rhs.dm_imp);
}

// test code

/*class X
{
  public:
    void foo(void);
};
void X::foo(void)
{
}

void testfunc(void)
{
  wexus::MemberFunction f(&X::foo);
}*/

