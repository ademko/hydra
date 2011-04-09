
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_WIDELISTVIEW_H__
#define __INCLUDED_HYDRADESKTOP_WIDELISTVIEW_H__

#include <QListView>

#include <desktop/View.h>

namespace desktop
{
  class WideListView;
}

class desktop::WideListView : public desktop::View
{
    Q_OBJECT

  public:
    WideListView(FileList *_filelist);

  private:
    void initGui(void);

  private:
    QListView *dm_list;
};

#endif

