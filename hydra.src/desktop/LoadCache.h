
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_LOADCACHE_H__
#define __INCLUDED_HYDRADESKTOP_LOADCACHE_H__

#include <assert.h>

#include <list>
#include <map>

#include <QDebug>
#include <QString>

#include <hydra/TR1.h>

namespace desktop {
template <class T, class KEY> class LoadCacheEntry;

template <class T, class KEY> class LoadCacheBase;
template <class T, class LOADER, class KEY> class LoadCache;

template <class T, class KEY> class cache_ptr;
} // namespace desktop

// internal class
template <class T, class KEY> class desktop::LoadCacheEntry {
  public:
    std::shared_ptr<T> ptr;
    bool isdel;

    typename std::list<
        typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator>::iterator
        list_iterator;

  public:
    LoadCacheEntry(void) : isdel(false) {}
};

/**
 * The LoaderCache foundation.
 *
 * This version doesn't have the automatic loader type stuff.
 *
 * @author Aleksander Demko
 */
template <class T, class KEY = QString> class desktop::LoadCacheBase {
  public:
    typedef T data_type;
    typedef KEY key_type;
    typedef desktop::cache_ptr<T, KEY> cache_ptr;

  public:
    explicit LoadCacheBase(size_t maxhold);

    //~LoadCacheBase() { qDebug() << __FUNCTION__; }

    bool containsItem(const KEY &key) const;

    void insertItem(const KEY &key, const std::shared_ptr<T> &item);

    // asserts if the item is not in the cache already
    desktop::cache_ptr<T, KEY> getItem(const KEY &key);

  protected:
    friend class desktop::cache_ptr<T, KEY>;

    size_t dm_maxhold, dm_curhold;

    std::map<KEY, LoadCacheEntry<T, KEY>> dm_keymap;
    std::list<typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator>
        dm_dellist;
};

/**
 * A LoadCache is a map that is able to manage a group of objects (T)
 * keyed by a key KEY, with the given demand LOADER class.
 *
 * This class will make sure all currently locked (via cache_ptr)
 * objects are in memory, aswell as possible some extras (within the
 * size maxhold as passed to the constructor
 *
 * @author Aleksander Demko
 */
template <class T, class LOADER, class KEY = QString>
class desktop::LoadCache : public LoadCacheBase<T, KEY> {
  public:
    explicit LoadCache(size_t maxhold, const LOADER &loader = LOADER())
        : LoadCacheBase<T, KEY>(maxhold), dm_loader(loader) {}

    desktop::cache_ptr<T, KEY> getItem(const KEY &key);

  protected:
    LOADER dm_loader;
};

template <class T, class KEY = QString> class desktop::cache_ptr {
  public:
    cache_ptr(void);
    cache_ptr(LoadCacheBase<T, KEY> *cache,
              typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator &ii);
    cache_ptr(const cache_ptr<T, KEY> &rhs);
    ~cache_ptr() { cleanup(); }

    cache_ptr<T, KEY> &operator=(const cache_ptr<T, KEY> &rhs);

    T *get(void) const { return dm_ptr.get(); }
    T &ref(void) const { return *dm_ptr; }
    T *operator->(void) const { return dm_ptr.get(); }
    T &operator*(void) const { return *dm_ptr; }

    bool operator==(const cache_ptr<T, KEY> &rhs) const {
        return dm_ptr == rhs.dm_ptr;
    }
    bool operator!=(const cache_ptr<T, KEY> &rhs) const {
        return dm_ptr != rhs.dm_ptr;
    }
    bool operator<(const cache_ptr<T, KEY> &rhs) const {
        return dm_ptr < rhs.dm_ptr;
    }

  private:
    void cleanup(void);

  private:
    LoadCacheBase<T, KEY> *dm_cache;
    typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator dm_iterator;
    std::shared_ptr<T> dm_ptr;
};

template <class T, class KEY>
desktop::LoadCacheBase<T, KEY>::LoadCacheBase(size_t maxhold)
    : dm_maxhold(maxhold), dm_curhold(0) {}

template <class T, class KEY>
bool desktop::LoadCacheBase<T, KEY>::containsItem(const KEY &key) const {
    return dm_keymap.count(key) > 0;
}

template <class T, class KEY>
void desktop::LoadCacheBase<T, KEY>::insertItem(
    const KEY &key, const std::shared_ptr<T> &item) {
    assert(!containsItem(key));

    dm_keymap[key].ptr = item;
}

template <class T, class KEY>
desktop::cache_ptr<T, KEY>
desktop::LoadCacheBase<T, KEY>::getItem(const KEY &key) {
    typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator ii(
        LoadCacheBase<T, KEY>::dm_keymap.find(key));

    assert(ii != (LoadCacheBase<T, KEY>::dm_keymap
                      .end())); // need the parens... gcc bug?
    assert(ii->second.ptr.get());

    return desktop::cache_ptr<T, KEY>(this, ii);
}

template <class T, class LOADER, class KEY>
desktop::cache_ptr<T, KEY>
desktop::LoadCache<T, LOADER, KEY>::getItem(const KEY &key) {
    typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator ii(
        LoadCacheBase<T, KEY>::dm_keymap.find(key));

    // load if it need be
    if (ii == LoadCacheBase<T, KEY>::dm_keymap.end()) {
        // load it
        LoadCacheBase<T, KEY>::dm_keymap[key].ptr = dm_loader(key);
        ii = LoadCacheBase<T, KEY>::dm_keymap.find(key);
    }
    assert(ii != (LoadCacheBase<T, KEY>::dm_keymap
                      .end())); // need the parens... gcc bug?
    assert(ii->second.ptr.get());

    return desktop::cache_ptr<T, KEY>(this, ii);
}

template <class T, class KEY>
desktop::cache_ptr<T, KEY>::cache_ptr(void) : dm_cache(0) {
    // qDebug() << "cache_ptr(void)";
}

template <class T, class KEY>
desktop::cache_ptr<T, KEY>::cache_ptr(
    LoadCacheBase<T, KEY> *cache,
    typename std::map<KEY, LoadCacheEntry<T, KEY>>::iterator &ii)
    : dm_cache(cache), dm_iterator(ii) {
    // qDebug() << "cache_ptr(init)";
    // extraction from cache

    assert(cache);

    if (dm_iterator->second.isdel) {
        // save it from deletion
        dm_cache->dm_curhold--;
        dm_cache->dm_dellist.erase(dm_iterator->second.list_iterator);
        dm_iterator->second.isdel = false;
    }

    dm_ptr = dm_iterator->second.ptr;

    assert(!dm_iterator->second.isdel);

    assert(dm_ptr.get());
    assert(!dm_ptr.unique());
}

template <class T, class KEY>
desktop::cache_ptr<T, KEY>::cache_ptr(const cache_ptr<T, KEY> &rhs)
    : dm_cache(rhs.dm_cache), dm_iterator(rhs.dm_iterator), dm_ptr(rhs.dm_ptr) {
    if (dm_cache) {
        assert(dm_ptr.get());
        assert(!dm_ptr.unique());
        assert(!dm_iterator->second.isdel);
    }
    // qDebug() << "cache_ptr(copy)";
}

/*template <class T, class KEY>
  desktop::cache_ptr<T>::~cache_ptr()
{
  //qDebug() << "cache_ptr(dtor)";
  cleanup();
}*/

template <class T, class KEY>
desktop::cache_ptr<T, KEY> &
desktop::cache_ptr<T, KEY>::operator=(const desktop::cache_ptr<T, KEY> &rhs) {
    // qDebug() << "cache_ptr(assignment)";
    // check if we are already a copy
    if (dm_cache == rhs.dm_cache &&
        (!dm_cache || dm_iterator == rhs.dm_iterator))
        return *this;

    cleanup();

    dm_cache = rhs.dm_cache;
    dm_iterator = rhs.dm_iterator;
    dm_ptr = rhs.dm_ptr;

    if (dm_cache) {
        assert(dm_ptr.get());
        assert(!dm_ptr.unique());
        assert(!dm_iterator->second.isdel);
    }

    return *this;
}

template <class T, class KEY> void desktop::cache_ptr<T, KEY>::cleanup(void) {
    if (!dm_cache)
        return;

    // qDebug() << "cache_ptr(cleanup) enter, curhold=" << dm_cache->dm_curhold
    // << " maxhold=" << dm_cache->dm_maxhold;

    assert(dm_ptr.get());
    assert(!dm_ptr.unique());
    dm_ptr.reset();

    // assert(dm_cache->dm_dellist.size() == dm_cache->dm_curhold);

    // qDebug() << __FUNCTION__ << dm_iterator->second.ptr.get() << "pre_del";
    assert(!dm_iterator->second.isdel);
    if (dm_iterator->second.ptr.unique()) {
        // add it to the delete queue, as noone else points to it anymore
        // qDebug() << __FUNCTION__ << dm_iterator->second.ptr.get() << "ISDEL =
        // true";
        dm_iterator->second.list_iterator = dm_cache->dm_dellist.insert(
            dm_cache->dm_dellist.end(), dm_iterator);
        dm_iterator->second.isdel = true;
        dm_cache->dm_curhold++;

        // prune the del list as needed
        while (dm_cache->dm_curhold > dm_cache->dm_maxhold) {
            // delete the front of the list and its map node
            // qDebug() << __FUNCTION__ <<
            // dm_cache->dm_dellist.front()->second.ptr.get() << "... purged!";
            dm_cache->dm_keymap.erase(dm_cache->dm_dellist.front());
            dm_cache->dm_dellist.pop_front();
            dm_cache->dm_curhold--;
            // qDebug() << "cache_ptr(cleanup) just NUKED one, curhold=" <<
            // dm_cache->dm_curhold << " maxhold=" << dm_cache->dm_maxhold;
        }
    }

    dm_cache = 0;
}

/*TEST CODE

#include <desktop/LoadCache.h>
class Data
{
  public:
    Data(const QString &d) : dm_data("DATA" + d) { qDebug() << __FUNCTION__ <<
dm_data; } ~Data() { qDebug() << __FUNCTION__ << dm_data; } private: QString
dm_data;
};
class ImageLoader
{
  public:
    std::shared_ptr<Data> operator()(const QString &fullfilename) {
      return std::shared_ptr<Data>(new Data(fullfilename));
    }
};
using namespace desktop;
void foo(void)
{
  //std::list< std::list<int>::iterator > iterator_list;
  typedef LoadCache<Data, ImageLoader> cache_type;
  cache_type cache(3);
  cache_ptr<Data> holder;

  {
    cache_ptr<Data> p1(cache.getItem("s1"));
    cache_ptr<Data> p2(cache.getItem("s2"));
    cache_ptr<Data> p3(cache.getItem("s3"));
    cache_ptr<Data> p4(cache.getItem("s4"));
    cache_ptr<Data> p5(cache.getItem("s5"));
    cache_ptr<Data> p6(cache.getItem("s6"));
    cache_ptr<Data> p7(cache.getItem("s7"));
    cache_ptr<Data> p8(cache.getItem("s8"));

    holder = p8;
  }


  qDebug() << "exiting";
}
*/
#endif
