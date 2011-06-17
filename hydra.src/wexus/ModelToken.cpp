
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ModelToken.h>

#include <assert.h>

using namespace wexus;

ModelToken::ModelToken(char typ)
  : dm_type(typ)
{
}

LiteralModelToken::LiteralModelToken(char typ, const QByteArray &lit)
  : ModelToken(typ), dm_data(lit)
{
  assert(typ == 'L');
}

FieldModelToken::FieldModelToken(char typ,
        const QString &fieldName, const QString &fieldType)
  : ModelToken(typ), dm_fieldName(fieldName), dm_fieldType(fieldType)
{
  assert(typ == 'F');
}

