
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
 *   TODO future allow full absolute dirs?
 *
 *
 * @author Aleksander Demko
 */ 
class wexus::FileApp : public wexus::Application
{
  public:
    FileApp(void);

    virtual void init(const QVariantMap &settings);

    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    QVector<QString> dm_dirs;   // vector, because once its built, it wont change
};

#endif

