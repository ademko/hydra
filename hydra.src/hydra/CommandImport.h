
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_COMMANDIMPORT_H__
#define __INCLUDED_HYDRA_COMMANDIMPORT_H__

#include <QString>

namespace hydra
{
  // not a public function, used by hydra
  // in the future, make this a public object
  // that uses usable by the gui (takes a filename string,
  // does some kind of progressable-iteration
  // returns number of entries processed, or -1 on error
  int commandImport(const QString &filename, bool smartmerge);
}

#endif

