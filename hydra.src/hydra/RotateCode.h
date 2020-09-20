
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_ROTATETAG_H__
#define __INCLUDED_HYDRA_ROTATETAG_H__

#include <QImage>
#include <QPixmap>

#include <hydra/Records.h>

namespace hydra {
/**
 * Converts a rotate code to degrees, for display.
 *
 * @author Aleksander Demko
 */
inline int rotateCodeToDegrees(int rotateCode) { return rotateCode * 90; }

/**
 * Converts degrees to a rotate code.
 * Returns -1 on invalid codes.
 *
 * @author Aleksander Demko
 */
int rotateDegreesToCode(int degrees);

/**
 * Adds the given amount (which can be negative)o to the rotateCode
 * and returns the result.
 * The returned result is always valid (0..3)
 *
 * @author Aleksander Demko
 */
int rotateCodeAdd(int rotateCode, int deltaCode);

/**
 * Detects the auto rotation information in the file, if any.
 * -1 error/no exif found.
 * 0 do not rotate
 * 1 rotate 90 degrees cw
 * 2 rotate 180 degrees cw
 * 3 rotate 270 degrees cw
 *
 * If exif detection support is not compiled, this function will always return
 * 0.
 *
 * @author Aleksander Demko
 */
short detectTagRotate(const hydra::FileItemRecord::tags_t &tags);

/**
 * Converts the given coordinates to the final coordinates, as if it where
 * rotated by the given exif rotation code. Node, the variables can the same
 * (&origW == newW, etc)
 *
 * @author Aleksander Demko
 */
void rotateSizeByCode(short rotatecode, unsigned long origW,
                      unsigned long origH, unsigned long &newW,
                      unsigned long &newH);

/**
 * Rotates an image as per the given rotate code.
 * 0 do not rotate
 * 1 rotate 90 degrees cw
 * 2 rotate 180 degrees cw
 * 3 rotate 270 degrees cw
 * rotatecode must be >=0
 *
 * @author Aleksander Demko
 */
QImage rotateImageByCode(short rotatecode, QImage &img);

/**
 * Rotates an pixmap as per the given rotate code.
 * 0 do not rotate
 * 1 rotate 90 degrees cw
 * 2 rotate 180 degrees cw
 * 3 rotate 270 degrees cw
 * rotatecode must be >=0
 *
 * @author Aleksander Demko
 */
QPixmap rotatePixmapByCode(short rotatecode, QPixmap &pix);

/**
 * Uses both detectExifRotate and detectTagRotate to come up with a reasonable
 * default. Will never return -1 (will return 0 instead).
 *
 * @author Aleksander Demko
 */
short detectMultiRotate(const QString &filename,
                        const hydra::FileItemRecord::tags_t &tags);
} // namespace hydra

#endif
