
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MimeTypes.h>

#include <assert.h>

#include <QStringList>

using namespace wexus;

MimeTypes::MimeTypesException::MimeTypesException(const QString &usermsg)
  : HTTPHandler::Exception("MimeTypesException: " + usermsg)
{
}

MimeTypes * MimeTypes::dm_instance;

MimeTypes::MimeTypes(void)
{
  assert(dm_instance == 0);
  dm_instance = this;

  initData();
}

MimeTypes::~MimeTypes()
{
  assert(dm_instance == this);
  dm_instance = 0;
}

MimeTypes * MimeTypes::instance(void)
{
  return dm_instance;
}

bool MimeTypes::containsMimeType(const QString &ext)
{
  assert(dm_instance && "[MimeTypes is not instantiated]");
  return dm_instance->dm_ext.find(ext) != dm_instance->dm_ext.end();
}

const QString & MimeTypes::mimeType(const QString &ext)
{
  assert(dm_instance && "[MimeTypes is not instantiated]");
  map_t::const_iterator ii = dm_instance->dm_ext.find(ext);

  if (ii == dm_instance->dm_ext.end())
    throw MimeTypesException(ext);
  else
    return *(ii->second);
}

void MimeTypes::addMimeType(const QString &type, const QString &ext)
{
  dm_ext[ext] = data_t(new QString(type));
}

void MimeTypes::addMimeType(const QString &type, const QStringList &extlist)
{
  data_t dat(new QString(type));

  for (QStringList::const_iterator ii=extlist.begin(); ii != extlist.end(); ++ii)
    dm_ext[*ii] = dat;
}

void MimeTypes::initData(void)
{
  addMimeType("application/xml", QStringList("xml") << "xsl" << "xsd");
  addMimeType("application/zip", "zip");
  addMimeType("text/html", QStringList("html") << "htm" << "shtml");
  addMimeType("text/plain", QStringList("asc") << "txt" << "text" << "pot" << "brf");
  addMimeType("text/css", "css");
  addMimeType("image/png", "png");
  addMimeType("image/jpeg", QStringList("jpeg") << "jpg" << "jpe");
  addMimeType("image/gif", "gif");
  addMimeType("image/tiff", QStringList("tiff") << "tif");

  // TODO add more types... read from OS?
  // TODO mark sertain formats as secure, or just not include insecure formats (like exe)
}

