
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/CommandImport.h>

#include <assert.h>

#include <map>

#include <QFile>
#include <QDomDocument>
#include <QXmlDefaultHandler>
#include <QDebug>

#include <hydra/Engine.h>
#include <hydra/Records.h>
#include <hydra/NodePath.h>

using namespace hydra;

struct loadeditem_t {
  FileItemRecord item;    // the item read from the xml import
  QUuid existingid;             // if !isNull, this is the real ID of the file already in the system
};

/**
 * This does a straight copy from src to existingdest
 * @author Aleksander Demko
 */ 
static bool copyItem(const FileItemRecord &src, FileItemRecord &existingdest)
{
  bool changed = false;

  // dont touch id

  if (src.title != existingdest.title) {
    existingdest.title = src.title;
    changed = true;
  }

  if (src.desc != existingdest.desc) {
    existingdest.desc = src.desc;
    changed = true;
  }

  if (src.tags != existingdest.tags) {
    existingdest.tags = src.tags;
    changed = true;
  }

  if (src.modtime != existingdest.modtime) {
    existingdest.modtime = src.modtime;
    changed = true;
  }

  return changed;
}

/**
 * This does a (old style) merge. title and desc are copied to existingdest
 * and the tags will be the super set of both records.
 *
 * @author Aleksander Demko
 */ 
static bool supersetMergeItem(const FileItemRecord &src, FileItemRecord &existingdest)
{
  bool changed = false;

  // dont touch id

  if (!src.title.isEmpty() && src.title != existingdest.title) {
    existingdest.title = src.title;
    changed = true;
  }

  if (!src.desc.isEmpty() && src.desc != existingdest.desc) {
    existingdest.desc = src.desc;
    changed = true;
  }

  // merge new tags and longest $-score tags
  QString scoretag;

  for (FileItemRecord::tags_t::const_iterator tt=src.tags.begin(); tt!=src.tags.end(); ++tt) {
    assert(!tt->isEmpty());

    if ((*tt)[0] == '$' && (*tt).size() > scoretag.size())
      scoretag = (*tt);
    else {
      if (existingdest.tags.find(*tt) == existingdest.tags.end()) {
        existingdest.tags.insert(*tt);    // insert the new tag
        changed = true;
      }
    }
  }

  // see if the scoretag needs inserting (eliminate any other ones)
  if (!scoretag.isEmpty())
    for (FileItemRecord::tags_t::iterator tt=existingdest.tags.begin(); tt!=existingdest.tags.end(); ++tt) {
      if ((*tt)[0] == '$') {
        // found the score tag
        if (scoretag.size() > tt->size())
          existingdest.tags.erase(tt);      // our score tag is better
        else
          scoretag.clear();                 // out score tag doesnt win
        break;
      }
    }
  // if we still have a scoretag, just insert it as the dest didnt have one at all
  if (!scoretag.isEmpty()) {
    existingdest.tags.insert(scoretag);
    changed = true;
  }

  // date/time
  if (src.modtime < existingdest.modtime) {
    existingdest.modtime = src.modtime;
    changed = true;
  }

  return changed;
}

/**
 * Does a merge based on modtimes.
 *
 * @author Aleksander Demko
 */ 
static bool modtimeMergeItem(const FileItemRecord &src, FileItemRecord &existingdest)
{
  QDateTime nulltime;

  // if they both dont have a user time, do an old style super set merge
  if (src.modtime == nulltime && existingdest.modtime == nulltime)
    return supersetMergeItem(src, existingdest);
  // if just source is invalid, bail
  if (src.modtime == nulltime)
    return false;
  // if just the dest is invalid, copy
  if (existingdest.modtime == nulltime)
    return copyItem(src, existingdest);
  // both are valid, do a full time compare
  if (existingdest.modtime < src.modtime)
    return copyItem(src, existingdest);
  // time compare failed, existing is newer, do nothing
  return false;
}

class MyHandler : public QXmlDefaultHandler
{
  public:
    MyHandler(bool smartmerge);
    virtual bool fatalError(const QXmlParseException & exception);
    virtual bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts);
    virtual bool endElement(const QString & namespaceURI, const QString & localName, const QString & qName);

    int commit(void);

  private:
    typedef std::map<QString, QUuid> hashmap_t;
    typedef std::map<QUuid, loadeditem_t> itemmap_t;

    bool dm_smartmerge;
    QUuid dm_last_item_id;

    hashmap_t dm_hashmap;
    itemmap_t dm_itemmap;
};

MyHandler::MyHandler(bool smartmerge)
  : dm_smartmerge(smartmerge)
{
}

bool MyHandler::fatalError(const QXmlParseException & exception)
{
  qDebug() << "Parse error at line " << exception.lineNumber() << ": " << exception.message();
  return true;
}

bool MyHandler::startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts)
{
  //std::cerr << "localName=" << localName.toUtf8().constData() << " qName=" << qName.toUtf8().constData() << '\n';

  // ignore path entries
  if (qName == "path")
    return true;

  if (qName == "hash") {
    // build up hashmap
    try {
      FileHashRecord hash;

      QString key_string = atts.value("hash");
      QString id_string = atts.value("id");

      NodePath::verify(!key_string.isEmpty());
      NodePath::verify(!id_string.isEmpty());

      hash.id = QUuid(id_string);
      NodePath::verify(!hash.id.isNull());

      dm_hashmap[key_string] = hash.id;
    }
    catch (const NodePath::error&) { }
    return true;
  }

  if (qName == "item") {
    try {
      // build up the item map
      try {
        FileItemRecord item;
        int index;

        QString id_string = atts.value("id");

        NodePath::verify(!id_string.isEmpty());

        item.id = QUuid(id_string);
        NodePath::verify(!item.id.isNull());

        index = atts.index("title");
        if (index != -1)
          item.title = atts.value("title");

        index = atts.index("desc");
        if (index != -1)
          item.desc = atts.value("desc");

        index = atts.index("filetype");
        if (index != -1) {
          bool okc;
          item.filetype = atts.value("filetype").toDouble(&okc);
          if (!okc)
            NodePath::verify(false);    // just throw something
        }

        // mod time
        index = atts.index("modtime");
        if (index != -1) {
          bool okc;
          quint64 modtime = atts.value("modtime").toULongLong(&okc);
          if (!okc)
            NodePath::verify(false);    // just throw something
          item.setModtimeFromInt64(modtime);
        }

        dm_itemmap[item.id].item = item;
        dm_last_item_id = item.id;
      }
      catch (const NodePath::error&) { }
    }
    catch (const NodePath::error&) { }
    return true;
  }

  if (qName == "tag" && !dm_last_item_id.isNull()) {
    QString tag_title = atts.value("title");

    if (!tag_title.isEmpty())
      dm_itemmap[dm_last_item_id].item.tags.insert(tag_title);
  }
  return true;
}

bool MyHandler::endElement(const QString & namespaceURI, const QString & localName, const QString & qName)
{
  if (qName == "item")
    dm_last_item_id = QUuid();

  return true;
}

int MyHandler::commit(void)
{
  int count = 0;
  Engine *eng = Engine::instance();

  // now iterate over all the loaded hashes.
  // for each one that is in our DB already, redirect the loaded item
  // to our internal, already existing item
  FileHashRecord hash;
  hashmap_t::iterator hh=dm_hashmap.begin(), endhh=dm_hashmap.end();
  while (hh != endhh) {
    if (eng->fileHashDB().get(hh->first, hash)) {
      // this hash already exists in my DB, lets do some remapping
      // first, change the loaded item (if any)
      itemmap_t::iterator tt = dm_itemmap.find(hh->second);
      if (tt != dm_itemmap.end())
        tt->second.existingid = hash.id;    // make the loaded item be merged with the already existing item

      // next, remove this hash (as it already exists in my db)
      hashmap_t::iterator todel = hh;
      ++hh;
      dm_hashmap.erase(todel);
    } else
      ++hh;   // move on and keep this hash, its new
  }//while hh

  // write the item records
  FileItemRecord myitem;
  for (itemmap_t::iterator ii=dm_itemmap.begin(); ii != dm_itemmap.end(); ++ii) {
    bool domerge;

    if (ii->second.existingid.isNull())
      domerge = eng->fileItemDB().get(ii->second.item.id, myitem);
    else
      domerge = eng->fileItemDB().get(ii->second.existingid, myitem);

    if (domerge) {
      if (
          (!dm_smartmerge && copyItem(ii->second.item, myitem)) ||
          (dm_smartmerge && modtimeMergeItem(ii->second.item, myitem)) ) {
        eng->fileItemDB().put(myitem.id, myitem);
        ++count;
      }
    } else {
      // not merge, new item
      assert(!ii->second.item.id.isNull());
      eng->fileItemDB().put(ii->second.item.id, ii->second.item);
      ++count;
    }
  } // for dm_itemmap

  // finally, comit all the new hashes to disk
  hh=dm_hashmap.begin();
  endhh=dm_hashmap.end();
  for (; hh != endhh; ++hh) {
    hash.id = hh->second;
    eng->fileHashDB().put(hh->first, hash);
  }

  return count;
}

int hydra::commandImport(const QString &filename, bool smartmerge)
{
  QFile file(filename);
  QByteArray buffer;
  QXmlInputSource inputsource;
  QXmlSimpleReader reader;
  MyHandler handler(smartmerge);
  bool ok = true, first = true;

  if (!file.open(QIODevice::ReadOnly))
    return -1;

  reader.setContentHandler(&handler);
  reader.setErrorHandler(&handler);

  while (ok) {
    buffer = file.read(64*1024);

    if (buffer.isEmpty())
      break;

    inputsource.setData(buffer);

    if (first) {
      ok = reader.parse(&inputsource, true);
      first = false;
    } else
      ok = reader.parseContinue();
  }

  if (!ok)
    return -1;  // xml parser error

  return handler.commit();
}

