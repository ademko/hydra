
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_TEMPLATETOKEN_H__
#define __INCLUDED_WEXUS_TEMPLATETOKEN_H__

#include <QByteArray>

namespace wexus
{
  class TemplateToken;
  class LiteralToken;
  class CodeToken;
}

/**
 * The base class of all the elements in a parsed
 * render node.
 *
 * @author Aleksander Demko
 */ 
class wexus::TemplateToken
{
  public:
    virtual ~TemplateToken();
};

class wexus::LiteralToken : public wexus::TemplateToken
{
  public:
    LiteralToken(const QByteArray &lit);

  private:
    QByteArray dm_lit;
};

class wexus::CodeToken : public wexus::TemplateToken
{
  public:
    CodeToken(char typ, const QByteArray &cod);

  private:
    char dm_typ;    // one of ' ' '='
    QByteArray dm_code;
};
#endif

