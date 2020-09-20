
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_DIRVIEW_H__
#define __INCLUDED_HYDRADESKTOP_DIRVIEW_H__

#include <QDirModel>
#include <QTreeView>

#include <desktop/View.h>

namespace desktop {
class DirView;
}

class desktop::DirView : public desktop::View {
    Q_OBJECT

  public:
    DirView(FileList *_filelist);

    virtual void onBaseChange(FileList *fl);

  protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);

  private slots:
    void onMenuOpenDir(void);
    void onActivated(const QModelIndex &i);
    void onRefreshDir(void);
    void onNewDir(void);
    void onRenameDir(void);
    void onDelDir(void);
    void onCopySelection(void);
    void onMoveSelection(void);

  private:
    void initGui(void);
    void doFileOp(int op);

  private:
    QDirModel dm_model;
    QTreeView *dm_tree;
};

#endif
