
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ActiveFile.h>

#include <QFile>

#include <wexus/Assert.h>

using namespace wexus;

ActiveFile::ActiveFile(const QString &dirname, const QRegExp &regexp)
  : dm_dirspec(new DirSpec)
{
  dm_dirspec->dirname = dirname;
  dm_dirspec->regexp = regexp;
}

QByteArray & ActiveFile::asByteArray(void)
{
  if (dm_data.get() && dm_data->filename == filename)
    return dm_data->bytearray;

  dm_data.reset(new DataSpec);

  QString fullfilename(dm_dirspec->dirname + "/" + filename);
  QFile infile(fullfilename);

  if (!infile.open(QIODevice::ReadOnly))
    throw AssertException("ActiveFile::asByteArray(): Can't open: " + fullfilename);

  dm_data->bytearray = infile.readAll(); // doesnt do any error checking, should probably do some

  return dm_data->bytearray;
}

void ActiveFile::all(void)
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

  ii->iterator = ii->filenames.begin();

  dm_iterator = ii;
}

bool ActiveFile::next(void)
{
  assertThrow(dm_iterator.get());

  if (dm_iterator->iterator != dm_iterator->filenames.end()) {
    filename = *dm_iterator->iterator;
    dm_iterator->iterator++;
    return true;
  }

  // ran out of entries
  dm_iterator.reset();

  return false;
}

