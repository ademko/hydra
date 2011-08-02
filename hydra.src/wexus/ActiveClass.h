
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ACTIVECLASS_H__
#define __INCLUDED_WEXUS_ACTIVECLASS_H__

#include <assert.h>

#include <hydra/TR1.h>

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

    class ActiveField
    {
      public:
        ActiveField(int style, const QString &fieldName, const QString &fieldType, const ValidationExpr &valexpr);

        int style(void) const { return dm_style; }

        const QString & fieldName(void) const { return dm_fieldName; }
        const QString & fieldType(void) const { return dm_fieldType; }
        QString sqlFieldType(void) const { return toSqlType(dm_fieldType); }

        const ValidationExpr validationExpr(void) const { return dm_valexpr; }

        virtual QVariant toVariant(const ActiveRecord *inst) const = 0;
        
        virtual void setVariant(ActiveRecord *inst, const QVariant &v) = 0;

      private:
        int dm_style;
        QString dm_fieldName, dm_fieldType;
        ValidationExpr dm_valexpr;
    };

    template <class RECT, class DATT> class ActiveFieldType : public ActiveField
    {
      public:
        typedef DATT RECT::*MemberPtr;

      public:
        ActiveFieldType(int style, const QString &fieldName, const QString &fieldType, const ValidationExpr &valexpr, MemberPtr memberptr)
          : ActiveField(style, fieldName, fieldType, valexpr), dm_memberptr(memberptr) { }

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

    const QString & className(void) const { return dm_classname; }

    const QString & tableName(void) const { return dm_tablename; }

    const QString & fieldsAsList(void) const { return dm_fieldsaslist; }
    const QString & fieldsAsListSansTable(void) const { return dm_fieldsaslistsanstable; }
    const QString & questionsAsList(void) const { return dm_questionsaslist; }

    /**
     * Returns the index of the key column.
     * There can only be one (for now.. anyways)
     *
     * @author Aleksander Demko
     */ 
    int keyColumn(void) const;

    /// creates the table in the database
    void createTable(void) const;

  public:
    typedef QVector<std::shared_ptr<ActiveField> > FieldVec;
    typedef QMap<QString, std::shared_ptr<ActiveField> > FieldMap;

    FieldVec & fieldsVec(void) { return dm_vec; }
    const FieldVec & fieldsVec(void) const { return dm_vec; }
    FieldMap & fieldsMap(void) { return dm_map; }
    const FieldMap & fieldsMap(void) const { return dm_map; }

  protected:
    /// field adder
    template <class RECT, class DATT>
      void addField(int style, const QString &fieldName, const QString &fieldType,
          const ValidationExpr &valexpr,
          typename ActiveFieldType<RECT,DATT>::MemberPtr memberptr) {
        std::shared_ptr<ActiveField> f(new ActiveFieldType<RECT,DATT>(style, fieldName, fieldType, valexpr, memberptr));

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

