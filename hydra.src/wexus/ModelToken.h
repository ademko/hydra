
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_MODELTOKEN_H__
#define __INCLUDED_WEXUS_MODELTOKEN_H__

#include <QByteArray>
#include <QString>

namespace wexus
{
  class ModelToken;
  class LiteralModelToken;
  class FieldModelToken;
}

/**
 * The base class of all model tokens.
 *
 * @author Aleksander Demko
 */ 
class wexus::ModelToken
{
  public:
    /**
     * Constructor.
     *
     * @param typ the type of token, one of 'S' (fields section) 'F' (fields) or 'L' (literal)
     * @param lit the literal code/byte stream
     *
     * @author Aleksander Demko
     */ 
    ModelToken(char typ);
    
    /// dtor marked virtual
    virtual ~ModelToken() { }

    char type(void) const { return dm_type; }

  private:
    char dm_type;    // one of ' ' '=' 'L' (literal)
};

class wexus::LiteralModelToken : public wexus::ModelToken
{
  public:
    /**
     * Constructor.
     *
     * @param typ the type of token, one of 'S' (fields section) 'F' (fields) or 'L' (literal)
     * @param lit the literal code/byte stream
     *
     * @author Aleksander Demko
     */ 
    LiteralModelToken(char typ, const QByteArray &lit);

    const QByteArray &data(void) const { return dm_data; }
    QByteArray &data(void) { return dm_data; }

  private:
    QByteArray dm_data;
};

class wexus::FieldModelToken : public wexus::ModelToken
{
  public:
    /**
     * Constructor.
     *
     * @param typ the type of token, one of 'S' (fields section) 'F' (a field) or 'L' (literal)
     * @param lit the literal code/byte stream
     *
     * @author Aleksander Demko
     */ 
    FieldModelToken(char typ,
        const QString &fieldName, const QString &fieldType,
        const QString &fieldValidationExpr,
        const QString &fieldInitLit);

    const QString & fieldName(void) const { return dm_fieldName; }
    const QString & fieldType(void) const { return dm_fieldType; }
    const QString & fieldValidationExpr(void) const { return dm_fieldValidationExpr; }
    const QString & fieldInitLit(void) const { return dm_fieldInitLit; }

  private:
    QString dm_fieldName, dm_fieldType, dm_fieldValidationExpr, dm_fieldInitLit;
};

#endif

