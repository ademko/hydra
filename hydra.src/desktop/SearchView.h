
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_SEARCHVIEW_H__
#define __INCLUDED_HYDRADESKTOP_SEARCHVIEW_H__

#include <QLineEdit>
#include <QHBoxLayout>

#include <hydra/Query.h>
#include <desktop/View.h>

namespace desktop
{
  class SearchView;
  class SearchSelectView;
}

/**
 * A view that presents a tag search box to the user, with four VCR
 * controls for advancing based on that search.
 *
 * @author Aleksander Demko
 */ 
class desktop::SearchView : public desktop::View
{
    Q_OBJECT

  public:
    SearchView(FileList *_filelist);

  protected slots:
    void onClear(void);
    void onFirst(void);
    void onPrev(void);
    void onNext(void);
    void onLast(void);

  protected:
    void initGui(void);
    bool verifyQuery(void);
    void search(int starti, int delta);

  protected:
    QLineEdit *dm_query;

    QString dm_last_query;
    std::tr1::shared_ptr<hydra::Token> dm_last_tok;  // null if the last query isnt valid

    QHBoxLayout *dm_buttonbox;
};

/**
 * Adds a "selected-by-tags" option to SearchView.
 *
 * @author Aleksander Demko
 */ 
class desktop::SearchSelectView : public desktop::SearchView
{
    Q_OBJECT

  public:
    SearchSelectView(FileList *_filelist);

  protected slots:
    void onHighlight(void);

  protected:
    void initGui(void);

};

#endif

