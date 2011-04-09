
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_RECORDS_H__
#define __INCLUDED_HYDRA_RECORDS_H__

#include <set>

#include <QUuid>
#include <QDateTime>

#include <hydra/DB.h>

namespace hydra
{
  inline quint64 dateTimeAsInt64(const QDateTime &dt) {
    // in the future, make this 64-bit compatible
    if (dt.isValid())
      return dt.toTime_t();
    else
      return 0;
  }

  inline void int64ToDateTime(quint64 i, QDateTime &out) {
    // in the future, make this 64-bit compatible
    if (i == 0)
      out = QDateTime();
    else
      out.setTime_t(static_cast<uint>(i));
  }

  class FilePathRecord;
  class FileHashRecord;

  class Tags;

  class FileItemRecord;
}

/**
 * Keys file paths to ids.
 *
 * key=full_path_name
 * dbfile=cache.sqlite table=filepaths
 *
 * @author Aleksander Demko
 */ 
class hydra::FilePathRecord : public hydra::Record
{
  public:
    QString hash;      // the hash this filepath last hased too
    double file_time;   // for change checking
    qint64 file_size;    // for change checking
  public:
    FilePathRecord(void);
    virtual void save(QDataStream &out) const;
    virtual void load(QDataStream &in);
};

/**
 * Keys hashes to item ids.
 *
 * key=hash_string
 * dbfile=main.sqlite table=filehashes
 *
 * @author Aleksander Demko
 */ 
class hydra::FileHashRecord : public hydra::Record
{
  public:
    QUuid id;    // the id of the fileitem this hash is refering too
  public:
    FileHashRecord(void);
    virtual void save(QDataStream &out) const;
    virtual void load(QDataStream &in);
};

/**
 * A std::set derived class that provides additional
 * useful functions for dealing with tag sets.
 *
 * @author Aleksander Demko
 */ 
class hydra::Tags : public std::set<QString>
{
  public:
    const static char SEP_CHAR = '=';

    typedef std::set<QString> parent_t;
    typedef std::set<QString> set_t;
  public:
    Tags(void);

    /// debugPrint function
    void debugPrint(void) const;

    /**
     * Sets the given tag, while maintaining some consistancy (for example, = key values).
     * This is prefered over direct write access to the std::set class. In the future,
     * direct access may be eliminated (or the whole thing replaced with a map?)
     *
     * If imageChanged is non-null AND this operation changes the image (rotate tags),
     * then *imageChanged will be set to true. It will be untouched otherwise.
     *
     * @param tag the tag to insert/set.
     * @param imageChanged was the image "changed"
     * @return true if the tags where modified (and probably need to be commited)
     * @author Aleksander Demko
     */ 
    bool insertTag(const QString &newtag, bool *imageChanged=0);

    /**
     * Erases a tag. See insertTag for API info.
     *
     * @author Aleksander Demko
     */ 
    bool eraseTag(const QString &newtag, bool *imageChanged=0);

    /**
     * Erases all the tags. See insertTag for API info.
     *
     * @author Aleksander Demko
     */ 
    bool clearTags(bool *imageChanged=0);

    /**
     * Check if this tag set has the given key.
     * Key-values look like: key=value
     * Note that if there is no =, then it is not a valid key-value. Therefore basic tags will return false for this test.
     *
     * TODO in the future, switch the hole Tags concept to a std::map, which would
     * be optimized for empty-values (as 99% of the tags wont have any values)... or not
     *
     * @author Aleksander Demko
     */ 
    bool hasKey(const QString &key) const;

    /**
     * Sets the current value for the given key.
     * Returns true if anything was done.
     *
     * @author Aleksander Demko
     */ 
    bool insertKey(const QString &key, const QString &value, bool *imageChanged=0);

    /**
     * Erase the given key, no matter what's its value.
     * Returns true if anything was erased, false otherwise.
     *
     * @author Aleksander Demko
     */
    bool eraseKey(const QString &key, bool *imageChanged=0);

    /**
     * Return the given value for the given key.
     * Returns "" if this key doesnt exist.
     * Key-values look like: key=value
     * Note that if there is no =, then it is not a valid key-value. Therefore basic tags will return false for this test.
     *
     * TODO in the future, switch the hole Tags concept to a std::map, which would
     * be optimized for empty-values (as 99% of the tags wont have any values)... or not
     *
     * @author Aleksander Demko
     */ 
    QString keyValue(const QString &key) const;

    /**
     * Returns the index of the keyvalue seperator, or -1 for not found.
     *
     * @author Aleksander Demko
     */ 
    static inline int indexOfKeyValue(const QString &s) { return s.indexOf(SEP_CHAR); }
};

/**
 * The main file records, keyed by their UUIDs.
 *
 * key=UUID
 * db=main.sqlite table=fileitems
 *
 * @author Aleksander Demko
 */ 
class hydra::FileItemRecord : public hydra::Record
{
  public:
    typedef hydra::Tags tags_t;

    QUuid id;    // my own id, duplicated from the key, to ease processing (technically redundant)

    QString title, desc;

    qint32 filetype; // 0 for image, 1 for "other" ... for now...

    tags_t tags;      // although you can have 0 tags, tag strings themselve cannot be empty (0-length)

    /**
     * The modificationTime of this record
     * new records are created with invalid QDateTime
     * only after the user deliberatly updates the record
     * does the record get a real modtime.
     *
     * (move this to the DB layer eventually? - probably not, as this is very specific)
     * @author Aleksander Demko
     */ 
    QDateTime modtime;

    // comments, permissions, etc

  public:
    FileItemRecord(void);
    virtual void save(QDataStream &out) const;
    virtual void load(QDataStream &in);

    quint64 modtimeAsInt64(void) const {
      return dateTimeAsInt64(modtime);
    }
    void setModtimeFromInt64(quint64 i) {
      int64ToDateTime(i, modtime);
    }
};

#endif

