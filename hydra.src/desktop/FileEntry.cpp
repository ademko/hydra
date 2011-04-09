
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FileEntry.h>

#include <assert.h>

#include <QDebug>

#include <hydra/Engine.h>
#include <hydra/FileIterator.h>
#include <hydra/RotateCode.h>
#include <hydra/Exif.h>

using namespace hydra;
using namespace desktop;

//
//
// FileEntry
//
//

FileEntry::FileEntry(const QString &_fullfilename)
  : dm_recordloaded(false), dm_fullfilename(_fullfilename), dm_justname(justName(_fullfilename))
{
  dm_fileSize = -1;
  dm_rotateCode = -1;
}

bool FileEntry::hasRecord(bool *needsBigRead) const
{
  if (needsBigRead)
    *needsBigRead = false;    // assume false

  if (!dm_recordloaded)
    loadRecord(needsBigRead);

  return dm_recordloaded;
}

const QString & FileEntry::recordHash(void) const
{
  if (!dm_recordloaded)
    loadRecord();

  return dm_hash;
}

hydra::FileItemRecord & FileEntry::recordItem(void) const
{
  if (!dm_recordloaded)
    loadRecord();

  //assert(dm_item.get());
  //return *dm_item;
  return dm_item;
}

QDateTime FileEntry::fileLastModified () const
{
  if (dm_fileSize == -1)
    loadFile();

  return dm_fileLastModified;
}

qint64 FileEntry::fileSize(void) const
{
  if (dm_fileSize == -1)
    loadFile();

  return dm_fileSize;
}

void FileEntry::loadRecord(bool *needsBigRead) const
{
  int code;

//qDebug() << __FUNCTION__ << dm_fullfilename << (needsBigRead!=0) << dm_recordloaded;
  if (dm_recordloaded)
    return;

  //dm_failedrecord = !FileItemCache::instance()->getItem(dm_fullfilename, dm_item, dm_hash, needsBigRead);
  // the following used to be in the FileItemCache, which in the future
  // we'll probably move back

  if (dm_hash.isEmpty()) {
    // read the full item record, by filename
    FilePathRecord pathrec;
    //FileHashRecord hashrec;
//qDebug() << "FileEntry" << __FUNCTION__ << "getFileItem" << dm_fullfilename;
    code = Engine::instance()->getFileItem(dm_fullfilename, &dm_item, 0,/*&hashrec,*/ &pathrec);
    if (code == Engine::Load_OK)
      dm_hash = pathrec.hash;
  } else {
    // read the item record, by hash
//qDebug() << "FileEntry" << __FUNCTION__ << "getFileItemByHash" << dm_hash;
    code = Engine::instance()->getFileItemByHash(dm_hash, &dm_item, 0);
  }

  if (code != Engine::Load_OK) {
    if (needsBigRead) {
      // big operation would follow. bail if the caller wanted us to
      *needsBigRead = true;
      return;
    }

    // see if we need to add the item, which is considered a bigread as this might trigger
    // a checksum of the file contents
//qDebug() << "FileEntry" << __FUNCTION__ << "addFile" << dm_fullfilename;
    if (Engine::instance()->addFile(dm_fullfilename) != Engine::Add_Error) {
      FilePathRecord pathrec;
      code = Engine::instance()->getFileItem(dm_fullfilename, &dm_item, 0, &pathrec);
      if (code == Engine::Load_OK)
        dm_hash = pathrec.hash;
    }
  }

  //dm_failedrecord = code != Engine::Load_OK;
  // make it a 'successfull' read... this way, even if we get an error, we wont
  // keep trying to reread the records
  dm_recordloaded = true;
}

void FileEntry::loadFile(void) const
{
  QFileInfo info(dm_fullfilename);

  dm_fileLastModified = info.lastModified();
  dm_fileSize = info.size();
}

int FileEntry::rotateCode(void)
{
  if (dm_rotateCode >= 0)
    return dm_rotateCode;

  // basically imeplementing hydra::RotateCode here

  // first use the tags
  dm_rotateCode = detectTagRotate(recordItem().tags);

  if (dm_rotateCode >= 0)
    return dm_rotateCode;

  // now use the exif info, if any
  dm_rotateCode = detectExifRotate(fullfilename());

  if (dm_rotateCode < 0)
    dm_rotateCode = 0;

  return dm_rotateCode;
}

int FileEntry::reloadRotateCode(void)
{
  dm_rotateCode = -1;

  return rotateCode();
}

