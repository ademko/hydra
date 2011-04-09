
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/SearchView.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QDebug>

#include <desktop/TagSelectView.h>

using namespace desktop;

SearchView::SearchView(FileList *_filelist)
  : View(_filelist), dm_last_query("not")   // init dm_last_query to something invalid so that it gets reset right away
{
  initGui();

  setWindowTitle("Search by Tags");
}

void SearchView::onClear(void)
{
  dm_query->clear();
}

void SearchView::onFirst(void)
{
  search(0, 1);
}

void SearchView::onPrev(void)
{
  search(fileList()->selectedFileIndex() - 1, -1);
}

void SearchView::onNext(void)
{
  search(fileList()->selectedFileIndex() + 1, 1);
}

void SearchView::onLast(void)
{
  search(fileList()->numFiles() - 1, -1);
}

void SearchView::initGui(void)
{
  QVBoxLayout *vlay = new QVBoxLayout;
  QHBoxLayout *lay;
  QPushButton *but;

  lay = new QHBoxLayout;
  dm_query = new QLineEdit(fileList()->baseQueryString());

  connect(dm_query, SIGNAL(returnPressed()), this, SLOT(onNext()));

  lay->addWidget(new QLabel("Tag search:"));
  lay->addWidget(dm_query);
  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), "Clear");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onClear()));
  lay->addWidget(but);

  vlay->addLayout(lay);

  lay = new QHBoxLayout;

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward), "First");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onFirst()));
  lay->addWidget(but);

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward), "Previous");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onPrev()));
  lay->addWidget(but);

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward), "Next");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onNext()));
  lay->addWidget(but);

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward), "Last");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onLast()));
  lay->addWidget(but);

  dm_buttonbox = lay;

  vlay->addLayout(lay);

  setLayout(vlay);
}

bool SearchView::verifyQuery(void)
{
  if (dm_query->text() != dm_last_query) {
    dm_last_query = dm_query->text();

    if (!hydra::parseQueryTokens(dm_last_query, dm_last_tok))
      dm_last_tok.reset();
  }

  if (dm_last_tok.get() == 0)
    return false;

  return true;
}

void SearchView::search(int starti, int delta)
{
  if (!fileList()->isValid(starti))
    return;

  if (!verifyQuery())
    return;

  if (starti < 0)
    starti = fileList()->numFiles() - 1;
  else
    if (starti >= fileList()->numFiles())
      starti = 0;

  // find the next entry
  int curi = starti;

  while (true) {
    if (dm_last_tok->isMatch(fileList()->fileAt(curi).recordItem().tags)) {
      fileList()->setSelectedFile(curi);
      return;
    }

    curi += delta;

    if (curi < 0)
      curi = fileList()->numFiles() - 1;
    else
      if (curi >= fileList()->numFiles())
        curi = 0;

    if (curi == starti)
      break;
  }//while
}

SearchSelectView::SearchSelectView(FileList *_filelist)
  : SearchView(_filelist)
{
  initGui();
}

void SearchSelectView::onHighlight(void)
{
  if (!verifyQuery())
    return;

  QItemSelection itemselection;

  TagSelectView::queryToSelection(fileList(), dm_last_tok.get(), itemselection);

  fileList()->selectionModel()->select(itemselection, QItemSelectionModel::ClearAndSelect);
}

void SearchSelectView::initGui(void)
{
  assert(dm_buttonbox);

  QPushButton *but;

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_FileDialogListView), "Select by Tags");
  connect(but, SIGNAL(clicked(bool)), this, SLOT(onHighlight()));
  dm_buttonbox->addWidget(but);
}

