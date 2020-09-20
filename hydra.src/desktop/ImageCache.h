
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_IMAGECACHE_H__
#define __INCLUDED_HYDRADESKTOP_IMAGECACHE_H__

#include <hydra/TR1.h>
#include <list>

#include <QImage>
#include <QPixmap>
#include <QString>

#include <desktop/LoadCache.h>

namespace desktop {
class ImageCache;
}

/**
 * A image-reading cache, useful for the big image viewers. Also does rescaling
 * of the image as needed. The rescaled versions however are not cached.
 *
 * @author Aleksander Demko
 */
class desktop::ImageCache {
  public:
    /// constructor
    ImageCache(int maxhold = 5);

    /**
     * Is the given fullfilename in the cache?
     *
     * @author Aleksander Demko
     */
    bool containsImage(const QString &fullfilename) const {
        return dm_cache.containsItem(fullfilename);
    }

    /**
     * Loads the given file as an image, from cache if possible.
     * This function never fails and never returns null - on error, an empty
     * image will be returned.
     *
     * @author Aleksander Demko
     */
    desktop::cache_ptr<QImage> getImage(const QString &fullfilename) {
        return dm_cache.getItem(fullfilename);
    }

    /**
     * Note that this is a static function, and can take in image.
     * Obviously, the resulting QPixmaps are saved or cached in any way.
     *
     * @author Aleksander Demko
     */
    static QPixmap getPixmap(QImage &image, int windoww, int windowh,
                             bool growtofit);

  private:
  private:
    class ImageLoader {
      public:
        // never returns null... failed loads will simply be empty images
        std::shared_ptr<QImage> operator()(const QString &fullfilename);
    };

    desktop::LoadCache<QImage, ImageLoader> dm_cache;
};

#endif
