
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_INFOVIEW_H__
#define __INCLUDED_HYDRADESKTOP_INFOVIEW_H__

#include <desktop/ImageCache.h>
#include <desktop/View.h>

#include <QLabel>

namespace desktop {
class InfoView;
}

class desktop::InfoView : public desktop::View {
    Q_OBJECT
  public:
    InfoView(FileList *_filelist, ImageCache *_loader);

  private slots:
    void onCurrentChanged(void);

  private:
    ImageCache *dm_loader; // needed for dimen info

    QLabel *dm_label;
};

#endif
