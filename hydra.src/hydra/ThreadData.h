
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_THREADAREA_H__
#define __INCLUDED_HYDRA_THREADAREA_H__

#include <QMutex>

namespace hydra
{
  template <class T> class MutexData;

  template <class T> class MutexPtr;
}

template <class T> class hydra::MutexData
{
  public:
    typedef T DataType;
  public:
    volatile T data;
    QMutex mutex;
};

template <class T> class hydra::MutexPtr
{
  public:
    typedef T DataType;
  public:
    MutexPtr(MutexData<T> &sa)
      : dm_ptr(const_cast<T*>(&sa.data)), dm_data(sa)
      { dm_data.mutex.lock(); }
    ~MutexPtr() { dm_data.mutex.unlock(); }

    T& operator*(void) { return *dm_ptr; }
    T* operator->(void) { return dm_ptr; }
  private:
    T * dm_ptr;
    MutexData<T> &dm_data;
};

#endif

