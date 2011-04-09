
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_TR1_H__
#define __INCLUDED_HYDRA_TR1_H__

/*
 * This folders std::tr1::shared_ptr into std::shared_ptr
 * if need be, so that it is consistance across platforms.
 */

#include <QtGlobal>

#ifdef Q_OS_WIN32
#include <memory>
#include <functional>
#include <array>
#else
#include <tr1/memory>
#include <tr1/functional>
#include <tr1/array>

#endif

namespace std
{
  using namespace std::tr1;
  using namespace std::tr1::placeholders;
}

#endif

