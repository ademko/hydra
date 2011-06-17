
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/TemplateParser.h>

using namespace wexus;

TemplateParser::Exception::Exception(const QString &_what) throw()
  : dm_what(_what.toUtf8())
{
}

TemplateParser::Exception::~Exception() throw()
{
}

wexus::TemplateParser::~TemplateParser()
{
}

