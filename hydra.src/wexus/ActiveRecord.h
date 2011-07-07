
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ACTIVERECORD_H__
#define __INCLUDED_WEXUS_ACTIVERECORD_H__

#include <hydra/TR1.h>

#include <wexus/ActiveExpr.h>
#include <wexus/ActiveClass.h>

namespace wexus
{
  class ActiveRecord;
}

class wexus::ActiveRecord
{
  public:
    class Exception : public std::exception
    {
      public:
        /// constructor
        Exception(const QString &_what) throw();
        virtual ~Exception() throw();

        virtual const char* what() const throw() { return dm_what; }

      private:
        // cant be a QString as then what() will return a * to a temporary
        QByteArray dm_what;
    };

  public:
    // helper functions
    // retutrns the DB, throwing an exception on failure
    static QSqlDatabase & database(void);
    // throws an assert is qy.lastError().isValid()
    static void check(const QSqlQuery &qy);

  public:
    /**
     * Returns the active class for this
     * record.
     * Should never be null.
     *
     * @author Aleksander Demko
     */ 
    std::shared_ptr<ActiveClass> activeClass(void);

  public:
    // stock queryies
    void all(const ActiveExpr & orderByExpre = ActiveExpr());

    // insert the current values into the database
    void insert(void);

    /**
     * After running a query function, this will move the current row
     * to the first row in the result set. Subsequent calls will move
     * the current row to the next one. true is returns if the current
     * row is valid.
     *
     * @author Aleksander Demko
     */ 
    bool next(void);

  protected:
    /// ctor, protected. this class is meanted to be inherited from.
    ActiveRecord(void);

    void setActiveClass(const QString &className, bool &hadToCreate);

    void setQuery(std::shared_ptr<QSqlQuery> qry);

    virtual void initClass(void) = 0;

  private:
    std::shared_ptr<ActiveClass> dm_class;
    std::shared_ptr<QSqlQuery> dm_query;

    typedef QMap<QString, std::shared_ptr<ActiveClass> > ActiveClassMap;

    static ActiveClassMap *dm_manager;
};

#endif

