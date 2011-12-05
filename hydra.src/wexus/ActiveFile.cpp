
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveFile.h>

#include <algorithm>

#include <QFile>
#include <QVariant>

#include <wexus/AssertException.h>

using namespace wexus;

ActiveFile::ActiveFile(const QString &dirname, const QRegExp &regexp)
  : dm_dirspec(new DirSpec)
{
  dm_dirspec->dirname = dirname;
  dm_dirspec->regexp = regexp;
}

QVariant ActiveFile::getIDAsVariant(void)
{
  return id;
}

void ActiveFile::checkFileName(const QString &filename)
{
  if (filename.isEmpty() || filename.indexOf('/') != -1
      || filename.indexOf('\\') != -1
      || filename[0] == '.'
      || !dm_dirspec->regexp.exactMatch(filename))
    throw AssertException("checkFileName failure");
}

void ActiveFile::find(const QVariant &keyVal)
{
  if (exists(keyVal)) {
    id = keyVal.toString();
    if (!onLoad(dm_dirspec->dirname + "/" + id))
      throw AssertException("ActiveFile::find: onLoad failed: " + keyVal.toString());
  } else
    throw AssertException("ActiveFile::find: file not found: " + keyVal.toString());
}

bool ActiveFile::exists(const QVariant &keyVal)
{
  QString fname(keyVal.toString());

  checkFileName(fname);

  return QFileInfo(dm_dirspec->dirname + "/" + fname).isFile();
}

QByteArray & ActiveFile::asByteArray(void)
{
  if (dm_data.get() && dm_data->filename == id)
    return dm_data->bytearray;

  dm_data.reset(new DataSpec);

  QString fullfilename(dm_dirspec->dirname + "/" + id);
  QFile infile(fullfilename);

  if (!infile.open(QIODevice::ReadOnly))
    throw AssertException("ActiveFile::asByteArray(): Can't open: " + fullfilename);

  dm_data->bytearray = infile.readAll(); // doesnt do any error checking, should probably do some

  return dm_data->bytearray;
}

void ActiveFile::all(bool reverseOrder)
{
  std::shared_ptr<IteratorSpec> ii(new IteratorSpec);

  QDirIterator dd(dm_dirspec->dirname);
  // find all the qualify records
  while (dd.hasNext()) {
    QString fpath = dd.next();
    QString fname = dd.fileName();

    if (!QFileInfo(fpath).isFile())
      continue;

    if (dm_dirspec->regexp.exactMatch(fname))
      ii->filenames.push_back(fname);
  }

  // sort the list
  ii->filenames.sort();
  if (reverseOrder)
    std::reverse(ii->filenames.begin(), ii->filenames.end());

  ii->iterator = ii->filenames.begin();

  dm_iterator = ii;
}

bool ActiveFile::next(void)
{
  assertThrow(dm_iterator.get());

  if (dm_iterator->iterator != dm_iterator->filenames.end()) {
    id = *dm_iterator->iterator;
    dm_iterator->iterator++;
    return onLoad(dm_dirspec->dirname + "/" + id);
  }

  // ran out of entries
  dm_iterator.reset();

  return false;
}

bool ActiveFile::onLoad(const QString &fullfilename)
{
  return true;
}

