
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

/**
 * The base class for all Wexus-generated
 * user records. This is a type of ORM.
 *
 * @author Aleksander Demko
 */ 
class wexus::ActiveRecord
{
  public:
    /**
     * General wexus::ActiveRecord exception.
     *
     * @author Aleksander Demko
     */ 
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
    /**
     * A record that was expected to exist does not.
     *
     * @author Aleksander Demko
     */ 
    class RecordNotFound : public Exception
    {
      public:
        /// constructor
        RecordNotFound(void);
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
    ActiveClass * activeClass(void) const { return dm_class; }

  public:
    /**
     * Sets the default ordering.
     * If not called, the default is no ordering
     * at all.
     *
     * @author Aleksander Demko
     */ 
    void order(const ActiveExpr & orderByExpr);

    /**
     * Returns all the records.
     *
     * @author Aleksander Demko
     */ 
    void all(void);

    /**
     * Returns all the records that match the given whereExpr
     * expression.
     *
     * @author Aleksander Demko
     */ 
    void where(const ActiveExpr & whereExpr);

    /**
     * Finds the record that that has the given
     * value as its primary key.
     * Throws an exception on not-found.
     *
     * @author Aleksander Demko
     */
    void find(const QVariant &keyVal);

    /**
     * Finds the loads the first record that matches the
     * given whereExpr expression.
     * This calls next() for, and returns true if that
     * succeeded.
     *
     * @author Aleksander Demko
     */
    bool first(const ActiveExpr & whereExpr = ActiveExpr());

    /**
     * Finds the loads the last record matches the
     * given whereExpr expression.
     * This calls next() for, and returns true if that
     * succeeded.
     *
     * @author Aleksander Demko
     */
    bool last(const ActiveExpr & whereExpr = ActiveExpr());

    /// insert the current values into the database
    void create(void);

    /**
     * Saves the current values back into the database.
     * You must not have changed the primary key value :)
     *
     * The current query is unaffected, so you can continue
     * iteration (with next()), for example.
     *
     * @author Aleksander Demko
     */ 
    void save(void);

    /**
     * Removes the current record from the DB.
     * Does nothing on failure.
     *
     * @author Aleksander Demko
     */ 
    void destroy(void);

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
    ActiveRecord(ActiveClass *klass);
    // virtual dtor required
    virtual ~ActiveRecord();

    void resetQuery(void);
    void setQuery(std::shared_ptr<QSqlQuery> qry);

    // core query function
    void internalWhere(const ActiveExpr & whereExpr, int limit);

  private:
    ActiveClass *dm_class;
    ActiveExpr dm_orderByExpr;
    std::shared_ptr<QSqlQuery> dm_query;
};

#endif

