
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FILERECORD_H__
#define __INCLUDED_WEXUS_FILERECORD_H__

#include <QString>
#include <QRegExp>
#include <QDirIterator>

#include <wexus/TR1.h>
#include <wexus/IDAble.h>

namespace wexus
{
  class ActiveFile;
}

/**
  * A database-record like representation
  * of a file. Instead of a table, it uses a directory
  * (plus a filtering regexp). Instead of record,
  * it uses an individual file.
  *
  * This concept is still being fleshed out. Future
  * ideas include:
  *
  * a) filename->field like decomposition based on regexp
  * (with custom user-made field lists)
  *
  * b) a query system for a)
  *
  * c) more file load types, like images and xml.
  *
  * d) rename this to ActiveFile?
  *
  * @author Aleksander Demko
  */ 
class wexus::ActiveFile : public wexus::IDAble
{
  public:
    QString id;       /// just the filename, no directory

  public:
    /**
     * Constructor
     *
     * @author Aleksander Demko
     */ 
    ActiveFile(const QString &dirname, const QRegExp &regexp);

    /// implementation
    virtual QVariant getIDAsVariant(void);

    /**
     * Security routine.
     * throws if the filename:
     *  - is empty
     *  - contains bad chars, like / or \
     *  - starts with a .
     *  - doesnt pass the reg ex
     *
     * @author Aleksander Demko
     */ 
    void checkFileName(const QString &filename);

    /**
     * Runs checkFileName on id
     *
     * @author Aleksander Demko
     */
    void checkFileName(void) { checkFileName(id); }

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
     * Demand load the curren file. Throws on errors.
     *
     * @author Aleksander Demko
     */ 
    QByteArray & asByteArray(void);

    /**
     * Returns all the records.
     *
     * @author Aleksander Demko
     */ 
    void all(bool reverseOrder = false);

    /**
     * Returns the next record in file list, or false if
     * done.
     *
     * @author Aleksander Demko
     */ 
    bool next(void);

  protected:
    /**
     * This is called after loading a record.
     * id (the filename) is already set up entering this function.
     * You can also examine the full filename parameter.
     *
     * Decendants should be able to work with empty-string ids
     * and can throw exceptions on errors.
     *
     * TODO in the future, this might be implemented
     * in a more automatic/regex manner using .eh-like files.
     *
     * The default implementation does nothing but return true;
     *
     * @return true on success (false causes this record to not
     * be loaded)
     * @author Aleksander Demko
     */ 
    virtual bool onLoad(const QString &fullfilename);

  protected:
    struct DirSpec
    {
      QString dirname;
      QRegExp regexp;
    };

    std::shared_ptr<DirSpec> dm_dirspec;

    struct DataSpec
    {
      QString filename;
      QByteArray bytearray;
    };
    // the data, if any
    // for multi-types, this will have to be a union-like thing
    std::shared_ptr<DataSpec> dm_data;

    struct IteratorSpec
    {
      QStringList filenames;
      QStringList::const_iterator iterator;
    };

    // current iteration, if any
    std::shared_ptr<IteratorSpec> dm_iterator;
};

#endif

