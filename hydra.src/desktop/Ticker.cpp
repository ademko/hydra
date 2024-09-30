
/*
 * Copyright (c) 2024 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/Ticker.h>

using namespace desktop;

Ticker::Ticker(int tickMinMs)
    : dm_tickMinMs(tickMinMs) {
    dm_timer.start();
}

bool Ticker::dequeueTick(void) {
    if (dm_timer.elapsed() >= dm_tickMinMs) {
        dm_timer.restart();
        return true;
    }
    return false;
}

