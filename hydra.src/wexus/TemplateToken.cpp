
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/TemplateToken.h>

#include <QDebug>

using namespace wexus;

TemplateToken::TemplateToken(int lineno, char typ, const QByteArray &cod)
  : dm_lineno(lineno), dm_type(typ), dm_data(cod)
{
}

