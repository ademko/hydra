
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ACTIVERECORD_H__
#define __INCLUDED_WEXUS_ACTIVERECORD_H__

#include <hydra/TR1.h>

#include <QString>
#include <QMap>
#include <QVector>

namespace wexus
{
  class ActiveClass;

  class ActiveRecord;
}

class wexus::ActiveClass
{
  public:

    class ActiveField
    {
      public:
        ActiveField(const QString &fieldName, const QString &fieldType);

        const QString & fieldName(void) const { return dm_fieldName; }
        const QString & fieldType(void) const { return dm_fieldType; }

      private:
        QString dm_fieldName, dm_fieldType;
    };

  public:

    ActiveClass(const QString &_className);
    void addField(const QString &fieldName, const QString &fieldType);

  private:
    QString dm_classname;

    typedef QVector<std::shared_ptr<ActiveField> > FieldVec;
    typedef QMap<QString, std::shared_ptr<ActiveField> > FieldMap;

    FieldVec dm_vec;
    FieldMap dm_map;
};

class wexus::ActiveRecord
{
  public:
    /**
     * Returns the active class for this
     * record.
     * Should never be null.
     *
     * @author Aleksander Demko
     */ 
    std::shared_ptr<ActiveClass> activeClass(void);

  protected:
    /// ctor, protected. this class is meanted to be inherited from.
    ActiveRecord(void);

    void setActiveClass(const QString &className, bool &hadToCreate);

    virtual void initClass(void) = 0;

  private:
    std::shared_ptr<ActiveClass> dm_class;

    typedef QMap<QString, std::shared_ptr<ActiveClass> > ActiveClassMap;

    static ActiveClassMap *dm_manager;
};

#endif

