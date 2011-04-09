
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_EXIF_H__
#define __INCLUDED_HYDRA_EXIF_H__

#include <QString>
#include <QImage>

namespace hydra
{
  /**
   * Detects the auto rotation information in the file, if any.
   * returns:
   * -1 error/no exif found.
   * 0 do not rotate
   * 1 rotate 90 degrees cw
   * 2 rotate 180 degrees cw
   * 3 rotate 270 degrees cw
   *
   * If exif detection support is not compiled, this function will always return 0.
   *
   * @author Aleksander Demko
   */ 
  short detectExifRotate(const QString &filename);
}

#endif

