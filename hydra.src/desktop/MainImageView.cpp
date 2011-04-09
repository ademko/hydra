
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/MainImageView.h>

#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QDebug>

#include <hydra/RotateCode.h>
#include <desktop/MainWindow.h>

using namespace desktop;

MainImageView::MainImageView(MainWindow *mainwin, FileList *_filelist, ImageCache *_loader)
  : View(mainwin, _filelist), dm_loader(_loader)
{
  dm_timerid = 0;
  dm_dirty = true;
  dm_fullscreenmode = false;

  setMinimumSize(320, 200);
  setFocusPolicy(Qt::StrongFocus);

  connect(_filelist->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
      this, SLOT(onCurrentChanged()));
  connect(_filelist, SIGNAL(modelReset()), this, SLOT(onCurrentChanged()));

  setFocusPolicy(Qt::ClickFocus);
}

void MainImageView::onImageChange(FileList *fl, int fileIndex)
{
  if (fileIndex != fl->selectedFileIndex())
    return;

  dm_dirty = true;

  update();
}

void MainImageView::showFullScreenMode(void)
{
  dm_fullscreenmode = true;
  dm_dirty = true;

  setWindowState(windowState() | Qt::WindowFullScreen);
  show();
  setFocus();
}

void MainImageView::resizeEvent(QResizeEvent *event)
{
  dm_dirty = true;
}

void MainImageView::paintEvent(QPaintEvent *event)
{
  QPainter dc(this);

  if (dm_dirty) {
    dm_dirty = false;

    if (fileList()->isValid()) {
      dm_image = dm_loader->getImage(fileList()->selectedFile().fullfilename());

      if (dm_image->isNull())
        dm_pixmap = QPixmap();
      else {
        // rotate as needed
        QImage rotimage = hydra::rotateImageByCode(fileList()->selectedFile().rotateCode(), *dm_image);
        dm_pixmap = ImageCache::getPixmap(rotimage, dc.window().width(), dc.window().height(), true);
      }
    }
  }//if dm_dirty

  if (dm_fullscreenmode)
    dc.setBackground(Qt::black);

  dc.eraseRect(dc.window());

  if (dm_pixmap.isNull()) {
    //dc.setPen(Qt::red);
    //dc.drawText(20, 20, "(error loading image)");
  } else
    dc.drawPixmap((dc.window().width()-dm_pixmap.width())/2, (dc.window().height()-dm_pixmap.height())/2, dm_pixmap.width(), dm_pixmap.height(), dm_pixmap);
}

void MainImageView::mousePressEvent(QMouseEvent * event)
{
  if (event->button() == Qt::LeftButton && !dm_timerid)
    dm_timerid = startTimer(200);
  View::mousePressEvent(event);
}

void MainImageView::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    if (dm_timerid) {
      killTimer(dm_timerid);
      dm_timerid = 0;
    }
    mainWindow()->toggleFullScreen();
  } else
    View::mouseDoubleClickEvent(event);
}

void MainImageView::onCurrentChanged(void)
{
  dm_dirty = true;

  update();
}

void MainImageView::timerEvent(QTimerEvent * event)
{
  killTimer(event->timerId());

  if (!dm_timerid || dm_timerid != event->timerId())
    return;

  dm_timerid = 0;

  // single click action
  //fileList()->setSelectedFile(fileList()->selectedFileIndex()+1);
  mainWindow()->advanceAndPreloadSelectedFile();
}

void MainImageView::keyPressEvent(QKeyEvent * event)
{
  if (isFullScreen() && event->modifiers() == Qt::CTRL && event->key() == Qt::Key_F) {
    // bug fix:
    // ctrl-f accelrator does not work when MainImageView is full screen and thus its own
    // top level window. need to do ctrl f handling directly
    mainWindow()->toggleFullScreen();
  }
  
  if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_Insert) {
    unsetTag(mainWindow()->lastTag());
    return;
  }
  if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_Delete) {
    unsetTag("del");
    return;
  }

  if (event->modifiers() != Qt::NoModifier) {
    View::keyPressEvent(event);
    return;
  }

  if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Down)
    mainWindow()->advanceAndPreloadSelectedFile();
  else if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Up) {
    fileList()->setSelectedFile(fileList()->selectedFileIndex()-1);
  } else if (event->key() == Qt::Key_Insert) {
    setTag(mainWindow()->lastTag());
  } else if (event->key() == Qt::Key_Delete) {
    setTag("del");
  } else
    View::keyPressEvent(event);
}

void MainImageView::contextMenuEvent(QContextMenuEvent * event)
{
  QMenu menu(mainWindow());

  mainWindow()->buildMenuImage(menu);

  menu.exec(event->globalPos());
}

