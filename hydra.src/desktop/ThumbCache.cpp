
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/ThumbCache.h>

#include <hydra/TR1.h>

#include <assert.h>

#include <QDebug>
#include <QPaintEvent>
#include <QThreadPool>
#include <QWidget>

#include <desktop/MainWindow.h>
#include <desktop/RunnableEvent.h>
#include <hydra/Thumb.h>

using namespace desktop;

//
//
// ThreadedThumbCache
//
//

ThreadedThumbCache::ThreadedThumbCache(void) : dm_cache(1000) {}

ThreadedThumbCache::~ThreadedThumbCache() {
    flushJobs();
    QThreadPool::globalInstance()->waitForDone();
}

void ThreadedThumbCache::flushJobs(void) {
    QMutexLocker L(&dm_pendingjobs_lock);

    dm_pendingjobs.clear();
}

bool ThreadedThumbCache::containsPixmap(
    /*const QString &fullfilename,*/ const QString &hash, int rotateCode,
    int windoww, int windowh) {
    return dm_cache.containsItem(
        hydra::Thumb::fileName(hash, rotateCode, windoww, windowh));
}

desktop::cache_ptr<QPixmap>
ThreadedThumbCache::getPixmap(const QString &fullfilename, const QString &hash,
                              int rotateCode, int windoww, int windowh,
                              FileList *fileList, int indexOfFile) {
    QString thumbName(
        hydra::Thumb::fileName(hash, rotateCode, windoww, windowh));

    if (dm_cache.containsItem(thumbName))
        return dm_cache.getItem(thumbName);

    // always enqueue a new task for this, so that its the higest priority
    // it doesnt matter that it might be already running
    QMutexLocker L(&dm_pendingjobs_lock);
    dm_pendingjobs.insert(thumbName);

    RunnableEventFunction::enqueueWorker(
        std::bind(std::mem_fn(&ThreadedThumbCache::workerFunc), this,
                  fullfilename, thumbName, hash, rotateCode, windoww, windowh,
                  fileList, indexOfFile),
        RunnableEvent::nextPriority());

    return desktop::cache_ptr<QPixmap>();
}

void ThreadedThumbCache::workerFunc(QString fullfilename, QString thumbName,
                                    QString hash, int rotateCode, int windoww,
                                    int windowh, FileList *fileList,
                                    int indexOfFile) {
    {
        QMutexLocker L(&dm_pendingjobs_lock);

        // check to see if this job was already done right from under us
        if (dm_pendingjobs.count(thumbName) == 0)
            return;
    }

    // runs in a worker thread
    QImage img;

    hydra::Thumb T(fullfilename);

    T.generate(thumbName, &img, rotateCode, windoww, windowh);
    // assert(!img.isNull());  // would trigger on corrupt images

    RunnableEventFunction::enqueueMain(
        std::bind(std::mem_fn(&ThreadedThumbCache::mainFunc), this,
                  fullfilename, thumbName, img, fileList, indexOfFile));
}

void ThreadedThumbCache::mainFunc(QString fullfilename, QString thumbName,
                                  QImage img, FileList *fileList,
                                  int indexOfFile) {
    // runs in the main guithread

    {
        QMutexLocker L(&dm_pendingjobs_lock);

        if (dm_pendingjobs.count(thumbName) == 0)
            return;
        else
            dm_pendingjobs.erase(thumbName);
    }

    dm_cache.insertItem(thumbName, std::shared_ptr<QPixmap>(
                                       new QPixmap(QPixmap::fromImage(img))));

    if (fileList) {
        if (indexOfFile == -1)
            fileList->emitChangedTagsAll();
        else
            fileList->emitChangedTagsOne(indexOfFile);
    }
}
