
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FLOWLAYOUT_H__
#define __INCLUDED_HYDRADESKTOP_FLOWLAYOUT_H__

#include <vector>

#include <QLayout>

namespace desktop
{
  class FlowLayout;
}

class desktop::FlowLayout : public QLayout
{
  public:
    FlowLayout(bool horizorder);
    virtual ~FlowLayout();

    virtual bool hasHeightForWidth(void) const;
    virtual int heightForWidth(int w) const;

    virtual int count(void) const { return static_cast<int>(dm_items.size()); }
    virtual void addItem(QLayoutItem *item);
    virtual QSize sizeHint() const;
    virtual QSize minimumSize() const;
    virtual QLayoutItem *itemAt(int index) const;
    virtual QLayoutItem *takeAt(int index);
    virtual void setGeometry(const QRect &rect);

    void insertWidget(int indexAt, QWidget *widget);

    QRect boundingBox(void) const { return dm_boundingBox; }

  private:
    int & fastPoint(QPoint &p) const;
    int fastPoint(const QPoint &p) const;
    int & slowPoint(QPoint &p) const;
    int slowPoint(const QPoint &p) const;
    int fastSize(const QSize &s) const;
    int slowSize(const QSize &s) const;

    QRect computeLayout(const QRect &rect, bool performSetGeometry) const;

  private:
    bool dm_horizorder;

    std::vector<QLayoutItem*> dm_items;

    QRect dm_boundingBox;
};

#endif

