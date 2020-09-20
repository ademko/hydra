
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/DirView.h>

#include <QContextMenuEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include <desktop/FileOp.h>
#include <desktop/MainWindow.h>

using namespace desktop;

DirView::DirView(FileList *_filelist) : View(_filelist) {
    initGui();

    setWindowTitle("Folders");

    setMinimumSize(QSize(200, 10));
}

void DirView::onBaseChange(FileList *fl) {
    QString existingBase(
        dm_model.filePath(dm_tree->selectionModel()->currentIndex()));

    if (fl->baseDir() == existingBase)
        return;

    dm_tree->selectionModel()->setCurrentIndex(
        dm_model.index(fileList()->baseDir()),
        QItemSelectionModel::SelectCurrent);
}

void DirView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this); // MainWindow::instance());
    QAction *action;

    action = menu.addAction("Open folder");
    action->setProperty("id", 0);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuOpenDir()));

    action = menu.addAction("Open folder (and sub-folders)");
    action->setProperty("id", 1);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuOpenDir()));

    menu.addSeparator();

    action = menu.addAction("Refresh folder");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onRefreshDir()));
    action = menu.addAction("New folder...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onNewDir()));
    action = menu.addAction("Rename folder...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onRenameDir()));
    action = menu.addAction("Delete folder...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onDelDir()));

    menu.addSeparator();

    if (!fileList()->selectionModel()->selectedIndexes().isEmpty()) {
        action = menu.addAction("Copy selected files here");
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onCopySelection()));
        action = menu.addAction("Move selected files here");
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onMoveSelection()));
    }

    menu.exec(event->globalPos());
}

void DirView::onMenuOpenDir(void) {
    int id = sender()->property("id").toInt();

    QString newdir(
        dm_model.filePath(dm_tree->selectionModel()->currentIndex()));

    FileListLoader loader(*fileList(), this);

    loader.setBaseDir(newdir);
    loader.setBaseRecurse(id == 1);
}

void DirView::onActivated(const QModelIndex &i) {
    // qDebug() << __FUNCTION__ << dm_model.filePath(i);

    FileListLoader loader(*fileList(), this);

    loader.setBaseDir(dm_model.filePath(i));
    loader.setBaseRecurse(false);
}

void DirView::onRefreshDir(void) { dm_model.refresh(dm_tree->currentIndex()); }

void DirView::onNewDir(void) {
    bool ok = false;
    QString newname = QInputDialog::getText(this, "Create Folder",
                                            "Folder name:", QLineEdit::Normal,
                                            "Untitled", &ok);

    if (!ok)
        return;

    QDir d(dm_model.filePath(dm_tree->currentIndex()));
    d.mkdir(newname);

    // dm_model.mkdir(dm_tree->currentIndex(), newname);
    dm_model.refresh(dm_tree->currentIndex());
}

void DirView::onRenameDir(void) {
    QString curname = dm_model.fileName(dm_tree->currentIndex());
    bool ok = false;
    QString newname = QInputDialog::getText(
        this, "Rename Folder", "Folder name:", QLineEdit::Normal, curname, &ok);

    if (!ok || newname.isEmpty())
        return;

    QModelIndex parent(dm_tree->currentIndex().parent());

    QString oldfull = dm_model.filePath(dm_tree->currentIndex());
    // QString newfull = QFileInfo(oldfull).filePath() + "/" + newname;
    QDir parentdir(QFileInfo(oldfull).dir());

    // do the rename operation using the QDir api
    bool success = parentdir.rename(curname, newname);
    // bool success = FileOpDisk().moveFile(curname, newname);

    if (!success) {
        QMessageBox::critical(this, "Directory Rename Error",
                              "There was an error in renaming the directory.");
        return;
    }

    dm_model.refresh(parent);

    // reload the file list if it was the current one
    if (fileList()->baseDir() == oldfull) {
        FileListLoader reloader(*fileList(), this);

        QString newdir(parentdir.filePath(newname));
        reloader.setBaseDir(newdir);
    }
}

void DirView::onDelDir(void) {
    // can't use this, becase the model is readonly by default
    //(and I'd like to keep it that way)
    // bool success = dm_model.rmdir(dm_tree->currentIndex());

    bool success =
        FileOpDisk().delDir(dm_model.filePath(dm_tree->currentIndex()));

    if (success)
        dm_model.refresh(dm_tree->currentIndex().parent());
    else
        QMessageBox::critical(
            this, "Directory Delete Error",
            "There was an error in deleting the directory. Note, only empty "
            "directories can be deleted.");
}

void DirView::onCopySelection(void) { doFileOp(FileOp::Op_Copy); }

void DirView::onMoveSelection(void) { doFileOp(FileOp::Op_Move); }

void DirView::initGui(void) {
    QHBoxLayout *lay = new QHBoxLayout;

    dm_model.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

    dm_tree = new QTreeView;

    dm_tree->setModel(&dm_model);
    dm_tree->selectionModel()->setCurrentIndex(
        dm_model.index(fileList()->baseDir()),
        QItemSelectionModel::SelectCurrent);
    dm_tree->setExpandsOnDoubleClick(false);
    dm_tree->setHeaderHidden(true);

    for (int x = 1; x < dm_model.columnCount(); ++x)
        dm_tree->setColumnHidden(x, true);

    connect(dm_tree, SIGNAL(activated(const QModelIndex &)), this,
            SLOT(onActivated(const QModelIndex &)));

    lay->addWidget(dm_tree);

    // dm_model.select(//dm_tree->setRootIndex(dm_model.index(QDir::currentPath()));
    // dm_tree->setRootIndex(dm_model.index(QDir::currentPath()));

    setLayout(lay);
}

void DirView::doFileOp(int op) {
    QStringList list;
    QModelIndex i(dm_tree->currentIndex());

    if (!i.isValid())
        return;

    for (int i = 0; i < fileList()->numFiles(); ++i)
        if (fileList()->selectionModel()->isSelected(fileList()->index(i, 0)))
            list.push_back(fileList()->fileAt(i).fullfilename());

    guiFileOpFiles(this, op, list, dm_model.filePath(i));
}
