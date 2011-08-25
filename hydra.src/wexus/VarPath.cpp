
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/VarPath.h>

#include <wexus/AssertException.h>

using namespace wexus;

//
// asVariantMap
//

class AsMap : public QVariant
{
  public:
    QVariantMap * mapPtr(void);
    const QVariantMap * mapPtr(void) const;
};

template <typename T>
inline T * my_v_cast(QVariant::Private *d, T * = 0)
  {
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
         ? static_cast<T *>(d->data.shared->ptr)
 : static_cast<T *>(static_cast<void *>(&d->data.c)));
}

template <typename T>
inline T * my_v_cast(const QVariant::Private *d, T * = 0)
  {
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
         ? static_cast<T *>(d->data.shared->ptr)
 : static_cast<T *>(static_cast<const void *>(&d->data.c)));
}

QVariantMap * AsMap::mapPtr(void)
{
  if (type() != Map) {
    assertThrow(type() == Map);
    throw VarPath::MapRequiredException();
  }

  return my_v_cast<QVariantMap>(&d);
}

const QVariantMap * AsMap::mapPtr(void) const
{
  if (type() != Map) {
    assertThrow(type() == Map);
    throw VarPath::MapRequiredException();
  }

  return my_v_cast<const QVariantMap>(&d);
}

QVariantMap & wexus::asVariantMap(QVariant &v)
{
  return *reinterpret_cast<AsMap*>(&v)->mapPtr();
}

const QVariantMap & wexus::asVariantMap(const QVariant &v)
{
  return *reinterpret_cast<const AsMap*>(&v)->mapPtr();
}

QVariantMap wexus::key(const QString &k, const QVariant &v)
{
  QVariantMap r;

  r[k] = v;

  return r;
}

QVariantMap operator + (const QVariant &left, const QVariant &right)
{
  const QVariantMap & leftmap = asVariantMap(left);
  const QVariantMap & rightmap = asVariantMap(right);

  QVariantMap r(leftmap);

  for (QVariantMap::const_iterator ii=rightmap.begin(); ii != rightmap.end(); ++ii)
    r[ii.key()] = *ii;

  return r;
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
  if (dm_node->isValid()) {
    // make sure it doesnt already exist as a non-map
    if (dm_node->type() != QVariant::Map)
      throw MapRequiredException();
  } else
    (*dm_node) = QVariantMap();


  QVariant &ref = asVariantMap(*dm_node)[key];
  return VarPath(ref);
}

VarPath VarPath::operator ()(const QString &key)
{
  QVariantMap::iterator ii, endii;

  if (dm_map) {
    // map type
    ii = dm_map->find(key);
    endii = dm_map->end();
  } else {
    // node type
    if (dm_node->type() != QVariant::Map)
      throw MapRequiredException();

    QVariantMap & map = asVariantMap(*dm_node);

    ii = map.find(key);
    endii = map.end();
  }

  if (ii == endii)
    throw NotFoundException();

  return VarPath(*ii);
}

VarPath & VarPath::operator = (const QVariant &v)
{
  if (!dm_node)
    throw VariantRequiredException();

  (*dm_node) = v;

  return *this;
}

QVariant & VarPath::asVariant(void)
{
  if (!dm_node)
    throw VariantRequiredException();

  return *dm_node;
}

QVariantMap & VarPath::asMap(void)
{
  if (dm_map)
    return *dm_map;

  assertThrow(dm_node);

  if (dm_node->type() != QVariant::Map)
    throw MapRequiredException();

  QVariantMap & map = asVariantMap(*dm_node);

  return map;
}

