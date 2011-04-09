
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Thumb.h>

#include <assert.h>

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <hydra/Engine.h>
#include <hydra/RotateCode.h>
#include <hydra/Exif.h>

using namespace hydra;

// current, want, output
void hydra::calcAspect(unsigned long C, unsigned long R, unsigned long WC, unsigned long WR, unsigned long &c, unsigned long &r, bool growtofit)
{
  if (!growtofit && R<=WR && C<=WC) {
    r = R;
    c = C;
    return;
  }
  double scr, scc;

  scr = static_cast<double>(WR)/R;
  scc = static_cast<double>(WC)/C;
  // assume scc is the smaller
  if (scr < scc)
    scc = scr; //its not?

  r = static_cast<unsigned long>(scc * R);
  c = static_cast<unsigned long>(scc * C);
}

Thumb::Thumb(const QString &filename)
  : dm_filename(filename)
{
}

Thumb::Thumb(QImage &srcimg)
  : dm_img(srcimg)
{
  assert(!dm_img.isNull());
}

Thumb::~Thumb()
{
}

QString Thumb::thumbDir(void)
{
  return Engine::dbDir() + "/thumbs";
}

void Thumb::mkThumbDir(void)
{
  QDir("/").mkdir(thumbDir());
}

QString Thumb::fileName(const QString &hash, int rotateCode, unsigned long desiredW, unsigned long desiredH)
{
  QString ret;
  QTextStream str(&ret);

  str << thumbDir() << "/" << hash << "." << desiredW << "x" << desiredH << "." << rotateCodeToDegrees(rotateCode) << ".jpg";

  return ret;
}

int Thumb::generate(const QString &destfilename, QImage *destimage,
        int rotateCode,
        unsigned long desiredW, unsigned long desiredH,
        unsigned long *actualW, unsigned long *actualH)
{
  assert(rotateCode >= 0);

  int ret;

  if (QFileInfo(destfilename).exists()) {
    if (destimage) {
      // the caller wants a copy of the existing image
      if (destimage->load(destfilename)) {
        if (actualW)
          *actualW = destimage->width();
        if (actualH)
          *actualH = destimage->height();
        return Generate_FileExists;   // good load of existing thumb
      }
      // the loading of the existing thumb was unsuccesfull... fall through and try to make it again
    } else
      return Generate_FileExists;
  }

  ret = verifyLoadImage();
  if (ret != Generate_Ok)
    return ret;

  assert(!dm_img.isNull());

  unsigned long row, col;
  unsigned long desiredW_rotated, desiredH_rotated;

  // we want to scale first, before the exif (if any) rotation
  // not technically correct, but good enough
  rotateSizeByCode(rotateCode, desiredW, desiredH, desiredW_rotated, desiredH_rotated);

  calcAspect(dm_img.width(), dm_img.height(), desiredW_rotated, desiredH_rotated, col, row, false);

  QImage scaled_image;
  if (!destimage)
    destimage = &scaled_image;
  assert(destimage);

  /*unsigned long irow = row*2, icol = col*2;

  // This is a thumnbnailer speedup as per http://labs.trolltech.com/blogs/2009/01/26/creating-thumbnail-preview/
  // it doesnt seem faster though, atleast not by much, so I'm not using it!
  if (icol < dm_img.width() && irow < dm_img.height())
{qDebug() << "FAST::generate";
    // perform a fast two-phase scale (first NN then smooted)
    *destimage = dm_img.scaled(icol, irow).scaled(QSize(col, row), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
  else*/
    *destimage = dm_img.scaled(QSize(col, row), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  *destimage = rotateImageByCode(rotateCode, *destimage);

  (*destimage).save(destfilename);

  if (actualW)
    *actualW = destimage->width();
  if (actualH)
    *actualH = destimage->height();

  return Generate_Ok;
}

int Thumb::verifyLoadImage(void)
{
  if (!dm_img.isNull())
    return Generate_Ok; // nothing to do, already loaded

  if (dm_img.load(dm_filename))
    return Generate_Ok;
  else
    return Generate_LoadError;
}

