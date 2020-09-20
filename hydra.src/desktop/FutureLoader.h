
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_FUTURELOADER_H__
#define __INCLUDED_FUTURELOADER_H__

#include <desktop/ThumbCache.h>
#include <desktop/View.h>

#include <QMutex>

namespace desktop {
class FutureLoader;
}

/**
 * This, via QThreadPool will fill out the unloaded hash and thumnb entries
 * of a FileList.
 *
 * @author Aleksander Demko
 */
class desktop::FutureLoader : public FileListListener {
  public:
    /// constructor
    FutureLoader(ThreadedThumbCache *thumbcache, FileList *flist);
    /// destructor
    ~FutureLoader();

    virtual void onBaseChange(FileList *fl) { start(); }

    /// signals all the jobs to die quickly
    void flushJobs(void);

    // start processing the list
    // if its already running, this function does nothing
    void start(void);

  private:
    void workerFunc(void);
    void mainFunc(void);

  private:
    ThreadedThumbCache *dm_thumbcache;
    FileList *dm_flist;

    QMutex dm_lock;
    bool dm_running;
    bool dm_quit; // should the running thread (if any) quit right now
    bool dm_hashstage;

    int dm_lastdoneindex;
};

#endif
