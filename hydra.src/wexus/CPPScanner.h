
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CPPSCANNER_H__
#define __INCLUDED_WEXUS_CPPSCANNER_H__

#include <QList>
#include <QByteArray>

namespace wexus
{
  class CPPToken;

  typedef QList<CPPToken> CPPTokenList;

  void CPPScanner(const QByteArray &ary, CPPTokenList &outlist);
}

class wexus::CPPToken
{
  public:
    static const int TK_Dlit = 256;
    static const int TK_Slit = 257;
    static const int TK_Float = 258;
    static const int TK_Id = 259;
    static const int TK_NameSep = 260;
    static const int TK_Arrow = 261;
    static const int TK_PlusPlus = 262;
    static const int TK_MinusMinus = 263;
    static const int TK_ArrowStar = 264;
    static const int TK_DotStar = 265;
    static const int TK_ShiftLeft = 266;
    static const int TK_ShiftRight = 267;
    static const int TK_IntegerDecimal = 268;
    static const int TK_IntegerOctal = 269;
    static const int TK_IntegerHex = 270;
    static const int TK_EqualsEquals = 271;
    static const int TK_NotEquals = 272;
    static const int TK_AndAnd = 273;
    static const int TK_OrOr = 274;
    static const int TK_MultAssign = 275;
    static const int TK_DivAssign = 276;
    static const int TK_PercentAssign = 277;
    static const int TK_PlusAssign = 278;
    static const int TK_MinusAssign = 279;
    static const int TK_AmpAssign = 280;
    static const int TK_CaretAssign = 281;
    static const int TK_BarAssign = 282;
    static const int TK_DotDotDot = 283;
    static const int TK_Whitespace = 284;
    static const int TK_Comment = 285;
    // + a lot of single char tokens for val <256

  public:
    int id;
    QByteArray::const_iterator ts, te;

  public:
    CPPToken(int _id, QByteArray::const_iterator _ts, QByteArray::const_iterator _te);

    /// returns this token as a debug-printable like string
    QString toString(void) const;
    /// returns ts/te as a string
    QString value(void) const;

    operator int(void) const { return id; }
};

#endif

