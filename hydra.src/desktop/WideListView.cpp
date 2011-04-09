
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/WideListView.h>

#include <QHBoxLayout>

using namespace desktop;

WideListView::WideListView(FileList *_filelist)
  : View(_filelist)
{
  initGui();
}

void WideListView::initGui()
{
  QHBoxLayout *lay = new QHBoxLayout;

  dm_list = new QListView(this);
  dm_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

  dm_list->setUniformItemSizes(true);
  dm_list->setFlow(QListView::LeftToRight);
  dm_list->setWrapping(true);
  dm_list->setResizeMode(QListView::Adjust);

  dm_list->setModel(fileList());
  dm_list->setSelectionModel(fileList()->selectionModel());

  lay->addWidget(dm_list);

  setLayout(lay);
}

