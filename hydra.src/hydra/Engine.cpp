
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Engine.h>

#include <assert.h>

#include <iostream>

#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QDebug>

#include <hydra/DB.h>
#include <hydra/Records.h>
#include <hydra/Hash.h>
#include <hydra/FileIterator.h>

using namespace hydra;

Engine *Engine::dm_instance;

Engine::Engine(void)
{
  // this is a work around bug
  // http://bugreports.qt.nokia.com/browse/QTBUG-11213
  qsrand(QDateTime::currentDateTime().toTime_t());
  qDebug() << "Engine random boot UUID" << QUuid::createUuid().toString();

  assert(dm_instance == 0);
  dm_instance = this;

  QString dir(dbDir());

  dm_filepathdb.reset(new DB(dir + "/cache1.sqlite", "filepaths"));
  dm_filehashdb.reset(new DB(dir + "/main1.sqlite", "filehashes"));
  dm_fileitemdb.reset(new DB(*dm_filehashdb, "fileitems"));
}

Engine::~Engine()
{
  assert(dm_instance == this);
  dm_instance = 0;
}

// may return empty string if not found/set/can't compute
QString Engine::homeDir(void)
{
  // update this to use QProcess::systemEnvironment
  const char * home_dir = ::getenv("HOME");
  if (!home_dir)
    home_dir = ::getenv("USERPROFILE");   //win32
  if (!home_dir)
    home_dir = ".";
  return home_dir;
}

//#define DB_TEST_DIR
QString Engine::dbDir(void)
{
  QFileInfo dbdir_name(homeDir() + "/.hydradb");

  //make sure the dir exists
  mkDir(dbdir_name.filePath());

  return dbdir_name.canonicalFilePath();
}

int Engine::addFile(const QString &fullfilename, const QString *precalchash)
{
  FilePathRecord path;
  bool new_path = true;
  double disk_time;
  qint64 disk_size;
  QUuid old_path_itemid;   // if new_path

  if (!statFile(fullfilename, disk_time, disk_size))
    return Add_Error; // perhaps throw an exception instead?

  // check if PATH exists
  if (dm_filepathdb->get(fullfilename, path) && !precalchash) {
    FileHashRecord hash;

    new_path = false;

    if (dm_filehashdb->get(path.hash, hash))
      if (dm_fileitemdb->has(hash.id))
        old_path_itemid = hash.id;

    // check if PATH needs updating
    if (disk_time == path.file_time && disk_size == path.file_size) {
      // PATH is synced with disk, now just make sure a HASH and ITEM exist

      if (!old_path_itemid.isNull())
        return Add_Exists;
    }

    // PATH needs updating, fall through
  }

  // populate PATH
  // calculate the has of this file
  if (precalchash)
    path.hash = *precalchash;
  else
    path.hash = calcFileHash(fullfilename);
  if (path.hash.isEmpty())
    return Add_Error; // perhaps throw an exception instead?
  path.file_time = disk_time;
  path.file_size = disk_size;

  FileHashRecord hash;

  // check if HASH exists
  if (dm_filehashdb->get(path.hash, hash)) {
    // exists, just save the new path
    if (!dm_filepathdb->put(fullfilename, path))
      return Add_Error; // perhaps throw an exception instead?
    if (new_path)
      return Add_NewPath;
    else
      return Add_UpdatedPath;
  }

  // no HASH, create a new ITEM and HASH
  if (old_path_itemid.isNull())
    // TODO in the future, replace this with libuui under Linux, as this needs to be STRONG
    hash.id = QUuid::createUuid();       // creating a new ID/item
  else
    hash.id = old_path_itemid;    // linking to the old item

  if (!dm_filepathdb->put(fullfilename, path))
    return Add_Error; // perhaps throw an exception instead?
  if (!dm_filehashdb->put(path.hash, hash))
    return Add_Error; // perhaps throw an exception instead?

  if (!old_path_itemid.isNull())
    return Add_UpdatedPath;

  // finally, add the ITEM
  FileItemRecord item;

  item.id = hash.id;

  // detect the file type
  if (isImageFile(QFileInfo(fullfilename).fileName()))
    item.filetype = 0;
  else
    item.filetype = 1;

  if (!dm_fileitemdb->put(item.id, item))
    return Add_Error; // perhaps throw an exception instead?

  return Add_New;
}

bool Engine::erasePath(const QString &fullfilename)
{
  return dm_filepathdb->erase(fullfilename);
}

bool Engine::eraseHash(const QString &hash)
{
  return dm_filehashdb->erase(hash);
}

int Engine::getFileItem(const QString &fullfilename, hydra::FileItemRecord *item, hydra::FileHashRecord *hash,
        hydra::FilePathRecord *path)
{
  FileItemRecord localitem;
  FileHashRecord localhash;
  FilePathRecord localpath;
  double disk_time;
  qint64 disk_size;

  if (!item)
    item = &localitem;
  if (!hash)
    hash = &localhash;
  if (!path)
    path = &localpath;

  if (!dm_filepathdb->get(fullfilename, *path))
    return Load_ErrorNotFound;

  if (!statFile(fullfilename, disk_time, disk_size))
    return Load_ErrorFileMissing;

  if (disk_size != path->file_size || disk_time != path->file_time)
    return Load_ErrorNeedsUpdate;

  if (!dm_filehashdb->get(path->hash, *hash))
    return Load_ErrorNotFound;

  return dm_fileitemdb->get(hash->id, *item) ? Load_OK : Load_ErrorNotFound;
}

int Engine::getFileItem(const QString &fullfilename, hydra::FileItemRecord &rec)
{
  return getFileItem(fullfilename, &rec, 0, 0);
}

int Engine::getFileItemByHash(const QString &hashkey, hydra::FileItemRecord *item, hydra::FileHashRecord *hash)
{
  FileItemRecord localitem;
  FileHashRecord localhash;

  if (!item)
    item = &localitem;
  if (!hash)
    hash = &localhash;

  if (!dm_filehashdb->get(hashkey, *hash))
    return Load_ErrorNotFound;

  return dm_fileitemdb->get(hash->id, *item) ? Load_OK : Load_ErrorNotFound;
}

int Engine::regetFileItem(hydra::FileItemRecord &rec)
{
  assert(!rec.id.isNull());
  return dm_fileitemdb->get(rec.id, rec) ? Load_OK : Load_ErrorFatal;
}

bool Engine::saveFileItem(hydra::FileItemRecord &rec, const QDateTime &newmodtime)
{
  if (newmodtime.isValid())
    rec.modtime = newmodtime;
  return dm_fileitemdb->put(rec.id, rec);
}

char Engine::codeToChar(int code)
{
  switch (code)
  {
    case Add_New: return 'N';
    case Add_Exists: return 'E';
    case Add_NewPath: return 'P';
    case Add_UpdatedPath: return 'U';
    case Load_OK: return 'F';
    default: return '!';
  }
}

static double dataTimeToDouble(const QDateTime &dt)
{
  QDateTime epoch;
  double a, b;

  epoch.setTime_t(0);

  a = epoch.daysTo(dt);
  b = QTime(0, 0).secsTo(dt.time());

  return a + b/(60*60*24);    // seconds per day
}

bool Engine::statFile(const QString &fullfilename, double &modtime, qint64 &filesize)
{
  QFileInfo f(fullfilename);

  if (!f.isFile())
    return false;

  modtime = dataTimeToDouble(f.lastModified());
  filesize = f.size();

  return true;
}

