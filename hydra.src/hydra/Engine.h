
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_ENGINE_H__
#define __INCLUDED_HYDRA_ENGINE_H__

#include <hydra/TR1.h>

#include <QString>
#include <QDateTime>

#define HYDRA_VERSION_STRING "0.22"
#define HYDRA_COPYRIGHT_STRING "2007-2014"

/**
 * The core hydra namespace contains all the classes
 * of the core hydra library.
 *
 * @author Aleksander Demko
 */ 
namespace hydra
{
  class Engine;
  class DB;//forward
  class FilePathRecord; //forward
  class FileHashRecord; //forward
  class FileItemRecord; //forward
}

/**
 * The main processing engine for hydra applications.
 * It is a singleton class that should be instatiated
 * by your main or similar function.
 *
 * @author Aleksander Demko
 */
class hydra::Engine
{
  public:
    // carious codes
    enum {
      Add_New = 10,
      Add_Exists,
      Add_NewPath,
      Add_UpdatedPath,
      Add_Error,

      Load_OK = 100,        // typically, you should just use Load_OK
      Load_ErrorNeedsUpdate,    // its in the DB, but it needs an update
      Load_ErrorNotFound, // not in db
      Load_ErrorFatal,    // some serious error
      Load_ErrorFileMissing,  // file missing or access error
    };
  public:
    /// constructor
    Engine(void);
    /// destructor
    ~Engine();

    /**
     * Returns the static instance, if any
     *
     * @author Aleksander Demko
     */
    static Engine * instance(void) { return dm_instance; }

    /**
     * Gets the path to the user's home directory, or "."
     * if not found.
     *
     * @author Aleksander Demko
     */ 
    static QString homeDir(void);

    /**
     * Returns the directory where the database files are stored.
     * (This is usually ~/.hydradb)
     *
     * This will also create it, if necesary.
     *
     * @author Aleksander Demko
     */ 
    static QString dbDir(void);

    /**
     * Adds a file to the db.
     * This will also repair/update any outof date records.
     * If precalchash is provided, then it is assumed to be the hash of the given
     * file.
     *
     * @author Aleksander Demko
     */ 
    int addFile(const QString &fullfilename, const QString *precalchash = 0);

    /**
     * Erase the given path from the database.
     * Returns true on success (which for now, is always)
     *
     * @author Aleksander Demko
     */ 
    bool erasePath(const QString &fullfilename);

    /**
     * Erase the given hash from the database.
     * Note, that this will UNTIE the hash from its current item record,
     * effectivly forgetting its tags, etc. Upon rediscovery, the hash
     * will get a new item record.
     * Returns true on success (which for now, is always)
     *
     * @author Aleksander Demko
     */ 
    bool eraseHash(const QString &hash);

    /**
     * Loads the given file from the DB
     * Any of the pointers may be null.
     *
     * @return a Load_* error code
     * @author Aleksander Demko
     */ 
    int getFileItem(const QString &fullfilename, hydra::FileItemRecord *item, hydra::FileHashRecord *hash,
        hydra::FilePathRecord *path);
    /**
     * Loads the given file from the DB
     *
     * This is a shorter version of the above.
     *
     * @return a Load_* error code
     * @author Aleksander Demko
     */ 
    int getFileItem(const QString &fullfilename, hydra::FileItemRecord &rec);

    /**
     * Loads the given file from the DB, by hash
     *
     * @return a Load_* error code
     * @author Aleksander Demko
     */ 
    int getFileItemByHash(const QString &hashkey, hydra::FileItemRecord *item, hydra::FileHashRecord *hash);

    /**
     * Reloads the given file item from db.
     * The fileitem must have previously been loaded from the db
     * (ie. its id field must be valid).
     *
     * Returns the same codes as getFileItem()
     *
     * @author Aleksander Demko
     */ 
    int regetFileItem(hydra::FileItemRecord &rec);

    /**
     * Saves the given item to the db.
     *
     *
     * @param rec the record to save
     * @param newmodtime the modification time to use. If this isValid, rec.modtime will be set to this before saving
     * @return true on success
     * @author Aleksander Demko
     */
    bool saveFileItem(hydra::FileItemRecord &rec, const QDateTime &newmodtime);

    // future, uses the internal cache bank
    //std::tr1::shared_ptr<FileItemRecord> getFileItem(const QString &fullfilename);

    hydra::DB & filePathDB(void) { return *dm_filepathdb; }
    hydra::DB & fileHashDB(void) { return *dm_filehashdb; }
    hydra::DB & fileItemDB(void) { return *dm_fileitemdb; }

    /**
     * This converts any of the errors codes into a ascii char
     * that may be suitable for status output.
     *
     * @author Aleksander Demko
     */ 
    static char codeToChar(int code);

    /// gets the mod time and file size of the given file
    /// returns true on success
    static bool statFile(const QString &fullfilename, double &modtime, qint64 &filesize);

  private:
    static Engine *dm_instance;

    std::shared_ptr<DB> dm_filepathdb, dm_filehashdb, dm_fileitemdb;
};

#endif

