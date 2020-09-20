
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_VCRVIEW_H__
#define __INCLUDED_HYDRADESKTOP_VCRVIEW_H__

#include <desktop/View.h>

namespace desktop {
class VCRView;
}

class desktop::VCRView : public desktop::View {
    Q_OBJECT
  public:
    VCRView(MainWindow *mainwin, FileList *_source);

  private slots:
    void onFirst(void);
    void onPrev(void);
    void onNext(void);
    void onLast(void);

  private:
    void initGui(void);
};

#endif
