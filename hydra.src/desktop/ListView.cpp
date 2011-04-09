
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/ListView.h>

#include <QHBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include <desktop/MainWindow.h>

using namespace desktop;

ListView::ListView(MainWindow *mainwin, FileList *_filelist)
  : View(mainwin, _filelist)
{
  initGui();

  setWindowTitle("File List");
}

void ListView::contextMenuEvent(QContextMenuEvent * event)
{
  QMenu menu(mainWindow());

  mainWindow()->buildMenuSelection(menu);

  menu.exec(event->globalPos());
}

void ListView::initGui(void)
{
  QHBoxLayout *lay = new QHBoxLayout;

  dm_table = new QTableView(this);
  dm_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  dm_table->setSelectionBehavior(QTableView::SelectRows);

  // setSortingEnabled must be called here, aparently before the setModel
  // stuff, otherwise a weird bug surfaces: the current selection
  // gets reset when new ListViews are instantiated
  dm_table->setSortingEnabled(true);

  dm_table->setModel(fileList());
  dm_table->setSelectionModel(fileList()->selectionModel());

  dm_table->setColumnWidth(0, 150);
  dm_table->setColumnWidth(1, 100);
  dm_table->setColumnWidth(2, 150);
  dm_table->setColumnWidth(3, 50);
  dm_table->setColumnWidth(4, 300);

  //dm_table->setSortingEnabled(true);

  lay->addWidget(dm_table);

  setLayout(lay);
}

