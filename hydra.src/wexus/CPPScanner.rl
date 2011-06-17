
#include <wexus/CPPScanner.h>

#include <QDebug>
#include <QByteArray>

using namespace wexus;

// how to generate:
//  ragel $PWD/CPPScanner.rl -o $PWD/CPPScanner.cpp

/*
 * A C++ scanner. Uses the longest match construction.
 * << <= <<= >> >= >>= are left out since angle brackets are used in templates.
 */

wexus::CPPToken::CPPToken(int _id, QByteArray::const_iterator _ts, QByteArray::const_iterator _te)
  : id(_id), ts(_ts), te(_te)
{
  //qDebug() << "adding " << toString();
}

QString wexus::CPPToken::toString(void) const
{
  QString r;

  r = "(id=";

  switch (id) {
    case TK_Dlit: r += "TK_Dlit"; break;
    case TK_Slit: r += "TK_Slit"; break;
    case TK_Float: r += "TK_Float"; break;
    case TK_Id: r += "TK_Id"; break;
    case TK_NameSep: r += "TK_NameSep"; break;
    case TK_Arrow: r += "TK_Arrow"; break;
    case TK_PlusPlus: r += "TK_PlusPlus"; break;
    case TK_MinusMinus: r += "TK_MinusMinus"; break;
    case TK_ArrowStar: r += "TK_ArrowStar"; break;
    case TK_DotStar: r += "TK_DotStar"; break;
    case TK_ShiftLeft: r += "TK_ShiftLeft"; break;
    case TK_ShiftRight: r += "TK_ShiftRight"; break;
    case TK_IntegerDecimal: r += "TK_IntegerDecimal"; break;
    case TK_IntegerOctal: r += "TK_IntegerOctal"; break;
    case TK_IntegerHex: r += "TK_IntegerHex"; break;
    case TK_EqualsEquals: r += "TK_EqualsEquals"; break;
    case TK_NotEquals: r += "TK_NotEquals"; break;
    case TK_AndAnd: r += "TK_AndAnd"; break;
    case TK_OrOr: r += "TK_OrOr"; break;
    case TK_MultAssign: r += "TK_MultAssign"; break;
    case TK_DivAssign: r += "TK_DivAssign"; break;
    case TK_PercentAssign: r += "TK_PercentAssign"; break;
    case TK_PlusAssign: r += "TK_PlusAssign"; break;
    case TK_MinusAssign: r += "TK_MinusAssign"; break;
    case TK_AmpAssign: r += "TK_AmpAssign"; break;
    case TK_CaretAssign: r += "TK_CaretAssign"; break;
    case TK_BarAssign: r += "TK_BarAssign"; break;
    case TK_DotDotDot: r += "TK_DotDotDot"; break;
    case TK_Whitespace: r += "TK_Whitespace"; break;
    case TK_Comment: r += "TK_Comment"; break;
    default: r += static_cast<char>(id); break;
  }

  if (id == TK_Id)
    r += " \"" + value() + "\"";
  r += ")";

  return r;
}

QString wexus::CPPToken::value(void) const
{
  QString r;

  QByteArray::const_iterator ii = ts;

  for (; ii != te; ++ii)
    r += *ii;

  return r;
}

#define token(ID) outlist.push_back(CPPToken(ID, ts, te));

%%{
	machine CPPScanner; 

	# Floating literals.
	fract_const = digit* '.' digit+ | digit+ '.';
	exponent = [eE] [+\-]? digit+;
	float_suffix = [flFL];

	c_comment := 
		any* :>> '*/'
		@{ fgoto main; };

	main := |*

	# Single and double literals.
	( 'L'? "'" ( [^'\\\n] | /\\./ )* "'" ) 
		{token( CPPToken::TK_Slit );};
	( 'L'? '"' ( [^"\\\n] | /\\./ )* '"' ) 
		{token( CPPToken::TK_Dlit );};

	# Identifiers
	( [a-zA-Z_] [a-zA-Z0-9_]* ) 
		{token( CPPToken::TK_Id );};

	# Floating literals.
	( fract_const exponent? float_suffix? | digit+ exponent float_suffix? ) 
		{token( CPPToken::TK_Float );};
	
	# Integer decimal. Leading part buffered by float.
	( ( '0' | [1-9] [0-9]* ) [ulUL]{0,3} ) 
		{token( CPPToken::TK_IntegerDecimal );};

	# Integer octal. Leading part buffered by float.
	( '0' [0-9]+ [ulUL]{0,2} ) 
		{token( CPPToken::TK_IntegerOctal );};

	# Integer hex. Leading 0 buffered by float.
	( '0' ( 'x' [0-9a-fA-F]+ [ulUL]{0,2} ) ) 
		{token( CPPToken::TK_IntegerHex );};

	# Only buffer the second item, first buffered by symbol. */
	'::' {token( CPPToken::TK_NameSep );};
	'==' {token( CPPToken::TK_EqualsEquals );};
	'!=' {token( CPPToken::TK_NotEquals );};
	'&&' {token( CPPToken::TK_AndAnd );};
	'||' {token( CPPToken::TK_OrOr );};
	'*=' {token( CPPToken::TK_MultAssign );};
	'/=' {token( CPPToken::TK_DivAssign );};
	'%=' {token( CPPToken::TK_PercentAssign );};
	'+=' {token( CPPToken::TK_PlusAssign );};
	'-=' {token( CPPToken::TK_MinusAssign );};
	'&=' {token( CPPToken::TK_AmpAssign );};
	'^=' {token( CPPToken::TK_CaretAssign );};
	'|=' {token( CPPToken::TK_BarAssign );};
	'++' {token( CPPToken::TK_PlusPlus );};
	'--' {token( CPPToken::TK_MinusMinus );};
	'->' {token( CPPToken::TK_Arrow );};
	'->*' {token( CPPToken::TK_ArrowStar );};
	'.*' {token( CPPToken::TK_DotStar );};

	# Three char compounds, first item already buffered. */
	'...' {token( CPPToken::TK_DotDotDot );};

	# Single char symbols.
	( punct - [_"'] ) {token( ts[0] );};

	# Comments and whitespace.
	'/*' { fgoto c_comment; };
	'//' [^\n]* '\n';
	( any - 33..126 )+;

	*|;
}%%

// DATA
%% write data nofinal;

void wexus::CPPScanner(const QByteArray &ary, CPPTokenList &outlist)
{
  QByteArray::const_iterator p = ary.begin(), pe = ary.end(), ts, te;
  QByteArray::const_iterator eof = pe;
  int cs;
  int act;

	%% write init;

  %% write exec;
}

