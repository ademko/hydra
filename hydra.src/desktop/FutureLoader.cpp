
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FutureLoader.h>

#include <assert.h>

#include <hydra/Thumb.h>
#include <hydra/RotateCode.h>
#include <desktop/FileList.h>
#include <desktop/RunnableEvent.h>

#include <QThreadPool>
#include <QDebug>

using namespace desktop;

FutureLoader::FutureLoader(ThreadedThumbCache *thumbcache, FileList *flist)
  : FileListListener(flist),
    dm_thumbcache(thumbcache), dm_flist(flist)
{
  assert(dm_thumbcache);
  assert(dm_flist);

  dm_running = false;
  dm_quit = false;
  dm_hashstage = true;
}

FutureLoader::~FutureLoader()
{
  flushJobs();
  QThreadPool::globalInstance()->waitForDone();
}

void FutureLoader::flushJobs(void)
{
  QMutexLocker L(&dm_lock);

  dm_quit = true;
}

void FutureLoader::start(void)
{
  QMutexLocker L(&dm_lock);

  dm_lastdoneindex = 0;
  dm_hashstage = true;

  if (dm_running)
    return;

  dm_running = true;
  dm_quit = false;

  RunnableEventFunction::enqueueWorker(
      std::bind(std::mem_fn(&FutureLoader::workerFunc), this)
      , -1);
}


void FutureLoader::workerFunc(void)
{
//qDebug() << "FutureLoader" << __FUNCTION__;
  QMutexLocker L(&dm_lock);

  if (dm_quit)
    return;

//qDebug() << "FutureLoader" << __FUNCTION__ << "enqueueMain";
  RunnableEventFunction::enqueueMain(
      std::bind(std::mem_fn(&FutureLoader::mainFunc), this)
      );
}

void FutureLoader::mainFunc(void)
{
//qDebug() << "FutureLoader" << __FUNCTION__;
  QMutexLocker L(&dm_lock);

  // check of quitting time
  if (dm_quit || dm_flist->numFiles() == 0) {
    dm_running = false;
    return;
  }

  int numFiles = dm_flist->numFiles();

  assert(dm_lastdoneindex>=0);
  if (dm_lastdoneindex >= numFiles)
    dm_lastdoneindex = 0;

//qDebug() << "FutureLoader" << __FUNCTION__ << "prehash";
  if (dm_hashstage) {
    // first stage, just load the file records
    int totest = 10; // quick check at most 100 files per pass (we are in the main gui thread after all)
    bool needsBigRead;

    while (dm_lastdoneindex < numFiles && totest > 0) {
      // check if this is something we can work on
      // check file info
      FileEntry &entry = (*dm_flist).fileAt(dm_lastdoneindex);
      
//qDebug() << "FutureLoader.record" << entry.fullfilename() << "@" << dm_lastdoneindex;
      entry.containsRecord(&needsBigRead);
      if (needsBigRead) {
        totest = 0;
//qDebug() << "FutureLoader" << "enqueue_background_hash" << entry.fullfilename();
        dm_flist->enqueueLoad(entry.fullfilename(), dm_lastdoneindex);
      } else {
        --totest;
      }

      // increment
      ++dm_lastdoneindex;
    }//while

    // see if we've completed this stage
    //if (totest>0) {
    if (dm_lastdoneindex >= numFiles) {
      dm_hashstage = false;
      dm_lastdoneindex = 0;
    }
  }//if dm_hashstage

//qDebug() << "FutureLoader" << __FUNCTION__ << "thumbstage";
  if (!dm_hashstage) {
    int totest = 100; // quick check at most 100 files per pass (we are in the main gui thread after all)
    // 2nd stage, load thumbs
    while (dm_lastdoneindex < numFiles && totest > 0) {
      // check if this is something we can work on
      // check file info
      FileEntry &entry = (*dm_flist).fileAt(dm_lastdoneindex);

//qDebug() << "FutureLoader.thumb" << entry.fullfilename() << "@" << dm_lastdoneindex;
      // check thumb info
      if (!dm_thumbcache->containsPixmap(/*entry.fullfilename(),*/ entry.recordHash(), entry.rotateCode(), hydra::Thumb::DEFAULT_THUMB_W, hydra::Thumb::DEFAULT_THUMB_H)) {
        totest = 0;

//qDebug() << "FutureLoader" << "enqueue_backgroud_thumb" << entry.fullfilename();
        dm_thumbcache->getPixmap(entry.fullfilename(), entry.recordHash(),
            entry.rotateCode(),
            hydra::Thumb::DEFAULT_THUMB_W, hydra::Thumb::DEFAULT_THUMB_H,
            dm_flist, dm_lastdoneindex);
      } else {
        --totest;
      }

      // increment
      ++dm_lastdoneindex;
    }//while

    //if (totest>0) {
    if (dm_lastdoneindex >= numFiles) {
//qDebug() << "FutureLoader.quitting";
      // we are DONE as we've exhausted the list
      dm_running = false;
    }
  }//if dm_hashstage
  // find the next task to do

//qDebug() << "FutureLoader" << __FUNCTION__ << "done" << dm_running;
  // not done, enqueue myself to run another time
  if (dm_running)
    RunnableEventFunction::enqueueWorker(
        std::bind(std::mem_fn(&FutureLoader::workerFunc), this)
        , -1);
}

