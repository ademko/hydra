
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FILEAPP_H__
#define __INCLUDED_WEXUS_FILEAPP_H__

#include <QVector>

#include <wexus/Application.h>

namespace wexus
{
  class FileApp;
}

/**
 * An application that just servers files from one
 * or more directories.
 *
 * INI parameters:
 *   dirX = "."
 *   dirX = "dir"
 *   dirX = "dir/subdir"
 *    X is a number, 1, 2, etc
 *   The directory to serve. If none are given then a dir1="." is assumed.
 *   All directotories will be relative to the app dir.
 *   for any dirX, you can have a optionX. this string can contain a space
 *   delimeted list any of the following (case insenstive):
 *   optionsX = "IndexHtml AutoDirIndex AllowAllMimeTypes"
 *   TODO future allow full absolute dirs?
 *
 *
 * @author Aleksander Demko
 */ 
class wexus::FileApp : public wexus::Application
{
  public:
    /// normal ctor
    FileApp(void);

    /**
     * Specific ctor called by the mainline.
     *
     * Do not call init if you use this ctor.
     *
     * docdir will have some flags set, like auto indexing and loading all files. careful.
     * TODO future, add flags parameter
     * @author Aleksander Demko
     */ 
    FileApp(const QString &docdir);

    virtual void init(const QVariantMap &settings);

    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    struct DirFlags
    {
      QString dirname;
      int flags;

      DirFlags(void) : flags(0) { }
      DirFlags(const QString &_dirname, int _flags = 0) : dirname(_dirname), flags(_flags) { }
    };

    QVector<DirFlags> dm_dirs;   // vector, because once its built, it wont change
};

#endif

