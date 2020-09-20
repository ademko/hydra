
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FileItemCache.h>

#include <QDebug>

#include <hydra/Engine.h>

using namespace hydra;
using namespace desktop;

FileItemCache *FileItemCache::dm_instance;

FileItemCache::FileItemCache(void) : dm_cache(10000) {
    assert(false); // remove this when we want to reenable this calss
    assert(dm_instance == 0);
    dm_instance = this;
}

FileItemCache::~FileItemCache() {
    assert(dm_instance == this);
    dm_instance = 0;
}

/*bool FileItemCache::hasItem(const QString &fullfilename)
{
  return dm_cache.hasItem(fullfilename);
}*/

bool FileItemCache::getItem(
    const QString &fullfilename,
    desktop::cache_ptr<hydra::FileItemRecord, QUuid> &outitem, QString &outhash,
    bool *needsBigRead) {
    FilePathRecord pathrec;
    FileHashRecord hashrec;
    int code;
    std::shared_ptr<FileItemRecord> item(new FileItemRecord);

    code = Engine::instance()->getFileItem(fullfilename, item.get(), &hashrec,
                                           &pathrec);

    if (code != Engine::Load_OK) {
        if (needsBigRead) {
            // big operation would follow. bail if the caller wanted us to
            *needsBigRead = true;
            return true;
        }

        if (Engine::instance()->addFile(fullfilename) != Engine::Add_Error)
            code = Engine::instance()->getFileItem(fullfilename, item.get(),
                                                   &hashrec, &pathrec);
    }

    if (code != Engine::Load_OK)
        return false;

    // do we already have this record for this UUID loaded?
    if (!dm_cache.containsItem(hashrec.id))
        dm_cache.insertItem(hashrec.id, item);
    // else qDebug() << "USING CACHED FileItemRecord for " << fullfilename;

    outhash = pathrec.hash;
    outitem = dm_cache.getItem(hashrec.id);

    return true;
}
