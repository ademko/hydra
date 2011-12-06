
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Exception.h>


wexus::Exception::Exception(const QString &_what) throw()
  : dm_whatbuf(_what.toUtf8())
{
}

wexus::Exception::~Exception() throw()
{
}

