
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FILEENTRY_H__
#define __INCLUDED_HYDRADESKTOP_FILEENTRY_H__

#include <QString>
#include <QDateTime>

#include <hydra/Records.h>
#include <desktop/FileItemCache.h>

namespace desktop
{
  class FileEntry;
}

/**
 * An individual FileEntry
 * This contains a filename, record info (demand loaded) and file info.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileEntry
{
  public:
    FileEntry(const QString &_fullfilename);

    bool operator < (const FileEntry &rhs) const { return dm_fullfilename < rhs.dm_fullfilename; }

    const QString & fullfilename(void) const { return dm_fullfilename; }
    const QString & justname(void) const { return dm_justname; }

    // have any meta data (hash/item)
    // may trigger a load
    bool hasRecord(bool *needsBigRead = 0) const;

    // may trigger a load
    const QString & recordHash(void) const;
    // may trigger a load
    hydra::FileItemRecord & recordItem(void) const;

    /// the last modified time of the disk file
    QDateTime fileLastModified(void) const;
    /// file's size on disk
    qint64 fileSize(void) const;

    // future image* info stuff?

    /// gets the rotate code, might have to load exif or tag info to determine this
    /// always returns >= 0
    int rotateCode(void);
    /// same as rotateCode, but always does a reload from tags/exif
    int reloadRotateCode(void);

  private:
    void loadRecord(bool *needsBigRead = 0) const;
    void loadFile(void) const;

  private:
    mutable bool dm_recordloaded;

    QString dm_fullfilename, dm_justname;
    mutable QString dm_hash;

    //mutable hydra::FileItemCache::cache_ptr dm_item;  // demand loaded, may be null
    mutable hydra::FileItemRecord dm_item;  // demand loaded, may be null

    mutable QDateTime dm_fileLastModified;
    mutable qint64 dm_fileSize;

    int dm_rotateCode; // -1 initially, which means not loaded
};

#endif

