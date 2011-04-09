
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_MAINIMAGEVIEW_H__
#define __INCLUDED_HYDRADESKTOP_MAINIMAGEVIEW_H__

#include <desktop/View.h>
#include <desktop/ImageCache.h>

namespace desktop
{
  class MainWindow; // forward

  class MainImageView;
}

class desktop::MainImageView : public desktop::View
{
    Q_OBJECT
  public:
    /// constructor
    MainImageView(MainWindow *mainwin, FileList *_filelist, ImageCache *_loader);

    virtual void onImageChange(FileList *fl, int fileIndex);

    void showFullScreenMode(void);

  protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void timerEvent(QTimerEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void contextMenuEvent(QContextMenuEvent * event);

  private slots:
    void onCurrentChanged(void);

  private:
    ImageCache *dm_loader;

    // used for double click detection
    int dm_timerid; // 0 for none

    bool dm_dirty;
    bool dm_fullscreenmode;

    desktop::cache_ptr<QImage> dm_image;
    QPixmap dm_pixmap;
};

#endif

