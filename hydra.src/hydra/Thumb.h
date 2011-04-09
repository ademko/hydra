
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_THUMB_H__
#define __INCLUDED_HYDRA_THUMB_H__

#include <QString>
#include <QImage>

namespace hydra
{
  /**
   * Retains the aspect ratio on resized dimensions.
   *
   * C, R current image width, heigth
   * WC WR the designed (wanted) width, heigth
   * c, r the resulting width, height
   * @param growtofit if true, images smaller than desired will be scaled UP to fit
   * @author Aleksander Demko
   */ 
  void calcAspect(unsigned long C, unsigned long R, unsigned long WC, unsigned long WR, unsigned long &c, unsigned long &r, 
      bool growtofit);

  class Thumb;
}

/**
 * Generator and maintainer of image thumbnails (and other scalled versions
 * of an image).
 *
 * Thumbnails are stores usually in ~/.hydra/thumbs/imghash[.SIZE].jpg
 * Where size is the thumbnail-desired size (but maybe not the image size, as the
 * aspect ratio will be conserved and therefore the image size will often be smaller
 * than the thumbnail size.
 *
 * @author Aleksander Demko
 */
class hydra::Thumb
{
  public:
    enum {
      Generate_Ok = 0,
      Generate_FileExists,
      Generate_LoadError,
    };

    static const int DEFAULT_VIEW_W = 800;
    static const int DEFAULT_VIEW_H = 600;
    static const int DEFAULT_THUMB_W = 266;
    static const int DEFAULT_THUMB_H = 200;

  public:
    /**
     * Constructor.
     * The source image file.
     * This doesnt load the file until (and if) Generate is called.
     *
     * @author Aleksander Demko
     */
    Thumb(const QString &filename);
    /**
     * Constructor.
     * Uses the existing in-memory QImage.
     *
     * @param note that no additional rotating will be done on this image
     *
     * @author Aleksander Demko
     */ 
    Thumb(QImage &srcimg);
    /// dtor
    ~Thumb();

    /**
     * Returns the thumb dir, which is usually in a subdir
     * of Engine::dbDir()
     *
     * @author Aleksander Demko
     */ 
    static QString thumbDir(void);

    /**
     * Creates the thumb nail directory.
     *
     * @author Aleksander Demko
     */
    static void mkThumbDir(void);

    /**
     * Computes the standard thumbnail filename (suitable to be given to generate())
     * for an image with the given hash and desired dimensions.
     *
     * @author Aleksander Demko
     */ 
    static QString fileName(const QString &hash, int rotateCode, unsigned long desiredW, unsigned long desiredH);

    /**
     * Makes the thumbnail and saves it to the given filename.
     *
     * If destimage is non-null, then the given image object will be loaded with
     * the genered thumbnail. If no thumbnail needed to be generated, then the existing one
     * will be read into the destimage.
     *
     * A Generate_* error code is returned.
     *
     * Returns a Generate_* code.
     *
     * @param rotateCode is how to rotate the image. it must be >=0
     * @author Aleksander Demko
     */
    int generate(const QString &destfilename, QImage *destimage,
        int rotateCode,
        unsigned long desiredW, unsigned long desiredH,
        unsigned long *actualW = 0, unsigned long *actualH = 0);

  private:
    QString dm_filename;

    QImage dm_img;

  private:
    int verifyLoadImage(void);
};

#endif

