
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/ImageCache.h>

#include <QDebug>

#include <hydra/Exif.h>
#include <hydra/RotateCode.h>
#include <hydra/Thumb.h>

using namespace desktop;

//
//
// ImageCache
//
//

ImageCache::ImageCache(int maxhold) : dm_cache(maxhold) {}

QPixmap ImageCache::getPixmap(QImage &image, int windoww, int windowh,
                              bool growtofit) {
    unsigned long final_w, final_h;

    hydra::calcAspect(image.width(), image.height(), windoww, windowh, final_w,
                      final_h, growtofit);

    QImage scaled_image =
        image.scaled(QSize(final_w, final_h), Qt::IgnoreAspectRatio,
                     Qt::SmoothTransformation);
    return QPixmap::fromImage(scaled_image);
}

std::shared_ptr<QImage>
ImageCache::ImageLoader::operator()(const QString &fullfilename) {
    std::shared_ptr<QImage> i(new QImage);

    i->load(fullfilename);

    return i;

    /*bool ok = i->load(fullfilename);
    int rotatecode = hydra::detectExifRotate(fullfilename);

    if (rotatecode < 0)
      rotatecode = 0;

    if (ok)
      *i = hydra::rotateImageByCode(rotatecode, *i);
    else
      *i = QImage();

    return i;*/
}
