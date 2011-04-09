
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_DIRMONITOR_H__
#define __INCLUDED_HYDRA_DIRMONITOR_H__

#include <set>
#include <map>
#include <hydra/TR1.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>

namespace hydra
{
  void pruneDirectory(const QString &dirname);

  class FileList;
  class DirChangeEvent;
  class DirMonitor;

  class DirDBUpdater;
}

/**
 * Emited when a directory changes.
 *
 * @author Aleksander Demko
 */ 
class hydra::DirChangeEvent : public QObject
{
  public:
    QStringList addFiles, removeFiles;

    bool isEmpty(void) const { return addFiles.isEmpty() && removeFiles.isEmpty(); }

    void debugPrint(void);
};

/**
 * Monitors one or more directories.
 *
 * @author Aleksander Demko
 */ 
class hydra::DirMonitor : public QObject
{
    Q_OBJECT

  protected:
    class MarkList;//fwd/internal class
  public:
    /// constructor
    DirMonitor(void);

    void addPath(const QString &path);
    void removePath(const QString &path);

  signals:
    void dirChange(const QString &path, const DirChangeEvent &);

  private slots:
    void onDirChanged(const QString &dir);
    void onTimer(void);

  private:
    void reloadDirs(void);
    void reloadDir(MarkList &marklist, DirChangeEvent &outevent);

  private:
    QFileSystemWatcher dm_watcher;

    bool dm_queued;
    bool dm_requeue;

    typedef std::map<QString, std::shared_ptr<MarkList> > marklists_t;
    marklists_t dm_marklists;
};

/**
 * This is a class that can listen to DirChangeEvent events and update
 * the DB (via the singleton Engine class).
 *
 * @author Aleksander Demko
 */ 
class hydra::DirDBUpdater : public QObject
{
    Q_OBJECT

  public:
    DirDBUpdater(void);

  public slots:
    /// connect this to dirChange signal of a DirMonitor
    void dirChanged(const QString &path, const DirChangeEvent &);
};

#endif

