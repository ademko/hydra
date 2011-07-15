
#line 1 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"

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


#line 156 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"


// DATA

#line 93 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
static const char _CPPScanner_actions[] = {
	0, 1, 0, 1, 1, 1, 3, 1, 
	4, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 1, 
	16, 1, 17, 1, 18, 1, 19, 1, 
	20, 1, 21, 1, 22, 1, 23, 1, 
	24, 1, 25, 1, 26, 1, 27, 1, 
	28, 1, 29, 1, 30, 1, 31, 1, 
	32, 1, 33, 1, 34, 1, 35, 1, 
	36, 1, 37, 1, 38, 1, 39, 1, 
	40, 1, 41, 1, 42, 1, 43, 1, 
	44, 1, 45, 1, 46, 2, 1, 2, 
	2, 4, 5, 2, 4, 6, 2, 4, 
	7, 2, 4, 8
};

static const unsigned char _CPPScanner_key_offsets[] = {
	0, 0, 3, 3, 6, 6, 7, 11, 
	13, 14, 20, 21, 23, 54, 56, 57, 
	58, 60, 61, 63, 66, 67, 71, 79, 
	85, 88, 98, 107, 111, 115, 119, 129, 
	133, 142, 143, 144, 151, 160, 161, 163
};

static const char _CPPScanner_trans_keys[] = {
	10, 34, 92, 10, 39, 92, 46, 43, 
	45, 48, 57, 48, 57, 10, 48, 57, 
	65, 70, 97, 102, 42, 42, 47, 33, 
	34, 37, 38, 39, 42, 43, 45, 46, 
	47, 48, 58, 61, 76, 94, 95, 124, 
	35, 44, 49, 57, 59, 64, 65, 90, 
	91, 96, 97, 122, 123, 126, 33, 126, 
	61, 61, 38, 61, 61, 43, 61, 45, 
	61, 62, 42, 42, 46, 48, 57, 69, 
	70, 76, 101, 102, 108, 48, 57, 70, 
	76, 102, 108, 48, 57, 42, 47, 61, 
	46, 69, 76, 85, 101, 108, 117, 120, 
	48, 57, 46, 69, 76, 85, 101, 108, 
	117, 48, 57, 76, 85, 108, 117, 76, 
	85, 108, 117, 76, 85, 108, 117, 76, 
	85, 108, 117, 48, 57, 65, 70, 97, 
	102, 76, 85, 108, 117, 46, 69, 76, 
	85, 101, 108, 117, 48, 57, 58, 61, 
	95, 48, 57, 65, 90, 97, 122, 34, 
	39, 95, 48, 57, 65, 90, 97, 122, 
	61, 61, 124, 0
};

static const char _CPPScanner_single_lengths[] = {
	0, 3, 0, 3, 0, 1, 2, 0, 
	1, 0, 1, 2, 17, 0, 1, 1, 
	2, 1, 2, 3, 1, 2, 6, 4, 
	3, 8, 7, 4, 4, 4, 4, 4, 
	7, 1, 1, 1, 3, 1, 2, 0
};

static const char _CPPScanner_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 1, 1, 
	0, 3, 0, 0, 7, 1, 0, 0, 
	0, 0, 0, 0, 0, 1, 1, 1, 
	0, 1, 1, 0, 0, 0, 3, 0, 
	1, 0, 0, 3, 3, 0, 0, 0
};

static const unsigned char _CPPScanner_index_offsets[] = {
	0, 0, 4, 5, 9, 10, 12, 16, 
	18, 20, 24, 26, 29, 54, 56, 58, 
	60, 63, 65, 68, 72, 74, 78, 86, 
	92, 96, 106, 115, 120, 125, 130, 138, 
	143, 152, 154, 156, 161, 168, 170, 173
};

static const char _CPPScanner_indicies[] = {
	0, 2, 3, 1, 1, 0, 5, 6, 
	4, 4, 8, 7, 9, 9, 10, 0, 
	10, 0, 12, 11, 14, 14, 14, 13, 
	16, 15, 16, 17, 15, 19, 1, 21, 
	22, 4, 23, 24, 25, 26, 27, 28, 
	30, 31, 33, 34, 32, 35, 20, 29, 
	20, 32, 20, 32, 20, 18, 36, 18, 
	38, 37, 39, 37, 40, 41, 37, 42, 
	37, 43, 44, 37, 45, 46, 47, 37, 
	49, 48, 50, 51, 52, 37, 54, 55, 
	55, 54, 55, 55, 52, 53, 55, 55, 
	55, 55, 10, 53, 56, 11, 57, 37, 
	52, 54, 60, 60, 54, 60, 60, 61, 
	59, 58, 52, 54, 63, 63, 54, 63, 
	63, 59, 62, 64, 64, 64, 64, 62, 
	65, 65, 65, 65, 58, 66, 66, 66, 
	66, 58, 68, 68, 68, 68, 14, 14, 
	14, 67, 69, 69, 69, 69, 67, 52, 
	54, 60, 60, 54, 60, 60, 29, 58, 
	70, 37, 71, 37, 32, 32, 32, 32, 
	72, 1, 4, 32, 32, 32, 32, 72, 
	73, 37, 74, 75, 37, 76, 0
};

static const char _CPPScanner_trans_targs[] = {
	12, 1, 12, 2, 3, 12, 4, 12, 
	12, 7, 23, 8, 12, 12, 30, 10, 
	11, 39, 13, 14, 12, 15, 16, 17, 
	18, 19, 21, 24, 25, 32, 33, 34, 
	35, 36, 37, 38, 12, 12, 12, 12, 
	12, 12, 12, 12, 12, 12, 12, 20, 
	12, 12, 12, 5, 22, 12, 6, 12, 
	12, 12, 12, 26, 28, 9, 12, 27, 
	12, 29, 12, 12, 31, 12, 12, 12, 
	12, 12, 12, 12, 0
};

static const char _CPPScanner_trans_actions[] = {
	83, 0, 11, 0, 0, 9, 0, 81, 
	55, 0, 0, 0, 61, 79, 0, 0, 
	0, 1, 0, 0, 57, 0, 0, 0, 
	0, 0, 7, 7, 94, 94, 0, 0, 
	0, 88, 0, 0, 77, 75, 25, 35, 
	27, 41, 31, 47, 37, 49, 39, 0, 
	73, 51, 53, 0, 91, 65, 0, 13, 
	59, 33, 67, 97, 0, 0, 69, 0, 
	17, 0, 15, 71, 0, 19, 21, 23, 
	63, 43, 45, 29, 0
};

static const char _CPPScanner_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 3, 0, 85, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char _CPPScanner_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 5, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const unsigned char _CPPScanner_eof_trans[] = {
	0, 1, 1, 1, 1, 8, 1, 1, 
	8, 14, 0, 0, 0, 37, 38, 38, 
	38, 38, 38, 38, 49, 38, 54, 54, 
	38, 59, 63, 63, 59, 59, 68, 68, 
	59, 38, 38, 73, 73, 38, 38, 0
};

static const int CPPScanner_start = 12;
static const int CPPScanner_error = 0;

static const int CPPScanner_en_c_comment = 10;
static const int CPPScanner_en_main = 12;


#line 160 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"

void wexus::CPPScanner(const QByteArray &ary, CPPTokenList &outlist)
{
  QByteArray::const_iterator p = ary.begin(), pe = ary.end(), ts, te;
  QByteArray::const_iterator eof = pe;
  int cs;
  int act;

	
#line 258 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
	{
	cs = CPPScanner_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 169 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"

  
#line 269 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _CPPScanner_actions + _CPPScanner_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 3:
#line 1 "NONE"
	{ts = p;}
	break;
#line 290 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
		}
	}

	_keys = _CPPScanner_trans_keys + _CPPScanner_key_offsets[cs];
	_trans = _CPPScanner_index_offsets[cs];

	_klen = _CPPScanner_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _CPPScanner_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _CPPScanner_indicies[_trans];
_eof_trans:
	cs = _CPPScanner_trans_targs[_trans];

	if ( _CPPScanner_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _CPPScanner_actions + _CPPScanner_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 94 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{ {cs = 12; goto _again;} }
	break;
	case 4:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 5:
#line 106 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{act = 3;}
	break;
	case 6:
#line 110 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{act = 4;}
	break;
	case 7:
#line 114 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{act = 5;}
	break;
	case 8:
#line 118 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{act = 6;}
	break;
	case 9:
#line 100 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_Slit );}}
	break;
	case 10:
#line 102 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_Dlit );}}
	break;
	case 11:
#line 110 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_Float );}}
	break;
	case 12:
#line 114 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_IntegerDecimal );}}
	break;
	case 13:
#line 118 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_IntegerOctal );}}
	break;
	case 14:
#line 122 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_IntegerHex );}}
	break;
	case 15:
#line 125 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_NameSep );}}
	break;
	case 16:
#line 126 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_EqualsEquals );}}
	break;
	case 17:
#line 127 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_NotEquals );}}
	break;
	case 18:
#line 128 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_AndAnd );}}
	break;
	case 19:
#line 129 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_OrOr );}}
	break;
	case 20:
#line 130 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_MultAssign );}}
	break;
	case 21:
#line 131 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_DivAssign );}}
	break;
	case 22:
#line 132 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_PercentAssign );}}
	break;
	case 23:
#line 133 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_PlusAssign );}}
	break;
	case 24:
#line 134 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_MinusAssign );}}
	break;
	case 25:
#line 135 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_AmpAssign );}}
	break;
	case 26:
#line 136 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_CaretAssign );}}
	break;
	case 27:
#line 137 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_BarAssign );}}
	break;
	case 28:
#line 138 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_PlusPlus );}}
	break;
	case 29:
#line 139 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_MinusMinus );}}
	break;
	case 30:
#line 141 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_ArrowStar );}}
	break;
	case 31:
#line 142 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_DotStar );}}
	break;
	case 32:
#line 145 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( CPPToken::TK_DotDotDot );}}
	break;
	case 33:
#line 148 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{token( ts[0] );}}
	break;
	case 34:
#line 151 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;{ {cs = 10; goto _again;} }}
	break;
	case 35:
#line 152 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p+1;}
	break;
	case 36:
#line 106 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_Id );}}
	break;
	case 37:
#line 110 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_Float );}}
	break;
	case 38:
#line 114 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_IntegerDecimal );}}
	break;
	case 39:
#line 118 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_IntegerOctal );}}
	break;
	case 40:
#line 122 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_IntegerHex );}}
	break;
	case 41:
#line 140 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( CPPToken::TK_Arrow );}}
	break;
	case 42:
#line 148 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;{token( ts[0] );}}
	break;
	case 43:
#line 153 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{te = p;p--;}
	break;
	case 44:
#line 114 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{{p = ((te))-1;}{token( CPPToken::TK_IntegerDecimal );}}
	break;
	case 45:
#line 148 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
	{{p = ((te))-1;}{token( ts[0] );}}
	break;
	case 46:
#line 1 "NONE"
	{	switch( act ) {
	case 0:
	{{cs = 0; goto _again;}}
	break;
	case 3:
	{{p = ((te))-1;}token( CPPToken::TK_Id );}
	break;
	case 4:
	{{p = ((te))-1;}token( CPPToken::TK_Float );}
	break;
	case 5:
	{{p = ((te))-1;}token( CPPToken::TK_IntegerDecimal );}
	break;
	case 6:
	{{p = ((te))-1;}token( CPPToken::TK_IntegerOctal );}
	break;
	}
	}
	break;
#line 549 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
		}
	}

_again:
	_acts = _CPPScanner_actions + _CPPScanner_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 1:
#line 1 "NONE"
	{ts = 0;}
	break;
	case 2:
#line 1 "NONE"
	{act = 0;}
	break;
#line 566 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.cpp"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _CPPScanner_eof_trans[cs] > 0 ) {
		_trans = _CPPScanner_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 171 "/mnt/x/workspace/hydra/hydra.src/wexus/CPPScanner.rl"
}

