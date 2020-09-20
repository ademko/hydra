
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_TAGSELECTVIEW_H__
#define __INCLUDED_HYDRADESKTOP_TAGSELECTVIEW_H__

#include <QLineEdit>

#include <desktop/View.h>

namespace desktop {
class TagSelectView;
}

class desktop::TagSelectView : public desktop::View {
    Q_OBJECT

  public:
    TagSelectView(FileList *_filelist);

    static void queryToSelection(FileList *fileList, hydra::Token *query,
                                 QItemSelection &outselection);

  private slots:
    void onClear(void);
    void onEntry(void);

  private:
    void initGui(void);

  private:
    QLineEdit *dm_query;
};

#endif
