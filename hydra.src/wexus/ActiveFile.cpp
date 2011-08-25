
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
  dm_iterator.reset(new QDirIterator(dm_dirspec->dirname));
}

bool ActiveFile::next(void)
{
  assertThrow(dm_iterator.get());

  // find the next record
  while (dm_iterator->hasNext()) {
    QString fpath = dm_iterator->next();
    QString fname = dm_iterator->fileName();

    if (!QFileInfo(fpath).isFile())
      continue;

    if (dm_dirspec->regexp.exactMatch(fname)) {
      filename = fname;
      return true;
    }
  }

  // ran out of QDirIterator entries
  dm_iterator.reset();

  return false;
}

