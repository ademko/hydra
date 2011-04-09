
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/TagSelectView.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QDebug>

using namespace desktop;

TagSelectView::TagSelectView(FileList *_filelist)
  : View(_filelist)
{
  initGui();

  setWindowTitle("Select by Tags");
}

void TagSelectView::onClear(void)
{
  dm_query->clear();

  onEntry();
}

void TagSelectView::onEntry(void)
{
  std::tr1::shared_ptr<hydra::Token> token;

  if (dm_query->text().isEmpty())
    return;

  if (!hydra::parseQueryTokens(dm_query->text(), token)) {
    dm_query->selectAll();
    return;
  }

  QItemSelection itemselection;

  queryToSelection(fileList(), token.get(), itemselection);

  fileList()->selectionModel()->select(itemselection, QItemSelectionModel::ClearAndSelect);
}

void TagSelectView::queryToSelection(FileList *fileList, hydra::Token *query, QItemSelection &outselection)
{
  bool expandingselection = false;
  int selectstart = -1;

  for (int x=0; x<fileList->numFiles(); ++x) {
    bool matches = query->isMatch(fileList->fileAt(x).recordItem().tags);

    if (matches && !expandingselection) {
      // start a new selection
      selectstart = x;
      expandingselection = true;
    }
    if (!matches && expandingselection) {
      // end the current selection and save it
      expandingselection = false;
      outselection.push_back(QItemSelectionRange(
            fileList->index(selectstart, 0),
            fileList->index(x-1, fileList->columnCount()-1)
            ));
    }
  }//for

  // see if there is one last selection
  if (expandingselection)
    outselection.push_back(QItemSelectionRange(
          fileList->index(selectstart, 0),
          fileList->index(fileList->numFiles()-1, fileList->columnCount()-1)
          ));
}

void TagSelectView::initGui(void)
{
  QHBoxLayout *lay = new QHBoxLayout;
  QPushButton *but;

  but = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), "Clear");

  dm_query = new QLineEdit(fileList()->baseQueryString());

  lay->addWidget(new QLabel("Tags:"));
  lay->addWidget(dm_query);
  lay->addWidget(but);

  connect(but, SIGNAL(clicked(bool)), this, SLOT(onClear()));
  connect(dm_query, SIGNAL(editingFinished()), this, SLOT(onEntry()));

  setLayout(lay);
}

