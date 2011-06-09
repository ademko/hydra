
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/ThumbsView.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include <hydra/Thumb.h>
#include <hydra/RotateCode.h>
#include <desktop/MainWindow.h>

using namespace desktop;

//
//
// ThumbsDelegate
//
//

ThumbsDelegate::ThumbsDelegate(FileList *_filelist, ThreadedThumbCache *_thumbcache)
  : dm_filelist(_filelist), dm_thumbcache(_thumbcache)
{
}

void ThumbsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  //QString fullfilename(index.model()->data(index, Qt::DisplayRole).toString());
  desktop::cache_ptr<QPixmap> pix;
  FileEntry &entry(dm_filelist->fileAt(index.row()));

  //qDebug() << option.rect << index.model()->data(index, Qt::DisplayRole).toString();

  // draw title and selection rectangle
  painter->eraseRect(option.rect);

  painter->drawText(
      option.rect.x(),
      option.rect.y()+OFFSET_H*2+PIC_H,
      option.rect.width(),
      OFFSET_H*2,
      Qt::AlignTop|Qt::AlignHCenter,
      entry.justname());

  if (option.state & QStyle::State_Selected) {
    painter->setPen(QPen(QBrush(Qt::black), 4));
    painter->drawRect(option.rect.x()+2, option.rect.y()+2, FULL_W-4, FULL_H-4);
  }

  // draw tag list
  QString tags;
  bool needsBigRead = false;

  entry.containsRecord(&needsBigRead);

  if (needsBigRead) {
    dm_filelist->enqueueLoad(entry.fullfilename(), index.row());
    tags = "loading...";
  } else
    for (hydra::FileItemRecord::tags_t::const_iterator ii=entry.recordItem().tags.begin();
        ii != entry.recordItem().tags.end(); ++ii) {
      QTextStream out(&tags);
      if (!tags.isEmpty())
        out << ' ';
      out << *ii;
    }

  if (!tags.isEmpty())
    painter->drawText(
        option.rect.x(),
        option.rect.y()+OFFSET_H*4+PIC_H,
        option.rect.width(),
        OFFSET_H*2,
        Qt::AlignTop|Qt::AlignHCenter,
        tags);

  if (needsBigRead)
    return;

  // draw thumbnail

  //qDebug() << index.row() << entry.fullfilename() << entry.hash();

  if (dm_images.count(entry.recordHash()) == 0) {
    // load the image from the thumbnail cache
    pix = dm_thumbcache->getPixmap(entry.fullfilename(), entry.recordHash(),
        entry.rotateCode(),
        hydra::Thumb::DEFAULT_THUMB_W, hydra::Thumb::DEFAULT_THUMB_H,
        dm_filelist, index.row());
    if (pix.get())
      dm_images[entry.recordHash()] = pix;
  } else
    pix = dm_images[entry.recordHash()];

  //painter->drawText(option.rect.x()+0, option.rect.y()+10, index.model()->data(index, Qt::DisplayRole).toString());
  //if ((option.state & QStyle::State_Selected) != 0)
    //painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);

  if (pix.get())
    painter->drawPixmap(
        option.rect.x()+OFFSET_W/2 + (PIC_W-pix->width()/2)/2, option.rect.y()+OFFSET_H/2 + (PIC_H-pix->height()/2)/2,
        pix->width()/2, pix->height()/2,
        *pix);
}

QSize ThumbsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  //qDebug() << __FUNCTION__;
  return QSize(FULL_W, FULL_H);
}

void ThumbsDelegate::clear(void)
{
  dm_images.clear();
}

void ThumbsDelegate::onImageChange(FileList *fl, int fileIndex)
{
  imagecache_t::iterator ii = dm_images.find(fl->fileAt(fileIndex).recordHash());
  if (ii != dm_images.end())
    dm_images.erase(ii);
}

//
//
// ThumbsView
//
//

ThumbsView::ThumbsView(MainWindow *mainwin, FileList *_filelist, ThreadedThumbCache *_thumbcache, bool wrap_list,
        bool popout_on_activate)
  : View(mainwin, _filelist), dm_thumbcache(_thumbcache),
    dm_delegate(_filelist, _thumbcache)
{
  //setMinimumHeight(ThumbsDelegate::PIC_H+ThumbsDelegate::PADDING_H);

  initGui(wrap_list);

  setWindowTitle("Thumbnails");

  dm_popout_on_active = popout_on_activate;

  //if (popout_on_activate)
  //do this all the time, it's nice
    connect(dm_list, SIGNAL(activated(const QModelIndex&)), this, SLOT(onActivated(const QModelIndex&)));
}

void ThumbsView::onImageChange(FileList *fl, int fileIndex)
{
  dm_delegate.onImageChange(fl, fileIndex);
  update();
}

void ThumbsView::contextMenuEvent(QContextMenuEvent * event)
{
  QMenu menu(mainWindow());

  mainWindow()->buildMenuSelection(menu);

  menu.exec(event->globalPos());
}

void ThumbsView::onActivated(const QModelIndex &i)
{
  if (!dm_popout_on_active)
    return;
  assert(mainWindow());

  MainWindow *newmain = new MainWindow(MainWindow::VCR_VIEW, mainWindow()->mainStatePtr());

  // maximize it
  newmain->setWindowState(newmain->windowState() ^ Qt::WindowMaximized);
  newmain->show();
}

/**
 * A hack. In thumbnail view, the scroll wheel seems to go to far.
 * This enhanced listview simply intercepts the mouse wheel events
 * and halves their delta values.
 *
 * @author Aleksander Demko
 */ 
class MyListView : public QListView
{
  public:
    MyListView(QWidget *parent);

  protected:
    virtual void wheelEvent(QWheelEvent *e);
};

MyListView::MyListView(QWidget *parent)
  : QListView(parent)
{
}

void MyListView::wheelEvent(QWheelEvent *e)
{
  //qDebug() << __FUNCTION__ << e->delta();

  // half the delta
  QWheelEvent newevent(e->pos(), e->globalPos(), e->delta()/2, e->buttons(), e->modifiers(), e->orientation());

  QListView::wheelEvent(&newevent);
}

void ThumbsView::initGui(bool wrap_list)
{
  QHBoxLayout *lay = new QHBoxLayout;

  dm_list = new MyListView(this);
  //dm_list->setViewMode(QListView::IconMode);
  dm_list->setFlow(QListView::LeftToRight);
  //dm_list->setMovement(QListView::Snap);
  if (wrap_list)
    dm_list->setWrapping(true);
  if (wrap_list)
    dm_list->setSelectionRectVisible(true);
  dm_list->setUniformItemSizes(true);
  dm_list->setResizeMode(QListView::Adjust);
  dm_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

  dm_list->setItemDelegate(&dm_delegate);
  dm_list->setModel(fileList());
  dm_list->setSelectionModel(fileList()->selectionModel());

  lay->addWidget(dm_list);

  setLayout(lay);
}

