
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ACTIVERECORD_H__
#define __INCLUDED_WEXUS_ACTIVERECORD_H__

#include <wexus/TR1.h>

#include <wexus/ActiveExpr.h>
#include <wexus/ActiveClass.h>
#include <wexus/ValidationExpr.h>
#include <wexus/Context.h>
#include <wexus/IDAble.h>

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
class wexus::ActiveRecord : public wexus::IDAble
{
  public:
    // helper functions
    // retutrns the DB, throwing an exception on failure
    static QSqlDatabase & database(void);
    // throws an exception is qy.lastError().isValid()
    static void check(const QSqlQuery &qy);
    // throws an exception on false
    static void check(bool b, const QString &exceptionMsg = "ActiveRecord::check(bool) failed");

  public:
    /// implementation
    virtual QVariant getIDAsVariant(void);

    /**
     * Returns the active class for this
     * record.
     * Should never be null.
     *
     * @author Aleksander Demko
     */ 
    ActiveClass * activeClass(void) const { return dm_class; }

    /**
     * Resets the fields to default values... usually -1 for ints
     * and empty strings.
     *
     * Skips the filter column, if set.
     *
     * @author Aleksander Demko
     */ 
    void clear(void);

    /**
     * Validates the fields in the current record
     * against any defined validation checks.
     *
     * outerrors should be Context::errors.
     *
     * @author Aleksander Demko
     */
    void test(Context::Errors *outerrors = 0) const;

    /**
     * Calls clear() first.
     *
     * Extracts the fields from the given form map and calls
     * test().
     *
     * If is v is invalid (the default), then
     * params[activeClass()->table()] will be assumed
     * (this works with the Form(ActiveRecord&) class.
     *
     * Returns true if atleast one field was extracted
     * and errors is empty (after calling test())
     *
     * @author Aleksander Demko
     */ 
    bool fromForm(const QVariant &v = QVariant());

    /**
     * Returns a printable string reprensentation
     * of the fields in this record.
     *
     * @author Aleksander Demko
     */ 
    QString toString(void) const;

    /**
     * Sets the filter column to use in all where clauses.
     * By default this is -1, for none.
     *
     * @author Aleksander Demko
     */ 
    void setFilterColumn(int colindex);
    
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
     * Same as find(), except returns false on failure.
     * find() raises an exception.
     *
     * @author Aleksander Demko
     */
    bool exists(const QVariant &keyVal);

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

    /**
     * Inserts the current values into the database.
     * If a specific id is desired, it must be passed here.
     * Otherwise, one will be auto generated.
     *
     * @author Aleksander Demko
     */ 
    void create(const QVariant &v = QVariant());

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
     * Removes the record with the given primary key from the DB.
     * Does nothing on failure.
     *
     * @author Aleksander Demko
     */ 
    void destroy(const QVariant &keyVal);

    /**
     * Deletes all the records that match the given query.
     * A null query (the default), deletes all the rows.
     * Does nothing on failure.
     *
     * @author Aleksander Demko
     */ 
    void destroyAll(const ActiveExpr & whereExpr = ActiveExpr());

    /**
     * Returns the number of rows that match the given where query,
     * or all the rows if no query is upplied.
     *
     * @author Aleksander Demko
     */ 
    int count(const ActiveExpr & whereExpr = ActiveExpr());

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

    /**
      * Appends the column filter criteria (if any)
      * to the given filter and returns that.
      *
      * @author Aleksander Demko
      */ 
    ActiveExpr filterExpr(const ActiveExpr &e);

    void resetQuery(void);
    void setQuery(std::shared_ptr<QSqlQuery> qry);

    // core query function
    void internalWhere(const ActiveExpr & whereExpr, int limit);

  private:
    ActiveClass *dm_class;
    int dm_filtercol; // the filter column, or -1 if none is set
    ActiveExpr dm_orderByExpr;
    std::shared_ptr<QSqlQuery> dm_query;
};

#endif

