
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_VIEW_H__
#define __INCLUDED_HYDRADESKTOP_VIEW_H__

#include <QWidget>
#include <QAbstractItemView>

#include <desktop/FileList.h>

namespace desktop
{
  class MainWindow; //fwd

  class FileListListener;
  class View;
}

class desktop::FileListListener
{
  public:
    FileListListener(FileList *_filelist);
    virtual ~FileListListener();

    /// sometimes called by FileList's destructor (doesn't modify the FileList listener list)
    void resetFileList(void);

    // may be null, in dtor phases
    FileList * fileList(void) const { return dm_filelist; }

    /**
     * Called when the base changes of the filelist.
     *
     * This handler does nothing by default.
     *
     * @author Aleksander Demko
     */ 
    virtual void onBaseChange(FileList *fl);

    /**
     * Called when the image itself changes
     * for the give file entry.
     * For now, this is usaully the result of a rotate operation.
     *
     * This handler does nothing by default.
     *
     * @author Aleksander Demko
     */ 
    virtual void onImageChange(FileList *fl, int fileIndex);

  private:
    FileList * dm_filelist;
};

/**
 * A View is a graphical (QWidget) compoenent that can monitor a FileList.
 *
 * @author Aleksander Demko
 */ 
class desktop::View : public QWidget, public FileListListener
{
  public:
    /// constructor
    View(FileList *_filelist);
    /// newer constructor, if the mainwin pointer needs to be retained
    /// if mainwin is null, this acts like the first ctor
    View(MainWindow *mainwin, FileList *_filelist);
    /// destrctor
    virtual ~View();

    // may be null, if one wasn't supplied in the ctor
    MainWindow *mainWindow(void) const { return dm_mainwindow; }

  protected:
    //virtual void contextMenuEvent(QContextMenuEvent * event);

    // used by tag-changing views, for convience
    // origianlly from TagEditorView2
    void setTag(const QString &newtag);
    // used by tag-changing views, for convience
    // origianlly from TagEditorView2
    void unsetTag(const QString &newtag);

  private:
    MainWindow *dm_mainwindow;
};

#endif

