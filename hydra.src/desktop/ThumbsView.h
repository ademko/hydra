
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_THUMBSVIEW_H__
#define __INCLUDED_HYDRADESKTOP_THUMBSVIEW_H__

#include <QAbstractListModel>
#include <QListView>
#include <QListWidget>

#include <hydra/Thumb.h>
#include <desktop/View.h>
#include <desktop/ThumbCache.h>

namespace desktop
{
  class ThumbsDelegate; // internal
  class ThumbsView;
}

// helper class for ThumbsView
class desktop::ThumbsDelegate : public QAbstractItemDelegate
{
  public:
    static const int PIC_W = hydra::Thumb::DEFAULT_THUMB_W/2;
    static const int PIC_H = hydra::Thumb::DEFAULT_THUMB_H/2;
    static const int OFFSET_W = 10;
    static const int OFFSET_H = 10;
    static const int FULL_W = PIC_W+OFFSET_W*2;
    static const int FULL_H = PIC_H+OFFSET_H*2+OFFSET_H*6;

  public:
    ThumbsDelegate(FileList *_filelist, ThreadedThumbCache *_thumbcache);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void clear(void);

    // called when an image is rotated, etc
    void onImageChange(FileList *fl, int fileIndex);
  private:
    FileList *dm_filelist;
    ThreadedThumbCache *dm_thumbcache;

    typedef std::map<QString, desktop::cache_ptr<QPixmap> > imagecache_t;
    // key is HASH, not filename
    mutable imagecache_t dm_images;
};

/**
 * A thumbnail veiwer.
 *
 * @author Aleksander Demko
 */ 
class desktop::ThumbsView : public desktop::View
{
    Q_OBJECT

  public:
    /**
     * Constructor.
     *
     * @param wrap_list if true, there will be wrapping (false = thumbstrip like mode)
     * @param popout_on_activate if nonnull, this MainWindow will be used to popout
     * a new view on activation
     * @author Aleksander Demko
     */ 
    ThumbsView(MainWindow *mainwin, FileList *_filelist, ThreadedThumbCache *_thumbcache, bool wrap_list,
        bool popout_on_activate = false);

    // called when an image is rotated, etc
    virtual void onImageChange(FileList *fl, int fileIndex);

  protected:
    virtual void contextMenuEvent(QContextMenuEvent * event);

  private slots:
    void onActivated(const QModelIndex &i);

  private:
    void initGui(bool wrap_list);

  private:
    ThreadedThumbCache *dm_thumbcache;

    QListView *dm_list;

    ThumbsDelegate dm_delegate;

    bool dm_popout_on_active;
};

#endif

