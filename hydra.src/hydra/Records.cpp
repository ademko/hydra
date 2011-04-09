
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Records.h>

#include <assert.h>

#include <QDebug>

#include <hydra/Query.h> // for SCORE_CHAR

using namespace hydra;

//
//
// FilePathRecord
//
//

FilePathRecord::FilePathRecord(void)
  : file_time(0), file_size(0)
{
}

void FilePathRecord::save(QDataStream &out) const
{
  out << (qint32)2;   // write version;
  out << hash;
  out << file_time;
  out << file_size;
}

void FilePathRecord::load(QDataStream &in)
{
  qint32 version;

  in >> version;    // read version
  in >> hash >> file_time >> file_size;
}

//
//
// FileHashRecord
//
//

FileHashRecord::FileHashRecord(void)
{
}

void FileHashRecord::save(QDataStream &out) const
{
  out << (qint32)1;   // write version;
  out << id;
}

void FileHashRecord::load(QDataStream &in)
{
  qint32 version;

  in >> version;    // read version

  in >> id;
}

Tags::Tags(void)
{
}

void Tags::debugPrint(void) const
{
  for (const_iterator ii=begin(); ii != end(); ++ii)
    qDebug() << *ii;
}

bool Tags::insertTag(const QString &newtag, bool *imageChanged)
{
  assert(newtag.size() > 0);

  if (count(newtag) > 0)
    return false; // already have this tag, no need

  bool isstar(newtag[0] == hydra::SCORE_CHAR);

  if (isstar) {
    // star logic
    // find the existing star, and erase it, if need be
    iterator ii = lower_bound(QString(hydra::SCORE_CHAR));

    // erase the existing score, if any
    if (ii != end() && (*ii)[0] == hydra::SCORE_CHAR)
      erase(ii);

    // finally, insert this new score
    insert(newtag);
    return true;
  }

  int k=indexOfKeyValue(newtag);

  if (k >= -1) {
    // this is a key=value, erase the old one first
    QString key(newtag.left(k));

    // remove the old key
    if (hasKey(key))
      eraseKey(key);

    insert(newtag);

    // does this change the image rotation? note it
    if (imageChanged && key == "rotate")
      *imageChanged = true;

    return true;
  }

  // basic tag, just insert it

  insert(newtag);

  return true;
}

bool Tags::eraseTag(const QString &newtag, bool *imageChanged)
{
  iterator ii = find(newtag);

  if (ii == end())
    return false;

#define LEN_OF_ROTATE 6
  int index = indexOfKeyValue(newtag);
  if (index == LEN_OF_ROTATE && imageChanged && newtag.startsWith("rotate"))
    *imageChanged = true;

  erase(ii);

  return true;
}

bool Tags::clearTags(bool *imageChanged)
{
  if (empty())
    return false;

  if (imageChanged && hasKey("rotate"))
    *imageChanged = true;

  clear();

  return true;
}

bool Tags::hasKey(const QString &key) const
{
  const_iterator ii = lower_bound(key);

  if (ii == end())
    return false;

  int index = indexOfKeyValue(*ii);

  if (index < 0)
    return false;

  // check if the key matches
  if (index != key.size() || !(*ii).startsWith(key))
    return false;
  return true;
}

bool Tags::insertKey(const QString &key, const QString &value, bool *imageChanged)
{
  QString newtag(key + SEP_CHAR + value);

  return insertTag(newtag, imageChanged);
}

bool Tags::eraseKey(const QString &key, bool *imageChanged)
{
  const_iterator ii = lower_bound(key);

  if (ii == end())
    return false;

  int index = indexOfKeyValue(*ii);

  if (index < 0)
    return false;

  // check if the key matches
  if (index != key.size() || !(*ii).startsWith(key))
    return false;

  // good, now erase it
  return eraseTag(*ii, imageChanged);
}

QString Tags::keyValue(const QString &key) const
{
  const_iterator ii = lower_bound(key);

  if (ii == end())
    return "";

  int index = indexOfKeyValue(*ii);

  if (index < 0)
    return "";

  // check if the key matches
  if (index != key.size() || !(*ii).startsWith(key))
    return "";

  // key is good, return the value
  return (*ii).mid(index+1);
}

//
//
// FileItemRecord
//
//

FileItemRecord::FileItemRecord(void)
  : filetype(0)
{
}

void FileItemRecord::save(QDataStream &out) const
{
  out << (qint32)3;   // write version;

  out << id << title << desc << filetype;

  out << (qint32)tags.size();
  for (tags_t::const_iterator ii=tags.begin(); ii != tags.end(); ++ii) {
    assert(!ii->isEmpty());
    out << *ii;
  }

  // always write out the data in 64-bit uint format
  // that way, we won't get hit by the ~2030 bug
  // (yes, we'll need to update the time_t functions
  // to do 64-bit stuff ofcourse
  out << modtimeAsInt64();
}

void FileItemRecord::load(QDataStream &in)
{
  qint32 version;

  modtime = QDateTime();

  in >> version;

  in >> id >> title >> desc >> filetype;

  qint32 sz;

  in >> sz;

  if (sz > 16000)        // surely greater than this is some kind of error?
    throw FormatException();

  tags.clear();
  for(; sz>0; --sz) {
    tags_t::value_type v;
    in >> v;
    assert(!v.isEmpty());
    tags.insert(v);
  }

  if (version < 3)
    return;

  // version 3 added modtime
  quint64 intmodtime;

  in >> intmodtime;

  // in the future, make this thing grok 64-bit dates to avoid a ~2030 bug
  setModtimeFromInt64(intmodtime);
}

