
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FILEOP_H__
#define __INCLUDED_HYDRADESKTOP_FILEOP_H__

#include <QWidget>

#include <hydra/FileOp.h>

namespace desktop
{
  class FileOp;

  class FileOpGuiTest;
  class FileOpDisk;
  class FileOpDB;
  //class FileOpEvents;

  int guiFileOpFiles(QWidget *parent, int op, const QStringList &srcfiles, 
      const QString &destDir, FileOp *handler = 0);
};

/**
 * This provides a common interface to file operations.
 * Subclasses can do various things, providing a nice consistance
 * interface for callers to use.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileOp
{
  public:
    enum {
      Op_Del = 1,
      Op_Move,
      Op_Copy,
    };

  public:
    virtual ~FileOp();

    // returns true on success
    virtual bool delFile(const QString &srcfile) = 0;
    // returns true on success
    virtual bool moveFile(const QString &srcfile, const QString &destfile) = 0;
    // returns true on success
    virtual bool copyFile(const QString &srcfile, const QString &destfile) = 0;
    // returns true on success
    virtual bool delDir(const QString &dirname) = 0;

    /**
     * Perform one file operation, based on the op field, which is one
     * of Op_Del, Op_Move or Op_Copy
     *
     * @author Aleksander Demko
     */ 
    bool opFile(int op, const QString &srcfile, const QString &destfile);
};

/**
 * This class 
 * @author Aleksander Demko
 */ 
class desktop::FileOpGuiTest : public desktop::FileOp
{
  public:
    // this may be thrown by any of the following functions
    class UserCanceled
    {
    };

  public:
    FileOpGuiTest(QWidget *parent);

    // returns true on success (ie. continue to perform this operation)
    virtual bool delFile(const QString &srcfile);
    // returns true on success (ie. continue to perform this operation)
    virtual bool moveFile(const QString &srcfile, const QString &destfile);
    // returns true on success (ie. continue to perform this operation)
    virtual bool copyFile(const QString &srcfile, const QString &destfile);
    // returns true on success
    virtual bool delDir(const QString &dirname);

  private:
    QWidget *dm_parent;
    bool dm_overwrite_all;
    bool dm_overwrite_none;
    bool dm_skip_errors;
};

/**
 * Performs the actual operations on the file system (disk)
 *
 * @author Aleksander Demko
 */ 
class desktop::FileOpDisk : public desktop::FileOp
{
  public:
    FileOpDisk(void);

    // returns true on success
    virtual bool delFile(const QString &srcfile);
    // returns true on success
    virtual bool moveFile(const QString &srcfile, const QString &destfile);
    // returns true on success
    virtual bool copyFile(const QString &srcfile, const QString &destfile);
    // returns true on success
    virtual bool delDir(const QString &dirname);
};

class desktop::FileOpDB : public desktop::FileOp
{
  public:
    FileOpDB(void);

    // returns true on success
    virtual bool delFile(const QString &srcfile);
    // returns true on success
    virtual bool moveFile(const QString &srcfile, const QString &destfile);
    // returns true on success
    virtual bool copyFile(const QString &srcfile, const QString &destfile);
    // returns true on success
    virtual bool delDir(const QString &dirname);
};

#endif

