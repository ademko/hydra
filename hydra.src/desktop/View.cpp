
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/View.h>

#include <QContextMenuEvent>
#include <QMenu>

#include <desktop/MainWindow.h>
#include <hydra/Engine.h>

using namespace hydra;
;
using namespace desktop;

//
// FileListListener
//

FileListListener::FileListListener(FileList *_filelist)
    : dm_filelist(_filelist) {
    assert(dm_filelist);
    dm_filelist->addListener(this);
}

FileListListener::~FileListListener() {
    if (dm_filelist)
        dm_filelist->removeListener(this);
}

void FileListListener::resetFileList(void) { dm_filelist = 0; }

void FileListListener::onBaseChange(FileList *fl) {}

void FileListListener::onImageChange(FileList *fl, int fileIndex) {}

//
// View
//

View::View(FileList *_filelist)
    : FileListListener(_filelist), dm_mainwindow(0) {}

View::View(MainWindow *mainwin, FileList *_filelist)
    : FileListListener(_filelist), dm_mainwindow(mainwin) {}

View::~View() {}

/*moved this into MainImageView
void View::contextMenuEvent(QContextMenuEvent * event)
{
  QMenu menu(MainWindow::instance());

  MainWindow::instance()->buildMenuContext(menu, this);

  menu.exec(event->globalPos());
}
*/

void View::setTag(const QString &newtag) {
    assert(mainWindow());
    assert(fileList());

    if (newtag != "del")
        mainWindow()->setLastTag(newtag); // del is never the last tag

    QDateTime now(QDateTime::currentDateTime());

    QModelIndexList selected = fileList()->selectionModel()->selectedIndexes();

    for (QModelIndexList::const_iterator ii = selected.begin();
         ii != selected.end(); ++ii) {
        FileEntry &entry(fileList()->fileAt(*ii));
        FileItemRecord &item(entry.recordItem());

        Engine::instance()->regetFileItem(item);

        bool imageChanged = false;
        if (item.tags.insertTag(newtag, &imageChanged))
            Engine::instance()->saveFileItem(item, now);
        if (imageChanged) {
            // the image changed (rotate operation or something)
            // invalidate it, causing all its files to be reloaded
            entry.reloadRotateCode();
            fileList()->emitImageChange(ii->row(), this);
        }
    }

    fileList()->emitChangedTagsAll();
}

void View::unsetTag(const QString &newtag) {
    QModelIndexList selected = fileList()->selectionModel()->selectedIndexes();
    QDateTime now(QDateTime::currentDateTime());

    for (QModelIndexList::const_iterator ii = selected.begin();
         ii != selected.end(); ++ii) {
        FileEntry &entry(fileList()->fileAt(*ii));
        FileItemRecord &item(entry.recordItem());

        Engine::instance()->regetFileItem(item);

        bool imageChanged = false;
        if (item.tags.eraseTag(newtag, &imageChanged)) {
            Engine::instance()->saveFileItem(item, now);
            if (imageChanged) {
                // the image changed (rotate operation or something)
                // invalidate it, causing all its files to be reloaded
                entry.reloadRotateCode();
                fileList()->emitImageChange(ii->row(), this);
            }
        }
    }

    fileList()->emitChangedTagsAll();
}
