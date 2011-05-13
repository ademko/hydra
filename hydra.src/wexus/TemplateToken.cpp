
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/TemplateToken.h>

#include <QDebug>

using namespace wexus;

TemplateToken::~TemplateToken()
{
}

LiteralToken::LiteralToken(const QByteArray &lit)
  : dm_lit(lit)
{
  qDebug() << __FUNCTION__ << dm_lit;
}

CodeToken::CodeToken(char typ, const QByteArray &cod)
  : dm_typ(typ), dm_code(cod)
{
  qDebug() << __FUNCTION__ << dm_typ << dm_code;
}

