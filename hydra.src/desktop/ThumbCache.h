
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_THUMBCACHE_H__
#define __INCLUDED_HYDRADESKTOP_THUMBCACHE_H__

#include <list>
#include <map>
#include <set>

#include <QMutex>
#include <QPixmap>

#include <desktop/ImageCache.h>
#include <desktop/LoadCache.h>

namespace desktop {
class ThreadedThumbCache;

class FileList; // fwd
};              // namespace desktop

/**
 * A ThumbCache that queues background threads to fill in any missing
 * thumbnails.
 *
 * @author Aleksander Demko
 */
class desktop::ThreadedThumbCache {
  public:
    /// constructor
    ThreadedThumbCache(void);
    // destructor
    ~ThreadedThumbCache();

    /// signals all the jobs to die quickly
    void flushJobs(void);

    /// check if this pixmap is in the cache
    /// done need the fullfilename (for now) as its all hash/rotateCode based
    bool containsPixmap(/*const QString &fullfilename,*/ const QString &hash,
                        int rotateCode, int windoww, int windowh);

    /// returns a cached pixmap
    /// if one needs to be generated, it does so via a background thread while
    /// immediatly returning a null ptr rotate code can be -1 if "not specified"
    desktop::cache_ptr<QPixmap> getPixmap(const QString &fullfilename,
                                          const QString &hash, int rotateCode,
                                          int windoww, int windowh,
                                          FileList *fileList = 0,
                                          int indexOfFile = -1);

  private:
    void workerFunc(QString fullfilename, QString thumbName, QString hash,
                    int rotateCode, int windoww, int windowh,
                    FileList *fileList, int indexOfFile);
    void mainFunc(QString fullfilename, QString thumbName, QImage img,
                  FileList *fileList, int indexOfFile);

  private:
    // all access to the member variables and parent functions are protected
    // by this mutex

    LoadCacheBase<QPixmap> dm_cache;

    QMutex dm_pendingjobs_lock;
    std::set<QString> dm_pendingjobs;
};

#endif
