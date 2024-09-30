
/*
 * Copyright (c) 2024 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_TICKER_H__
#define __INCLUDED_HYDRADESKTOP_TICKER_H__

#include <QElapsedTimer>

namespace desktop {
    class Ticker;
}

class desktop::Ticker {
  public:
    Ticker(int tickMinMs);

    bool dequeueTick(void);

  private:
    QElapsedTimer dm_timer;
    int dm_tickMinMs;
};

#endif

