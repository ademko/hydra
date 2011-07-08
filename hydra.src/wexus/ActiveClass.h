
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

namespace wexus
{
  class ActiveClass;

  class ActiveRecord; //fwd

  inline void variantToData(const QVariant &src, QString *outdata) { *outdata = src.toString(); }
  inline void variantToData(const QVariant &src, int *outdata) { *outdata = src.toInt(); }
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

    class ActiveField
    {
      public:
        ActiveField(const QString &fieldName, const QString &fieldType);

        const QString & fieldName(void) const { return dm_fieldName; }
        const QString & fieldType(void) const { return dm_fieldType; }
        QString sqlFieldType(void) const { return toSqlType(dm_fieldType); }

        virtual QVariant toVariant(ActiveRecord *inst) const = 0;
        
        virtual void setVariant(ActiveRecord *inst, const QVariant &v) = 0;

      private:
        QString dm_fieldName, dm_fieldType;
    };

    template <class RECT, class DATT> class ActiveFieldType : public ActiveField
    {
      public:
        typedef DATT RECT::*MemberPtr;

      public:
        ActiveFieldType(const QString &fieldName, const QString &fieldType, MemberPtr memberptr)
          : ActiveField(fieldName, fieldType), dm_memberptr(memberptr) { }

        virtual QVariant toVariant(ActiveRecord *inst) const {
          RECT * recinstance = dynamic_cast<RECT*>(inst);
          DATT * datptr;

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

    /// creates the table in the database
    void createTable(void);

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
      void addField(const QString &fieldName, const QString &fieldType,
          typename ActiveFieldType<RECT,DATT>::MemberPtr memberptr) {
        std::shared_ptr<ActiveField> f(new ActiveFieldType<RECT,DATT>(fieldName, fieldType, memberptr));

        dm_vec.push_back(f);
        dm_map[fieldName] = f;
      }
    /// called by descendants construtors
    void doneConstruction(void);

  private:
    QString dm_classname;
    QString dm_tablename;
    QString dm_fieldsaslist, dm_fieldsaslistsanstable, dm_questionsaslist;

    FieldVec dm_vec;
    FieldMap dm_map;
};

#endif

