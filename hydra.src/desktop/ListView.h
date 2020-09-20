
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_LISTVIEW_H__
#define __INCLUDED_HYDRADESKTOP_LISTVIEW_H__

#include <QTableView>

#include <desktop/View.h>

namespace desktop {
class ListView;
}

class desktop::ListView : public desktop::View {
    Q_OBJECT

  public:
    ListView(MainWindow *mainwin, FileList *_filelist);

  protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);

  private:
    void initGui(void);

  private:
    QTableView *dm_table;
};

#endif
