
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/MainWindow.h>

#include <QMessageBox>
#include <QInputDialog>

#include <desktop/FileOp.h>
#include <desktop/TagSelectView.h>

using namespace desktop;

class SelectAllPlugin : public desktop::MainPlugin
{
  public:
    virtual QString description(void) const { return "Select All"; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList *filelist)
    {
      filelist->selectionModel()->select(QItemSelection(filelist->index(0,0), filelist->index(filelist->rowCount()-1,filelist->columnCount()-1)),QItemSelectionModel::Select);
    }
};

static hydra::Register<SelectAllPlugin> reg00;//desktop::MainPlugin::registry);

class DeletePlugin : public desktop::MainPlugin
{
  public:
    virtual QString description(void) const { return "Delete Selected Files"; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList *filelist);
};

static hydra::Register<DeletePlugin> reg04;

void DeletePlugin::go(desktop::MainWindow *mainwindow, desktop::FileList *filelist)
{
  QStringList list;

  for (int i=0; i<filelist->numFiles(); ++i)
    if (filelist->selectionModel()->isSelected(filelist->index(i,0)))
      list.push_back(filelist->fileAt(i).fullfilename());

  if (QMessageBox::question(mainwindow, "Comfirm Delete",
        "Are you sure you want to delete " + QString::number(list.size()) + " selected files?",
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)
      == QMessageBox::Cancel)
    return;

  guiFileOpFiles(mainwindow, FileOp::Op_Del, list, "");
}

class ReloadPlugin : public desktop::MainPlugin
{
  public:
    virtual QString description(void) const { return "Reload List"; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList *filelist)
    {
      FileListLoader loader(*filelist);

      loader.setReload();
    }
};

static hydra::Register<ReloadPlugin> reg05;

class SelectByTagsPlugin : public desktop::MainPlugin
{
  public:
    virtual QString description(void) const { return "Select by Tags"; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList *filelist);
};

static hydra::Register<SelectByTagsPlugin> reg04_23948xx;

void SelectByTagsPlugin::go(desktop::MainWindow *mainwindow, desktop::FileList *filelist)
{
  QString query_string;
  std::tr1::shared_ptr<hydra::Token> query_token;

  while (true) {
    query_string = QInputDialog::getText(mainwindow,
        "Select by Tags", "Tags:", QLineEdit::Normal, query_string);

    if (query_string.isEmpty())
      return;

    if (hydra::parseQueryTokens(query_string, query_token))
      break;
  }

  QItemSelection itemselection;

  TagSelectView::queryToSelection(filelist, query_token.get(), itemselection);

  filelist->selectionModel()->select(itemselection, QItemSelectionModel::ClearAndSelect);
}

