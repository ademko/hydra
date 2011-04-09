
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FlowLayout.h>

#include <assert.h>

#include <QWidget>
#include <QDebug>

using namespace desktop;

FlowLayout::FlowLayout(bool horizorder)
  : dm_horizorder(horizorder)
{
}

FlowLayout::~FlowLayout(void)
{
  //deleteAllItems();

  for (int i=0; i<dm_items.size(); ++i)
    delete dm_items[i];
}

bool FlowLayout::hasHeightForWidth () const
{
  return true;
}

int FlowLayout::heightForWidth(int w) const
{
  QSize boundingbox = computeLayout(QRect(0, 0, w, 0), false).size();

  return boundingbox.height();
}

void FlowLayout::addItem(QLayoutItem *item)
{
  dm_items.push_back(item);

  //qDebug() << __FUNCTION__ << dm_items.size();
}

QSize FlowLayout::sizeHint() const
{
  return minimumSize();
  /*
  QSize firstsize(100, 100);

  if (!dm_items.empty())
    firstsize = dm_items[0]->sizeHint();
  QSize boundingbox = computeLayout(QRect(QPoint(0,0), firstsize), false).size();

qDebug() << __FUNCTION__ << boundingbox;
  return boundingbox;*/
}

QSize FlowLayout::minimumSize() const
{
//return sizeHint();
  if (dm_items.empty())
    return QSize(100, 100);
  else
    return dm_items[0]->minimumSize();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
  if (index < 0 || index >= dm_items.size())
    return 0;

  return dm_items[index];
}

QLayoutItem *FlowLayout::takeAt(int index)
{
  if (index < 0 || index >= dm_items.size())
    return 0;

  QLayoutItem *item = dm_items[index];

  dm_items.erase(dm_items.begin() + index);

  return item;
}

void FlowLayout::setGeometry(const QRect &rect)
{
  QLayout::setGeometry(rect);

  //qDebug() << __FUNCTION__ << dm_items.size() << rect;
  QRect boundingbox = computeLayout(rect, true);
  //qDebug() << __FUNCTION__ << " afterset geometry" << geometry();

  dm_boundingBox = boundingbox;

  //qDebug() << " boundingbox" << boundingbox;

  //QLayout::setGeometry(boundingbox);

  //if (parentWidget() && !parentWidget()->geometry().contains(boundingbox))
    //parentWidget()->setGeometry(boundingbox);
}

void FlowLayout::insertWidget(int indexAt, QWidget *widget)
{
  addChildWidget(widget);

  dm_items.insert(dm_items.begin() + indexAt, new QWidgetItem(widget));

  //qDebug() << __FUNCTION__ << dm_items.size();
}

int & FlowLayout::fastPoint(QPoint &p) const
{
  if (dm_horizorder)
    return p.rx();
  else
    return p.ry();
}

int FlowLayout::fastPoint(const QPoint &p) const
{
  if (dm_horizorder)
    return p.x();
  else
    return p.y();
}

int & FlowLayout::slowPoint(QPoint &p) const
{
  if (dm_horizorder)
    return p.ry();
  else
    return p.rx();
}

int FlowLayout::slowPoint(const QPoint &p) const
{
  if (dm_horizorder)
    return p.y();
  else
    return p.x();
}

int FlowLayout::fastSize(const QSize &s) const
{
  if (dm_horizorder)
    return s.width();
  else
    return s.height();
}

int FlowLayout::slowSize(const QSize &s) const
{
  if (dm_horizorder)
    return s.height();
  else
    return s.width();
}

QRect FlowLayout::computeLayout(const QRect &rect, bool performSetGeometry) const
{
  QPoint nextp = rect.topLeft();
  int slowinc = 0;
  QRect boundingbox(rect);

  //qDebug() << "initial boundingbox" << boundingbox;
  for (int index=0; index<dm_items.size(); ++index) {
    QSize itemsizehint(dm_items[index]->sizeHint());
    QRect nextrect(nextp, itemsizehint);

    //qDebug() << " nextrect " << nextrect << " contains? " << rect.contains(nextrect, false);
    if (fastPoint(nextp) != fastPoint(rect.topLeft()) &&
        fastPoint(nextrect.bottomRight()) >= fastPoint(rect.bottomRight())
       ) {
      //qDebug() << " NEWLINE nextp=" << nextp << "rect.bottomRight" << rect.bottomRight();
      fastPoint(nextp) = fastPoint(rect.topLeft());
      slowPoint(nextp) += slowinc + spacing();

      nextrect = QRect(nextp, dm_items[index]->sizeHint());

      slowinc = 0;
    }
    //qDebug() << " nextrect " << nextrect;

    // place the widget here
    if (performSetGeometry)
      dm_items[index]->setGeometry(nextrect);

    if (nextrect.right() > boundingbox.right())
      boundingbox.setRight(nextrect.right());
    if (nextrect.bottom() > boundingbox.bottom())
      boundingbox.setBottom(nextrect.bottom());
    //qDebug() << "index" << index << "nextrect" << nextrect << "boundingbox" << boundingbox << "nextrect.x" << nextrect.topLeft().x();
    //qDebug() << " subset" << index << nextrect << "itemsizehint" << itemsizehint;

    if (slowSize(itemsizehint) > slowinc)
      slowinc = slowSize(itemsizehint);

    // increment
    fastPoint(nextp) += fastSize(itemsizehint) + spacing();

    //qDebug() << " nextp" << nextp;
  }//for

  return boundingbox;
}

