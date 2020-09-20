
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/DirMonitor.h>

#include <assert.h>

#include <QDebug>
#include <QTimer>

#include <hydra/DB.h>
#include <hydra/Engine.h>
#include <hydra/FileIterator.h>

using namespace hydra;

void hydra::pruneDirectory(const QString &dirname) {
    QString prefix(makeAbsolute(dirname) + "/");
    int prefix_len = prefix.size();
    // qDebug() << "Pruning " << prefix;

    Cursor C(Engine::instance()->filePathDB(), prefix);
    // Cursor C(Engine::instance()->filePathDB());
    QStringList eraselist;

    while (C.next()) {
        QString curkey(C.getKey());

        // this will SKIP all the sub-directories, as they will show up in the
        // prefix match (this is a hack until we/maybe we implement a
        // directory-centric filedirpath system DB
        if (curkey.lastIndexOf('/') >= prefix_len)
            continue;
        // qDebug() << "key=  " << curkey << curkey.lastIndexOf("/") <<
        // prefix_len;

        if (!QFileInfo(curkey).exists())
            eraselist.push_back(curkey);
    }

    for (QStringList::const_iterator ii = eraselist.begin();
         ii != eraselist.end(); ++ii) {
        qDebug() << "PRUNE" << *ii;
        Engine::instance()->filePathDB().erase(*ii);
    }
}

//
//
// DirChangeEvent
//
//

void DirChangeEvent::debugPrint(void) {
    qDebug() << "addFiles:";
    for (QStringList::const_iterator ii = addFiles.begin();
         ii != addFiles.end(); ++ii)
        qDebug() << *ii;
    qDebug() << "removeFiles:";
    for (QStringList::const_iterator ii = removeFiles.begin();
         ii != removeFiles.end(); ++ii)
        qDebug() << *ii;
}

//
//
// DirMonitor
//
//

class DirMonitor::MarkList {
  public:
    typedef std::map<QString, bool> namelist_t;

    QString dirName;
    bool needsReload;
    namelist_t foundNames;

  public:
    MarkList(const QString &_dirname) : dirName(_dirname), needsReload(false) {}
};

DirMonitor::DirMonitor(void) {
    dm_queued = false;
    dm_requeue = false;
    connect(&dm_watcher, SIGNAL(directoryChanged(const QString &)), this,
            SLOT(onDirChanged(const QString &)));
}

void DirMonitor::addPath(const QString &path) {
    assert(dm_marklists.count(path) == 0);

    MarkList *m;
    dm_marklists[path].reset(m = new MarkList(path));

    dm_watcher.addPath(path);

    DirChangeEvent evt;
    reloadDir(*m, evt);
}

void DirMonitor::removePath(const QString &path) {
    assert(dm_marklists.count(path) > 0);

    dm_watcher.removePath(path);
    dm_marklists.erase(path);
}

void DirMonitor::onDirChanged(const QString &dir) {
    // qDebug() << __FUNCTION__;
    if (dm_marklists.count(dir) == 0)
        return; // was it removed while an effevent was triggered?

    dm_marklists[dir]->needsReload = true;

    if (dm_queued) {
        dm_requeue = true;
        return;
    }

    dm_queued = true;
    QTimer::singleShot(3000, this, SLOT(onTimer()));
}

void DirMonitor::onTimer(void) {
    // qDebug() << __FUNCTION__ << dm_currentdir;
    if (dm_requeue) {
        dm_requeue = false;
        QTimer::singleShot(3000, this, SLOT(onTimer()));
    }

    dm_queued = false;

    reloadDirs();
}

void DirMonitor::reloadDirs(void) {
    // qDebug() << __FUNCTION__;

    for (marklists_t::iterator ii = dm_marklists.begin();
         ii != dm_marklists.end(); ++ii)
        if (ii->second->needsReload) {
            ii->second->needsReload = false;

            DirChangeEvent event;

            reloadDir(*ii->second, event);

            if (!event.isEmpty())
                dirChange(ii->first, event);
        }
}

void DirMonitor::reloadDir(MarkList &marklist, DirChangeEvent &outevent) {
    // qDebug() << __FUNCTION__ << marklist.dirName;

    hydra::FileIterator I(marklist.dirName, false, false);

    // init the name

    for (MarkList::namelist_t::iterator ii = marklist.foundNames.begin();
         ii != marklist.foundNames.end(); ++ii)
        ii->second = false;

    while (I.hasNext()) {
        QString curfull(I.next());
        QString justname(hydra::justName(curfull));

        if (!hydra::isImageFile(justname))
            continue;

        // qDebug() << "checking" << curfull;

        // check if we have this file name
        MarkList::namelist_t::iterator ff = marklist.foundNames.find(curfull);

        if (ff == marklist.foundNames.end()) {
            // not found at all, this is a new file
            outevent.addFiles.push_back(curfull);
        } else {
            // mark it as found
            ff->second = true;
        }
    } // while

    // now find all the entries to remove
    for (MarkList::namelist_t::iterator ii = marklist.foundNames.begin();
         ii != marklist.foundNames.end(); ++ii)
        if (!ii->second)
            outevent.removeFiles.push_back(ii->first);

    // the event is ready
    if (!outevent.isEmpty())
        outevent.debugPrint();

    // now process the event for the local list
    for (QStringList::const_iterator ii = outevent.removeFiles.begin();
         ii != outevent.removeFiles.end(); ++ii)
        marklist.foundNames.erase(*ii);
    for (QStringList::const_iterator ii = outevent.addFiles.begin();
         ii != outevent.addFiles.end(); ++ii)
        marklist.foundNames[*ii] = false;
}

//
//
// DirDBUpdater
//
//

DirDBUpdater::DirDBUpdater(void) {}

void DirDBUpdater::dirChanged(const QString &path,
                              const DirChangeEvent &event) {
    // qDebug() << __FUNCTION__;

    for (QStringList::const_iterator ii = event.removeFiles.begin();
         ii != event.removeFiles.end(); ++ii)
        Engine::instance()->filePathDB().erase(*ii);
    for (QStringList::const_iterator ii = event.addFiles.begin();
         ii != event.addFiles.end(); ++ii)
        Engine::instance()->addFile(*ii);
}
