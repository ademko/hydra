
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/VarPath.h>

#include <assert.h>

using namespace wexus;

//
// asVariantMap
//

class AsMap : public QVariant
{
  public:
    QVariantMap * mapPtr(void);
};

template <typename T>
inline T * my_v_cast(QVariant::Private *d, T * = 0)
  {
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
         ? static_cast<T *>(d->data.shared->ptr)
 : static_cast<T *>(static_cast<void *>(&d->data.c)));
}

QVariantMap * AsMap::mapPtr(void)
{
  assert(type() == Map);

  //QVariant::Private *my_d = &d;

  return my_v_cast<QVariantMap>(&d);
}
QVariantMap & wexus::asVariantMap(QVariant &v)
{
  return *reinterpret_cast<AsMap*>(&v)->mapPtr();
}

//
//
// VarPath
//
//

VarPath::VarPath(QVariantMap &srcmap)
  : dm_map(&srcmap), dm_node(0)
{
}
    
VarPath::VarPath(QVariant &v)
  : dm_map(0), dm_node(&v)
{
}

VarPath VarPath::operator [](const QString &key)
{
  if (dm_map) {
    // we are a map
    QVariant &ref = (*dm_map)[key];
    return VarPath(ref);
  }

  // not the root node

  // create a map if this is a new subnode
  if (!dm_node->isValid())
    (*dm_node) = QVariantMap();

  // make sure it doesnt already exist as a non-map
  if (dm_node->type() != QVariant::Map)
    throw TypeException();

  QVariant &ref = asVariantMap(*dm_node)[key];
  return VarPath(ref);
}

// TODO to implement this function
// we need to fully implement a ConstVarPath class
// to shadow VarPath, but with const reference

/*VarPath VarPath::operator ()(const QString &key) const
{
  QVariantMap::const_iterator ii, endii;

  if (dm_map) {
    // map type
    ii = dm_map->find(key);
    endii = dm_map->end();
  } else {
    // node type
  if (dm_node->type() == QVariant::Map)
    if (dm_node->type() != QVariant::Map)
      throw TypeException();
    const QVariantMap & map = asVariantMap(*dm_node);

    ii = map.find(key);
    endii = map.end();
  }

  if (ii == endii)
    throw NotFoundException();

  // we probably need a ConstVarPath to finish this method

  return ConstVarPath(*ii);
}*/

VarPath & VarPath::operator = (const QVariant &v)
{
  if (!dm_node)
    throw Exception();

  (*dm_node) = v;

  return *this;
}

QVariant & VarPath::asVariant(void)
{
  if (!dm_node)
    throw Exception();

  return *dm_node;
}

QVariantMap & VarPath::asMap(void)
{
  if (!dm_map)
    throw Exception();

  return *dm_map;
}

