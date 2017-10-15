
/*
 * Copyright (c) 2017 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#pragma once

/*
 * This used to fold the TR1 stuff into std, but now since we've moved
 * to C++14, its not longer needed.
 */

#include <QtGlobal>

#ifdef Q_OS_WIN32
#include <memory>
#include <functional>
#include <array>
#else
#include <memory>
#include <functional>
#include <array>
#endif

