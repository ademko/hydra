
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FILEITEMCACHE_H__
#define __INCLUDED_HYDRADESKTOP_FILEITEMCACHE_H__

#include <desktop/LoadCache.h>
#include <hydra/Records.h>

namespace desktop {
class FileItemCache;
}

/**
 * A cache mapping file hashes to FileItemRecords.
 *
 * THIS CLASS IS CURRENTLY NOT BEING USED.
 * Will reenable in the future when we want to catch circular references.
 *
 * @author Aleksander Demko
 */
class desktop::FileItemCache {
  public:
    typedef LoadCacheBase<hydra::FileItemRecord, QUuid>::cache_ptr cache_ptr;

  public:
    FileItemCache(void);
    ~FileItemCache();

    static FileItemCache *instance(void) { return dm_instance; }

    // bool containsItem(const QString &fullfilename);

    // returns true on success
    bool getItem(const QString &fullfilename,
                 desktop::cache_ptr<hydra::FileItemRecord, QUuid> &outitem,
                 QString &outhash, bool *needsBigRead = 0);

  private:
    LoadCacheBase<hydra::FileItemRecord, QUuid> dm_cache;

  private:
    static FileItemCache *dm_instance;
};

#endif
