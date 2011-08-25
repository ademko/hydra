
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FILERECORD_H__
#define __INCLUDED_WEXUS_FILERECORD_H__

#include <wexus/TR1.h>

#include <QString>
#include <QRegExp>
#include <QDirIterator>

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
class wexus::ActiveFile
{
  public:
    QString filename;       /// just the filename, no directory

  public:
    /**
     * Constructor
     *
     * @author Aleksander Demko
     */ 
    ActiveFile(const QString &dirname, const QRegExp &regexp);

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
    void all(void);

    /**
     * Returns the next record in file list, or false if
     * done.
     *
     * @author Aleksander Demko
     */ 
    bool next(void);

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

