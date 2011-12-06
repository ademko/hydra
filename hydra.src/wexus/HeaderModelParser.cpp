
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HeaderModelParser.h>

#include <QDebug>

#include <wexus/CPPScanner.h>

using namespace wexus;

HeaderModelParser::Exception::Exception(const QString &_what) throw()
  : wexus::Exception(_what)
{
}

HeaderModelParser::HeaderModelParser(void)
{
}

void HeaderModelParser::parse(QIODevice &input, ModelTokenList &outlist)
{
  QByteArray ary(input.readAll());

  if (ary.isEmpty())
    return;

  CPPTokenList cpptoklist;

  CPPScanner(ary, cpptoklist);

  if (cpptoklist.isEmpty())
    return;

  {
    CPPTokenList::const_iterator p = cpptoklist.begin(), pe = cpptoklist.end();
    QByteArray::const_iterator lit_start = ary.end(), lit_end;
    //int cs;
    enum {
      looking_for_field,
      got_field_want_colon,
      in_fields_want_type,
      in_fields_want_name,
      in_fields_want_semi,
      in_fields_want_validationexpr,
    } state = looking_for_field;
    QString fieldType, fieldName;
    QString fieldValidationExpr;
    QString fieldInitLit;
    int parencount = 0;
    bool in_initlit = false;

    for (; p != pe; ++p) {
      if (lit_start == ary.end())
        lit_start = p->ts;

      switch (state) {
        case looking_for_field:
          if (p->id == CPPToken::TK_Id && p->value() == "fields") {
            lit_end = p->ts;    // this might be the literal token end!
            state = got_field_want_colon;
          }
          // else is all OK
          break;
        case got_field_want_colon:
          if (p->id == ':') {
            state = in_fields_want_type;

            fieldType.clear();

            outlist.push_back(std::shared_ptr<LiteralModelToken>(new LiteralModelToken('L',
                    ary.mid(lit_start - ary.begin(), lit_end - lit_start))));
            outlist.push_back(std::shared_ptr<ModelToken>(new ModelToken('S')));
            lit_start = ary.end();
          } else
            state = looking_for_field;    // back to normal scanning
          break;
        case in_fields_want_type:
          if (p->id == CPPToken::TK_Id) {
            fieldType += p->value();
            state = in_fields_want_name;
            fieldValidationExpr.clear();
            fieldInitLit.clear();
          } else if (p->id == '}') {
            state = looking_for_field;  // done
          } else
            throw Exception("Expected field type");
          break;
        case in_fields_want_name:
          if (p->id == CPPToken::TK_Id) {
            fieldName = p->value();
            if (!fieldName[0].isLower())
              throw Exception("Field names must start with a lower case letter: " + fieldName);
            state = in_fields_want_semi;
          } else if (p->id == CPPToken::TK_NameSep) {
            // got a ::, go back to building the type
            fieldType += "::";
            state = in_fields_want_type;
          } else if (p->id == ':') {
            state = looking_for_field;  // done via protected: etc
          } else
            throw Exception("Expected field name");
          break;
        case in_fields_want_semi:
          if (p->id == '=' && !in_initlit) {
            in_initlit = true;
          } else if (p->id == '(') {
            state = in_fields_want_validationexpr;
            parencount = 0;
          } else if (p->id == ';') {
            outlist.push_back(std::shared_ptr<ModelToken>(new FieldModelToken('F', fieldName, fieldType, fieldValidationExpr, fieldInitLit)));
            state = in_fields_want_type;
            fieldType.clear();
            lit_start = ary.end();
            in_initlit = false;
          } else if (in_initlit) {
            fieldInitLit += " " + p->value();
          } else
            throw Exception("Can't find a ; to terminate the field");
          break;
        case in_fields_want_validationexpr:
          if (p->id == ')' && parencount == 0) {
            // time to break out of this state
            state = in_fields_want_semi;
            break;
          }
          if (p->id == '(')
            parencount++;
          if (p->id == ')')
            parencount--;
          fieldValidationExpr += " " + p->value();
          break;
      }//switch
    }//for p

    // push the trailing literlal, to, if any
    if (lit_start != ary.end())
      outlist.push_back(std::shared_ptr<LiteralModelToken>(new LiteralModelToken('L',
              ary.mid(lit_start - ary.begin(), ary.end() - lit_start))));
  }
}

