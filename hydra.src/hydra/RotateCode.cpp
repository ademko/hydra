
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/RotateCode.h>

#include <assert.h>

#include <QDebug>

#include <hydra/Exif.h>

int hydra::rotateDegreesToCode(int degrees)
{
  switch (degrees) {
    case 0: return 0;
    case 90: return 1;
    case 180: return 2;
    case 270: return 3;
  }
  return -1;
}

int hydra::rotateCodeAdd(int rotateCode, int deltaCode)
{
  assert(rotateCode >= 0);
  assert(rotateCode < 4);

  rotateCode += deltaCode;

  if (rotateCode < 0)
    rotateCode = 4 - (-rotateCode) % 4;
  if (rotateCode > 3) // still need this case to fix the previous case... ugly algs :)
    rotateCode = rotateCode % 4;

  assert(rotateCode >= 0);
  assert(rotateCode < 4);

  return rotateCode;
}

short hydra::detectTagRotate(const hydra::FileItemRecord::tags_t &tags)
{
  if (tags.hasKey("rotate")) {
    QString r = tags.keyValue("rotate");
    bool ok = false;
    int i = r.toInt(&ok);

    if (!ok)
      return -1;

    return rotateDegreesToCode(i);
  }
  return -1;
}

void hydra::rotateSizeByCode(short rotatecode, unsigned long origW, unsigned long origH, unsigned long &newW, unsigned long &newH)
{
  assert(rotatecode >= 0);
  if ((rotatecode%2) == 0) {
    newW = origW;
    newH = origH;
  } else {
    // swap em
    newW = origH;
    newH = origW;
  }
}

QImage hydra::rotateImageByCode(short rotatecode, QImage &img)
{
  assert(rotatecode >= 0);

  if (rotatecode == 0)
    return img;

  QTransform x;
  x.rotate(rotateCodeToDegrees(rotatecode));
  return img.transformed(x);
}

QPixmap hydra::rotatePixmapByCode(short rotatecode, QPixmap &pix)
{
  assert(rotatecode >= 0);

  if (rotatecode == 0)
    return pix;

  QTransform x;
  x.rotate(rotateCodeToDegrees(rotatecode));
  return pix.transformed(x);
}

short hydra::detectMultiRotate(const QString &filename, const hydra::FileItemRecord::tags_t &tags)
{
  // first, use the tag thing
  short r;

  r = detectTagRotate(tags);

  if (r>=0)
    return r;

  // fall back to exif
  r = detectExifRotate(filename);

  if (r>=0)
    return r;

  return 0;
}

