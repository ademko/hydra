
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_FILEITERATOR_H__
#define __INCLUDED_HYDRA_FILEITERATOR_H__

#include <list>

#include <QDirIterator>
#include <QString>

#include <hydra/TR1.h>

namespace hydra {
/**
 * Return the given file as a unique, absolute filename.
 *
 * @author Aleksander Demko
 */
QString makeAbsolute(const QString &s);

/**
 * Returns just the filename and extension.
 *
 * @author Aleksander Demko
 */
QString justName(const QString &s);

/**
 * Is this a normal file.
 * A normal file doesnt begin with _ . or ,
 *
 * @author Aleksander Demko
 */
bool isNormalFile(const QString &justname);

/**
 * Returns true if the given directory is "normal".
 * A normal file is one that isn't hidden.
 * A hidden file starts with a .
 *
 * @author Aleksander Demko
 */
bool isNormalDirectory(const QString &justdirname);

/**
 * Is this an image file.
 *
 * @author Aleksander Demko
 */
bool isImageFile(const QString &justname);

/**
 * Creates a directory.
 *
 * @author Aleksander Demko
 */
bool mkDir(const QString &name);

class FileIterator;
} // namespace hydra

/**
 * This iterateos over a directory and all its subdirectories
 * enumeratoring all the files.
 *
 * @author Aleksander Demko
 */
class hydra::FileIterator {
  public:
    /**
     * Construtor. The initial dir (which can also just be a file)
     * is supplied.
     *
     * Unless allfiles is true, then hidden directories (.*) will
     * be skippeded.
     *
     * If recurse is true, then all subdirectories will be examined too.
     *
     * @author Aleksander Demko
     */
    FileIterator(const QString &fileOrDir, bool allfiles = false,
                 bool recurse = true);

    /**
     * Is this iterator traversing a directory set? If false, then the
     * iterator was only fed one file.
     *
     * @author Aleksander Demko
     */
    bool isTraversing(void) const { return dm_istraversing; }

    /**
     * Is there another file in the iteration?
     *
     * @author Aleksander Demko
     */
    bool hasNext(void) const { return !dm_current.isEmpty(); }

    /**
     * Returns the next file in the iteration.
     * Note that all the files will be returned, so you will
     * need to filter further if you only want files of a certain
     * type, etc.
     *
     * @return the next filename, which will always be a full, absolute filename
     *
     * @author Aleksander Demko
     */
    QString next(void);

  private:
    void loadNextFile(void);

  private:
    QString dm_current;
    bool dm_allfiles;
    bool dm_istraversing;

    std::unique_ptr<QDirIterator> dm_iterator;
};

#endif
