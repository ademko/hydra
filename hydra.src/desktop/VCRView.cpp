
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/VCRView.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>

#include <desktop/MainWindow.h>

using namespace desktop;

VCRView::VCRView(MainWindow *mainwin, FileList *_source)
  : View(mainwin, _source)
{
  initGui();
}

void VCRView::initGui(void)
{
  QHBoxLayout *lay = new QHBoxLayout;
  QPushButton *but;

  lay->addStretch();

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward), "First");
  connect(but, SIGNAL(clicked()), this, SLOT(onFirst()));
  lay->addWidget(but);
  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward), "Previous");
  connect(but, SIGNAL(clicked()), this, SLOT(onPrev()));
  lay->addWidget(but);
  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward), "Next");
  connect(but, SIGNAL(clicked()), this, SLOT(onNext()));
  lay->addWidget(but);
  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward), "Last");
  connect(but, SIGNAL(clicked()), this, SLOT(onLast()));
  lay->addWidget(but);

  lay->addStretch();

  setLayout(lay);
}

void VCRView::onFirst(void)
{
  fileList()->setSelectedFile(0);
}

void VCRView::onPrev(void)
{
  fileList()->setSelectedFile(fileList()->selectedFileIndex()-1);
}

void VCRView::onNext(void)
{
  //fileList()->setSelectedFile(fileList()->selectedFileIndex()+1);
  mainWindow()->advanceAndPreloadSelectedFile();
}

void VCRView::onLast(void)
{
  fileList()->setSelectedFile(fileList()->numFiles() - 1);
}

