
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_TAGEDITOR_H__
#define __INCLUDED_HYDRADESKTOP_TAGEDITOR_H__

#include <desktop/FlowLayout.h>
#include <desktop/View.h>

#include <QCheckBox>

#include <ui_TagEditorView.h>

namespace desktop {
class TagEditorView;
}

class desktop::TagEditorView : public desktop::View,
                               public Ui_TagEditorDialog2 {
    Q_OBJECT

  public:
    TagEditorView(MainWindow *mainwin, FileList *_filelist);

    virtual void onImageChange(FileList *fl, int fileIndex);

  public slots:
    void updateEditor(void);

  private slots:
    void onAdd(void);
    void onClear(void);
    void onCheckBox(bool state);

    void onDataChanged(void);

  private:
    void initGui(void);

  private:
    QWidget *dm_containerWidget;
    FlowLayout *dm_containerLayout;

    std::set<QString> dm_recently_added;

    typedef std::map<QString, QCheckBox *> boxes_t;
    boxes_t dm_boxes;
};

#endif
