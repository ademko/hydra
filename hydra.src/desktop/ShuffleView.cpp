
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/ShuffleView.h>

#include <QHBoxLayout>

using namespace desktop;

ShuffleView::ShuffleView(FileList *_filelist) : View(_filelist) { initGui(); }

void ShuffleView::onToggle(int state) {
    if ((state != 0) == fileList()->isBaseShuffle())
        return;

    FileListLoader loader(*fileList());

    loader.setBaseShuffle(state != 0);
}

void ShuffleView::initGui(void) {
    QHBoxLayout *lay = new QHBoxLayout;

    dm_check = new QCheckBox("Randomly Shuffle Image List");
    dm_check->setChecked(fileList()->isBaseShuffle());

    lay->addWidget(dm_check);

    connect(dm_check, SIGNAL(stateChanged(int)), this, SLOT(onToggle(int)));

    setLayout(lay);
}
