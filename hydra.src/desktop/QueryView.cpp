
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/QueryView.h>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

using namespace desktop;

QueryView::QueryView(FileList *_filelist) : View(_filelist) {
    initGui();

    setWindowTitle("Tag Filter");
}

void QueryView::onClear(void) {
    dm_query->clear();

    onEntry();
}

void QueryView::onEntry(void) {
    // performance optimization
    if (dm_query->text() == fileList()->baseQueryString())
        return;

    FileListLoader loader(*fileList());

    if (!loader.setBaseQuery(dm_query->text()))
        dm_query->selectAll();
}

void QueryView::initGui(void) {
    QHBoxLayout *lay = new QHBoxLayout;
    QPushButton *but;

    but = new QPushButton(
        QApplication::style()->standardIcon(QStyle::SP_TrashIcon), "Clear");

    dm_query = new QLineEdit(fileList()->baseQueryString());

    lay->addWidget(new QLabel("Tags:"));
    lay->addWidget(dm_query);
    lay->addWidget(but);

    connect(but, SIGNAL(clicked(bool)), this, SLOT(onClear()));
    connect(dm_query, SIGNAL(editingFinished()), this, SLOT(onEntry()));

    setLayout(lay);
}
