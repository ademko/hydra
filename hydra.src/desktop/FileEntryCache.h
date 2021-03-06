
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FILEENTRYCACHE_H__
#define __INCLUDED_HYDRADESKTOP_FILEENTRYCACHE_H__

#include <desktop/FileEntry.h>
#include <desktop/LoadCache.h>

namespace desktop {
class FileEntryCache;
}

class desktop::FileEntryCache {
  public:
    FileEntryCache(void);

    desktop::cache_ptr<desktop::FileEntry>
    getEntry(const QString &fullfilename) {
        return dm_loader.getItem(fullfilename);
    }

  private:
    class EntryLoader {
      public:
        std::shared_ptr<desktop::FileEntry>
        operator()(const QString &fullfilename) {
            return std::shared_ptr<desktop::FileEntry>(
                new desktop::FileEntry(fullfilename));
        }
    };

    LoadCache<FileEntry, EntryLoader> dm_loader;
};

#endif
