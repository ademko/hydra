
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_QUERYVIEW_H__
#define __INCLUDED_HYDRADESKTOP_QUERYVIEW_H__

#include <QLineEdit>

#include <desktop/View.h>

namespace desktop
{
  class QueryView;
}

class desktop::QueryView : public desktop::View
{
    Q_OBJECT

  public:
    QueryView(FileList *_filelist);

  private slots:
    void onClear(void);
    void onEntry(void);

  private:
    void initGui(void);

  private:
    QLineEdit *dm_query;
};

#endif

