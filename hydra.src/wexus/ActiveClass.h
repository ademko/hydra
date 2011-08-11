
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ACTIVECLASS_H__
#define __INCLUDED_WEXUS_ACTIVECLASS_H__

#include <assert.h>

#include <wexus/TR1.h>

#include <QString>
#include <QMap>
#include <QVector>
#include <QVariant>
#include <QSqlDatabase>

#include <wexus/ValidationExpr.h>

namespace wexus
{
  class ActiveClass;

  class ActiveRecord; //fwd

  inline void variantToData(const QVariant &src, QString *outdata)
  {
    if (src.isValid())
      *outdata = src.toString();
    else
      outdata->clear();
  }
  inline void variantToData(const QVariant &src, int *outdata)
  {
    if (src.isValid())
      *outdata = src.toInt();
    else
      *outdata = -1;
  }
}

/**
 * Represents a wexus::ActiveRecord type.
 * Every wexus::ActiveRecord has a link to one of these.
 *
 * @author Aleksander Demko
 */ 
class wexus::ActiveClass
{
  public:
    /**
     * Converts C++ types (int QString) to SQL types.
     *
     * @author Aleksander Demko
     */ 
    static QString toSqlType(const QString &t);

    /**
     * Field "styles".
     *
     * various "styles" of keys. I really want to call
     * these "types", but that would conflict with fieldType()
     * already.
     * @author Aleksander Demko
     */ 
    enum {
      keyStyle = 1,
      varStyle,
      fKeyStyle,
    };

    /**
     * Represents one field in an wexus::ActiveClass
     *
     * @author Aleksander Demko
     */ 
    class ActiveField
    {
      public:
        /// constructor
        ActiveField(int style, const QString &fieldName, const QString &fieldType,
            const ValidationExpr &valexpr, const QVariant &initVal);

        /**
         * Returns the "style" (type) of field this is.
         *
         * @author Aleksander Demko
         */ 
        int style(void) const { return dm_style; }

        /**
         * Returns the field name of this field.
         *
         * @author Aleksander Demko
         */ 
        const QString & fieldName(void) const { return dm_fieldName; }
        /**
         * Returns the C++ data type of this field.
         *
         * @author Aleksander Demko
         */ 
        const QString & fieldType(void) const { return dm_fieldType; }
        /**
         * Returns the SQL datatype (converted from fieldType() via toSqlType())
         * of this field.
         *
         * @author Aleksander Demko
         */ 
        QString sqlFieldType(void) const { return toSqlType(dm_fieldType); }

        /**
         * Returns the wexus::ValidationExpr defined in the .eh
         * for this field, or a null wexus::ValidationExpr is
         * none was defined.
         *
         * @author Aleksander Demko
         */ 
        const ValidationExpr & validationExpr(void) const { return dm_valexpr; }

        /**
         * Returns the default value, as defined in the .eh file.
         * Returns an invalida QVariant if none was set.
         *
         * @author Aleksander Demko
         */ 
        const QVariant & initVal(void) const { return dm_initval; }

        /**
         * Converts the current value of this field in the given
         * wexus::ActiveRecord instance to a QVariant
         * and returns it.
         *
         * @author Aleksander Demko
         */ 
        virtual QVariant toVariant(const ActiveRecord *inst) const = 0;
        
        /**
         * Sets the value of thie field in the given wexus::ActiveRecord
         * instance to v.
         *
         * @author Aleksander Demko
         */ 
        virtual void setVariant(ActiveRecord *inst, const QVariant &v) = 0;

      private:
        int dm_style;
        QString dm_fieldName, dm_fieldType;
        ValidationExpr dm_valexpr;
        QVariant dm_initval;
    };

    template <class RECT, class DATT> class ActiveFieldType : public ActiveField
    {
      public:
        typedef DATT RECT::*MemberPtr;

      public:
        ActiveFieldType(int style, const QString &fieldName, const QString &fieldType,
            const ValidationExpr &valexpr, const QVariant &initVal,
            MemberPtr memberptr)
          : ActiveField(style, fieldName, fieldType, valexpr, initVal), dm_memberptr(memberptr) { }

        virtual QVariant toVariant(const ActiveRecord *inst) const {
          const RECT * recinstance = dynamic_cast<const RECT*>(inst);
          const DATT * datptr;

          assert(recinstance);

          datptr = & (recinstance->*dm_memberptr);

          return QVariant(*datptr);
        }

        virtual void setVariant(ActiveRecord *inst, const QVariant &v) {
          RECT * recinstance = dynamic_cast<RECT*>(inst);
          DATT * datptr;

          assert(recinstance);

          datptr = & (recinstance->*dm_memberptr);

          variantToData(v, datptr);
        }

      private:
        MemberPtr dm_memberptr;
    };

  public:

    /// ctor
    ActiveClass(const QString &_className);

    /**
     * The C++ class name of this type.
     *
     * @author Aleksander Demko
     */ 
    const QString & className(void) const { return dm_classname; }

    /**
     * The C++ class converted to a SQL-comptabile string (mostly :: removed)
     *
     * @author Aleksander Demko
     */ 
    const QString & tableName(void) const { return dm_tablename; }

    /**
     * Returns all the field names as one string, seperated
     * by commas. Each field name will be prefixed by tableName().
     *
     * @author Aleksander Demko
     */ 
    const QString & fieldsAsList(void) const { return dm_fieldsaslist; }
    /**
     * Returns all the field names as one string, seperated
     * by commas.
     *
     * @author Aleksander Demko
     */ 
    const QString & fieldsAsListSansTable(void) const { return dm_fieldsaslistsanstable; }
    /**
     * Returns a list of question marks, each seperated by a comma.
     * There will be as many question marks as their are fields in this
     * class.
     *
     * @author Aleksander Demko
     */ 
    const QString & questionsAsList(void) const { return dm_questionsaslist; }

    /**
     * Returns the index of the key column.
     * There can only be one (for now.. anyways)
     *
     * Never fails.
     *
     * @author Aleksander Demko
     */ 
    int keyColumn(void) const;

    /**
     * Return the ActiveField referenced by keyColumn()
     *
     * @author Aleksander Demko
     */ 
    std::shared_ptr<ActiveField> keyField(void) const { return fieldsVec()[keyColumn()]; }

    /// creates the table in the database
    void createTable(void) const;

  public:
    typedef QVector<std::shared_ptr<ActiveField> > FieldVec;
    typedef QMap<QString, std::shared_ptr<ActiveField> > FieldMap;

    /**
     * Returns all the fields as a vector.
     *
     * @author Aleksander Demko
     */ 
    FieldVec & fieldsVec(void) { return dm_vec; }

    /**
     * Returns all the fields as a vector.
     *
     * @author Aleksander Demko
     */ 
    const FieldVec & fieldsVec(void) const { return dm_vec; }

    /**
     * Returns all the fields as a map
     * keyed by their field names.
     *
     * @author Aleksander Demko
     */ 
    FieldMap & fieldsMap(void) { return dm_map; }

    /**
     * Returns all the fields as a map
     * keyed by their field names.
     *
     * @author Aleksander Demko
     */ 
    const FieldMap & fieldsMap(void) const { return dm_map; }

  protected:
    /// field adder
    template <class RECT, class DATT>
      void addField(int style, const QString &fieldName, const QString &fieldType,
          const ValidationExpr &valexpr,
          const QVariant &initVal,
          typename ActiveFieldType<RECT,DATT>::MemberPtr memberptr) {
        std::shared_ptr<ActiveField> f(new ActiveFieldType<RECT,DATT>(style, fieldName, fieldType, valexpr, initVal, memberptr));

        dm_vec.push_back(f);
        dm_map[fieldName] = f;

        if (style == keyStyle) {
          assert(dm_keycolumn == -1);   // only one primary key field for now
          dm_keycolumn = dm_vec.size()-1;
        }
      }
    /// called by descendants construtors
    void doneConstruction(void);

  private:
    QString dm_classname;
    QString dm_tablename;
    QString dm_fieldsaslist, dm_fieldsaslistsanstable, dm_questionsaslist;

    int dm_keycolumn;

    FieldVec dm_vec;
    FieldMap dm_map;
};

#endif

