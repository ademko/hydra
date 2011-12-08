
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_DB_H__
#define __INCLUDED_HYDRA_DB_H__

#include <hydra/TR1.h>

#include <QString>

#include <hydra/Record.h>

struct sqlite3;   // forward
typedef struct sqlite3 sqlite3;   // forward
struct sqlite3_stmt;  // forward
typedef struct sqlite3_stmt sqlite3_stmt; // forward

namespace hydra
{
  class DB;
  class Cursor;
};

/**
 * This represents a sqlite database table.
 *
 * This in the future, this can be abtracted away as an interface to allow
 * many possible backends (the old BerkleyDB one, postgresql, etc)
 *
 * Internally, this backend uses std::string and 8-bit UTF strings
 * for size-conservation.
 *
 * @author Aleksander Demko
 */ 
class hydra::DB
{
  public:
    /**
     * Creates a DB (disk backed map). It will use the given sqllite db
     * file and prepend the given prefix/namespace to the tables.
     * Using different tablescope's allows you to have multiple DBs
     * in the same sqlite file.
     *
     * @param filename the sqlite filename on disk
     * @param tablescope the prefix/table name to use. Cannot be empty.
     * @author Aleksander Demko
     */ 
    DB(const QString &filename, const QString & tablescope);
    /**
     * Creates a DB (disk backed map). It will use the given sqllite db
     * file and prepend the given prefix/namespace to the tables.
     *
     * This constructor allows you to reuse anoter DB instance's sqlite connection.
     *
     * @param linked_db the DB from whome this DB will share a single sqlite connection.
     * @param tablescope the prefix/table name to use. Cannot be empty.
     * @author Aleksander Demko
     */ 
    DB(DB &linked_db, const QString &tablescope);
    /// destructor
    virtual ~DB();

    /// is this key even in the db?
    bool contains(const std::string &key) { return contains(key.c_str()); }
    /// is this key even in the db?
    bool contains(const QString &key) { return contains(key.toUtf8().constData()); }
    /// is this key even in the db?
    bool contains(const char *key);

    /// removes the key from the db, returns true if anything was done
    bool erase(const std::string &key) { return erase(key.c_str()); }
    /// removes the key from the db, returns true if anything was done
    bool erase(const QString &key) { return erase(key.toUtf8().constData()); }
    /// removes the key from the db, returns true if anything was done
    bool erase(const char *key);

    /// saves to the db, returns true on success, will replace if exists
    bool insert(const std::string &key, const std::string &value) { return insert(key.c_str(), value); }
    /// saves to the db, returns true on success, will replace if exists
    bool insert(const QString &key, const std::string &value) { return insert(key.toUtf8().constData(), value); }
    /// saves to the db, returns true on success, will replace if exists
    bool insert(const char *key, const std::string &value);

    /// returns false on not-found or failed to load
    bool get(const std::string &key, std::string &value) { return get(key.c_str(), value); }
    /// returns false on not-found or failed to load
    bool get(const QString &key, std::string &value) { return get(key.toUtf8().constData(), value); }
    /// returns false on not-found or failed to load
    bool get(const char *key, std::string &value);

    /// saves to the db, returns true on success, will replace if exists
    bool insert(const std::string &key, const hydra::Record &value) { return insert(key.c_str(), value); }
    /// saves to the db, returns true on success, will replace if exists
    bool insert(const QString &key, const hydra::Record &value) { return insert(key.toUtf8().constData(), value); }
    /// saves to the db, returns true on success, will replace if exists
    bool insert(const char *key, const hydra::Record &value);

    /// returns false on not-found or failed to load
    bool get(const std::string &key, hydra::Record &value) { return get(key.c_str(), value); }
    /// returns false on not-found or failed to load
    bool get(const QString &key, hydra::Record &value) { return get(key.toUtf8().constData(), value); }
    /// returns false on not-found or failed to load
    bool get(const char *key, hydra::Record &value);

  private:
    void initDB(void);
    void initPrepared(void);

  private:
    class Connection
    {
      public:
        QString fileName;
        sqlite3 *handle;
      public:
        Connection(void) : handle(0) { }
        ~Connection();
    };
    class Statement
    {
      public:
        sqlite3_stmt *handle;
      public:
        Statement(void) : handle(0) { }
        ~Statement();
    };
    class ResetOnExit
    {
      public:
        ResetOnExit(sqlite3_stmt *handle) : dm_handle(handle) { }
        ~ResetOnExit();
      private:
        sqlite3_stmt *dm_handle;
    };

  private:
    friend class hydra::Cursor;

    std::shared_ptr<Connection> dm_connection;
    QString dm_table;

    // these are for performance cacheing
    Statement dm_put_statement, dm_has_statement, dm_get_statement, dm_erase_statement;

    // hot buffer used by the record-based put function
    QByteArray dm_put_hotbuf, dm_get_hotbuf;
};

/**
 * A iterator though a DB.
 *
 * @author Aleksander Demko
 */
class hydra::Cursor
{
  public:
    /**
     * Constructor.
     * This will iterate though all the entries in the DB.
     *
     * The cursor will initially be inValid. You must use next()
     * to move to the first valid row (if any).
     *
     * @author Aleksander Demko
     */
    Cursor(hydra::DB &db);
    /**
     * Constructor.
     * This will iterate though all the entries in the DB whos key has
     * the given prefix
     *
     * The cursor will initially be inValid. You must use next()
     * to move to the first valid row (if any).
     *
     * @author Aleksander Demko
     */
    Cursor(hydra::DB &db, const QString &prefix);

    /**
     * Is the cursot at a valid row?
     *
     * @author Aleksander Demko
     */ 
    bool isValid(void) const { return dm_isvalid; }

    /**
     * Move the cursor to the next row, returns true if this was successul
     * and the cursor is at the next row and is valid.
     *
     * @author Aleksander Demko
     */ 
    bool next(void);

    /**
     * Returns the key value at the current row. The current row must be valid.
     *
     * @author Aleksander Demko
     */ 
    const char * getKey(void);

    /**
     * Loads the current value. The current row must be valid.
     *
     * @author Aleksander Demko
     */ 
    void get(std::string &value);

    /**
     * Loads the current value. The current row must be valid.
     *
     * @author Aleksander Demko
     */ 
    void get(QString &value);

    /**
     * Loads the current value. The current row must be valid.
     *
     * Returns false if the record failed to deserialize (ie. it threw an exception)
     *
     * @author Aleksander Demko
     */ 
    bool get(hydra::Record &value);

    //Erase(void);

  private:
    bool dm_isvalid;

    std::shared_ptr<DB::Connection> dm_connection;

    DB::Statement dm_select_statement;

    QByteArray dm_put_hotbuf, dm_get_hotbuf, dm_prefix_string;
};

#endif

