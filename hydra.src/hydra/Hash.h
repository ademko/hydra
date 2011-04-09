
/*
 * Copyright (c) 2005 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_HASH_H__
#define __INCLUDED_HYDRA_HASH_H__

#include <QString>

namespace hydra
{
  /**
   * Calculates the has of the given file.
   *
   * @return The hash of the file (in ascii form) or an empty string on error.
   * @author Aleksander Demko
   */ 
  QString calcFileHash(const QString &filename);
}

#endif

