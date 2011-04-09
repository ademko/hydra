
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_SHUFFLEVIEW_H__
#define __INCLUDED_HYDRADESKTOP_SHUFFLEVIEW_H__

#include <QCheckBox>

#include <desktop/View.h>

namespace desktop
{
  class ShuffleView;
}

class desktop::ShuffleView : public desktop::View
{
    Q_OBJECT

  public:
    ShuffleView(FileList *_filelist);

  private slots:
    void onToggle(int state);

  private:
    void initGui(void);

  private:
    QCheckBox *dm_check;
};

#endif

