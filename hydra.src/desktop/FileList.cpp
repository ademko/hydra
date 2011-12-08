
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FileList.h>

#include <assert.h>

#include <algorithm>
#include <hydra/TR1.h>

#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QThreadPool>

#include <hydra/Engine.h>
#include <hydra/FileIterator.h>
#include <hydra/Hash.h>
#include <desktop/View.h>

#include <desktop/RunnableEvent.h>

using namespace hydra;
using namespace desktop;

//
//
// FileSystemWatcher
//
//

FileSystemWatcher::FileSystemWatcher(FileList *parent)
  : dm_parent(parent)
{
  dm_queued = false;
  dm_requeue = false;

  assert(parent);

  connect(&dm_watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirChanged()));
}

void FileSystemWatcher::setWatchDir(const QString &dir)
{
  if (dir == dm_currentdir)
    return;

  if (!dm_currentdir.isEmpty())
    dm_watcher.removePath(dm_currentdir);
  if (!dir.isEmpty())
    dm_watcher.addPath(dir);

  dm_currentdir = dir;
}

void FileSystemWatcher::onDirChanged(void)
{
  //qDebug() << __FUNCTION__ << dm_currentdir;
  if (dm_queued) {
    dm_requeue = true;
    return;
  }

  dm_queued = true;
  QTimer::singleShot(1000, this, SLOT(onTimer()));
}

void FileSystemWatcher::onTimer(void)
{
  //qDebug() << __FUNCTION__ << dm_currentdir;
  if (dm_requeue) {
    dm_requeue = false;
    QTimer::singleShot(1000, this, SLOT(onTimer()));
  }

  dm_queued = false;
  FileListReloader r(*dm_parent);
}

//
//
// FileList
//
//

FileList::FileList(FileEntryCache *_cache)
  : dm_cache(_cache), dm_peer(0), dm_base_recurse(false), dm_base_shuffle(false), /*dm_selectedfile(0),*/ dm_selectionmodel(this),
    dm_watcher(this)
{
  dm_all_files.reserve(1000);
  dm_filtered_files.reserve(1000);

  assert(dm_cache);
  parseQueryTokens(dm_base_query, dm_base_query_token);
}

FileList::~FileList()
{
  listeners_t::iterator ii;
  for (ii=dm_listeners.begin(); ii != dm_listeners.end(); ++ii)
    (*ii)->resetFileList();

  flushJobs();
  QThreadPool::globalInstance()->waitForDone();
}

int FileList::rowCount(const QModelIndex &parent) const
{
  return numFiles();
}

int FileList::columnCount(const QModelIndex &parent) const
{
  return 5;
}

QVariant FileList::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation != Qt::Horizontal)
    return QVariant();

  switch (section) {
    case 0: return "Filename";
    case 1: return "Tags";
    case 2: return "Date";
    case 3: return "Size";
    case 4: return "Path";
    default: assert(false); return QVariant();
  }
}

QVariant FileList::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();

  const FileEntry &entry = fileAt(index.row());

  switch (index.column()) {
    case 0:
      return entry.justname();
    case 1: {
              bool needsBigRead = false;
              entry.containsRecord(&needsBigRead);

              if (needsBigRead) {
                const_cast<FileList*>(this)->enqueueLoad(entry.fullfilename(), index.row());
                return "loading...";
              } else {
                QString ret;
                for (FileItemRecord::tags_t::const_iterator ii=entry.recordItem().tags.begin();
                    ii != entry.recordItem().tags.end(); ++ii) {
                  ret.append(' ');
                  ret.append(*ii);
                }
                return ret;
              }
            }
    case 2: return entry.fileLastModified().toString(Qt::SystemLocaleShortDate);
    //case 2: return entry.fileLastModified().toString(Qt::ISODate);
    case 3: return QString::number(entry.fileSize() / 1024) + "K";
    case 4:
      return QFileInfo(entry.fullfilename()).path();
    default:
      assert(false);
      return QVariant();
  }
}

void FileList::sort(int column, Qt::SortOrder order)
{
  short sortby;

  switch (column) {
    case 0: sortby = FileEntryLessThan::colJustName; break;
    case 1: sortby = FileEntryLessThan::colRecordTags; break;
    case 2: sortby = FileEntryLessThan::colFileLastModified; break;
    case 3: sortby = FileEntryLessThan::colFileSize; break;
    default: return;
  }

  qSort(dm_filtered_files.begin(), dm_filtered_files.end(), FileEntryLessThan(sortby, order));

  reset();
  emitChangedTagsAll();
}

void FileList::setPeer(FileList *peer)
{
  dm_peer = peer;
}

int FileList::selectedFileIndex(void) const
{
  QModelIndex m(dm_selectionmodel.currentIndex());

  if (m.isValid())
    return m.row();
  else
    return -1;
}

int FileList::findFileIndexByName(const QString &fullfilename) const
{
  int index = 0;

  for (EntryList::const_iterator ii=dm_filtered_files.begin(); ii != dm_filtered_files.end(); ++ii, ++index)
    if ((*ii)->fullfilename() == fullfilename)
      return index;

  return -1;
}

void FileList::setSelectedFile(int index)
{
  //qDebug() << "A";
  if (index < 0 || index >= dm_filtered_files.size() || index == selectedFileIndex())
    return;
  //qDebug() << "B";

  //dm_selectedfile = index;
  dm_selectionmodel.setCurrentIndex(FileList::index(index,0), QItemSelectionModel::ClearAndSelect);
}

void FileList::addListener(FileListListener *v)
{
  dm_listeners.push_back(v);
}

/*void FileList::setMarkedFiles(const std::vector<bool> &marks, View *source)
{
  dm_marks = marks;
  emitMarkedChanged(source);
}

void FileList::setMarkedFiles(QAbstractItemView *itemview, View *source)
{
  //QModelIndexList &list
  QItemSelectionModel *m = itemview->selectionModel();
  QModelIndex i;

  for (int x=0; x<dm_marks.size(); ++x) {
    i = index(x, 0);

    dm_marks[x] = m->isSelected(i);
  }
  emitMarkedChanged(source);
}*/

void FileList::removeListener(FileListListener *v)
{
  listeners_t::iterator ii;
  for (ii=dm_listeners.begin(); ii != dm_listeners.end(); ++ii)
    if (*ii == v) {
      dm_listeners.erase(ii);
      return;
    }
}

void FileList::emitChangedTagsAll(void)
{
  dataChanged(index(0, 1), index(numFiles()-1, 1));
}

void FileList::emitChangedTagsOne(int idx)
{
  dataChanged(index(idx, 1), index(idx+1, 1));
}

void FileList::flushJobs(void)
{
  QMutexLocker L(&dm_pendingjobs_lock);

  dm_pendingjobs.clear();
}

void FileList::enqueueLoad(const QString &fullfilename, int idx)
{
  //if (dm_pendingjobs.count(fullfilename) > 0)
    //return;

  // always enqueue a new task for this, so that its the higest priority
  // it doesnt matter that it might be already running
  QMutexLocker L(&dm_pendingjobs_lock);
  dm_pendingjobs.insert(fullfilename);

  RunnableEventFunction::enqueueWorker(
      std::bind(std::mem_fn(&FileList::metaLoaderFunc), this,
        fullfilename, idx),
      RunnableEvent::nextPriority());
}

void FileList::metaLoaderFunc(const QString &fullfilename, int idx)
{
  {
    QMutexLocker L(&dm_pendingjobs_lock);

    // check to see if this job was already done right from under us
    if (dm_pendingjobs.count(fullfilename) == 0)
      return;
  }

  // executed in background thread!
  QString outhash = hydra::calcFileHash(fullfilename);

  RunnableEventFunction::enqueueMain(
        std::bind(std::mem_fn(&FileList::metaLoaderFuncCommit), this,
        fullfilename, idx, outhash));
}

void FileList::metaLoaderFuncCommit(const QString &fullfilename, int idx, const QString &hash)
{
  // executed in the gui/main thread

  {
    QMutexLocker L(&dm_pendingjobs_lock);

    if (dm_pendingjobs.count(fullfilename) == 0)
      return;
    else
      dm_pendingjobs.erase(fullfilename);
  }

  Engine::instance()->addFile(fullfilename, &hash);
  const_cast<FileList*>(this)->emitChangedTagsOne(idx);
}

void FileList::emitBaseChange(FileListListener *source)
{
  //if (!isValid())
    //return;

  listeners_t::iterator ii;
  for (ii=dm_listeners.begin(); ii != dm_listeners.end(); ++ii)
    if (source != *ii)
      (*ii)->onBaseChange(this);
}

void FileList::emitImageChange(int fileIndex, FileListListener *source)
{
  listeners_t::iterator ii;
  for (ii=dm_listeners.begin(); ii != dm_listeners.end(); ++ii)
    if (source != *ii)
      (*ii)->onImageChange(this, fileIndex);
}

//
//
// FileEntryLessThan
//
//

FileEntryLessThan::FileEntryLessThan(short col, Qt::SortOrder order)
  : dm_col(col), dm_order(order)
{
}

bool FileEntryLessThan::operator()(const desktop::cache_ptr<desktop::FileEntry> &left, const desktop::cache_ptr<desktop::FileEntry> &right) const
{
  switch (dm_col) {
    case colJustName:
      if (dm_order == Qt::AscendingOrder)
        return left->justname() < right->justname();
      else
        return right->justname() < left->justname();
    case colFullFileName:
      if (dm_order == Qt::AscendingOrder)
        return left->fullfilename() < right->fullfilename();
      else
        return right->fullfilename() < left->fullfilename();
    case colRecordTags:
      if (dm_order == Qt::AscendingOrder)
        return tagsLessThan(left->recordItem().tags, right->recordItem().tags);
      else
        return tagsLessThan(right->recordItem().tags, left->recordItem().tags);
    case colFileLastModified:
      if (dm_order == Qt::AscendingOrder)
        return left->fileLastModified() < right->fileLastModified();
      else
        return right->fileLastModified() < left->fileLastModified();
    case colFileSize:
      if (dm_order == Qt::AscendingOrder)
        return left->fileSize() < right->fileSize();
      else
        return right->fileSize() < left->fileSize();
  }

  assert(false);
  return false;
}

bool FileEntryLessThan::tagsLessThan(const hydra::FileItemRecord::tags_t &left,
        const hydra::FileItemRecord::tags_t &right)
{
  // first, count the star values in each and compare on that
  int scoreLeft, scoreRight;

  scoreLeft = scoreValue(left);
  scoreRight = scoreValue(right);

  if (scoreLeft < scoreRight)
    return true;
  if (scoreRight < scoreLeft)
    return false;

  return left.size() < right.size();
}

//
//
// FileListLoader
//
//

// merge this from pixscore into a common base class?
namespace {
class Ticker
{
  public:
    Ticker(int ms);

    bool dequeueTick(void);

  private:
    QTime dm_time;
    int dm_ms;
};
}

Ticker::Ticker(int ms)
  : dm_ms(ms)
{
  dm_time.start();
}

bool Ticker::dequeueTick(void)
{
  if (dm_time.elapsed() > dm_ms) {
    dm_time.restart();
    return true;
  }
  return false;
}


FileListLoader::FileListLoader(FileList &filelist, FileListListener *source, bool resetSelectionTo0)
  : dm_filelist(filelist), dm_source(source), dm_resetselection(resetSelectionTo0)
{
  dm_changed_base = false;
  dm_changed_base_query = false;
  dm_changed_list = false;
  dm_changed_shuffle = false;

  dm_progdlg.reset(new QProgressDialog("Scanning image files", 0, 100, 0));
  dm_progdlg->setWindowTitle("Scanning image files");
  dm_progdlg->setWindowModality(Qt::ApplicationModal);
  dm_progdlg->setValue(20);
  //progdlg->setMinimumDuration(0);
}

FileListLoader::~FileListLoader()
{
  if (dm_changed_base && !dm_changed_list) {
    // only changed the base params? rebuild the file list
    // this will trigger the next if() too
    clearList();
    appendListDir(dm_filelist.dm_base_dir);
    dm_filelist.dm_watcher.setWatchDir(dm_filelist.dm_base_dir);
  }
  if (dm_changed_list || dm_changed_shuffle) {
    // sort or shuffle the list
    if (dm_filelist.isBaseShuffle())
      std::random_shuffle(dm_filelist.dm_all_files.begin(), dm_filelist.dm_all_files.end());
    else
      std::sort(dm_filelist.dm_all_files.begin(), dm_filelist.dm_all_files.end(), FileList::EntryListComp);

    dm_changed_list = true; // trigger the next secion
  }
  if (dm_changed_list || dm_changed_base || dm_changed_base_query) {
    // do a filter if need be
    if (dm_filelist.hasBaseQuery()) {
      std::shared_ptr<hydra::Token> q;
      Ticker ticker(1000);

      q = dm_filelist.baseQueryToken();

      dm_filelist.dm_filtered_files.clear();

      for (FileList::EntryList::const_iterator ii=dm_filelist.dm_all_files.begin(); ii != dm_filelist.dm_all_files.end(); ++ii) {
        if (q->isMatch((*ii)->recordItem().tags))
          dm_filelist.dm_filtered_files.push_back(*ii);

        if (ticker.dequeueTick()) {
          dm_progdlg->setLabelText((*ii)->justname());
          dm_progdlg->setValue(70);
        }
      }

    } else
      dm_filelist.dm_filtered_files = dm_filelist.dm_all_files;

    dm_changed_list = true; // trigger the next secion
  }

  if (dm_changed_list) {
    // emit some signals
    //qDebug () << "1";
    dm_filelist.reset();

    //qDebug () << "2";
    //dm_filelist.dm_selectionmodel.clear();
    //qDebug () << "3";
    //dm_filelist.dm_selectedfile = -1;
    //qDebug () << "4";

    if (dm_resetselection)
      dm_filelist.setSelectedFile(0);
  }
  if (dm_changed_base || dm_changed_base_query || dm_changed_shuffle) {
    dm_filelist.emitBaseChange(dm_source);
  }
}

void FileListLoader::setBaseDir(const QString &fulldirname)
{
  dm_filelist.dm_base_dir = fulldirname;

  dm_changed_base = true;
}

void FileListLoader::setBaseRecurse(bool recurse)
{
  dm_filelist.dm_base_recurse = recurse;

  dm_changed_base = true;
}

void FileListLoader::setBaseShuffle(bool shuffle)
{
  dm_filelist.dm_base_shuffle = shuffle;

  dm_changed_shuffle = true;
}

bool FileListLoader::setBaseQuery(const QString &query)
{
  if (!hydra::parseQueryTokens(query, dm_filelist.dm_base_query_token))
    return false;

  dm_filelist.dm_base_query = query;

  dm_changed_base_query = true;

  return true;
}

void FileListLoader::setReload(void)
{
  dm_changed_base = true;   // this will trigger a reload
}

void FileListLoader::clearList(void)
{
  dm_filelist.dm_all_files.clear();

  dm_changed_list = true;
}

int FileListLoader::appendListDir(const QString &dirname)
{
  int count = 0;
  Ticker ticker(1000);

  hydra::FileIterator I(dirname, false, dm_filelist.isBaseRecurse());

  while (I.hasNext()) {
    QString curfull(I.next());
    QString justname(hydra::justName(curfull));

    if (!hydra::isImageFile(justname))
      continue;

    dm_filelist.dm_all_files.push_back(dm_filelist.dm_cache->getEntry(curfull));

    if (ticker.dequeueTick()) {
      dm_progdlg->setLabelText(curfull);
      dm_progdlg->setValue(50);
    }

    //qDebug() << curfull;
    ++count;
  }

  dm_changed_list = true;

  return count;
}

//
//
// FileListReloader
//
//

FileListReloader::FileListReloader(FileList &filelist, FileListListener *source)
  : dm_filelist(filelist), dm_source(source)
{
}

class ReloadEntry
{
  public:
    const QString *fullfilename;
    bool foundMaker;
    size_t originalIndex;

    bool operator < (const ReloadEntry &rhs) const { return *fullfilename < *rhs.fullfilename; }
};

FileListReloader::~FileListReloader()
{
  // first, initialize all the cookies in the filelist

  ptrdiff_t i, k, sizei = dm_filelist.dm_all_files.size();
  bool did_something = false;
  std::vector<ReloadEntry> all_full_names;
  std::vector<ReloadEntry>::iterator foundii;
  std::vector<QString> new_files;
  std::vector<QString>::const_iterator iinew;
  std::set<cache_ptr<FileEntry> > nukeset;

  // load the sortable all_full_names array
  all_full_names.resize(sizei);
  for (i=0; i<sizei; ++i) {
    ReloadEntry &e = all_full_names[i];

    e.fullfilename = &dm_filelist.dm_all_files[i]->fullfilename();
    e.foundMaker = false;
    e.originalIndex = i;

    all_full_names[i] = e;
  }

  std::sort(all_full_names.begin(), all_full_names.end());

  // open the directory
  // TODO what to do with recusion?
  hydra::FileIterator I(dm_filelist.dm_base_dir, false, false);

  // read each file, compare with dm_all_files

  while (I.hasNext()) {
    QString curfull(I.next());
    QString justname(hydra::justName(curfull));

    if (!hydra::isImageFile(justname))
      continue;

    //qDebug() << "checking" << curfull;

    // check if we have this file name
    ReloadEntry targetentry;
    targetentry.fullfilename = &curfull;
    foundii = std::lower_bound(all_full_names.begin(), all_full_names.end(), targetentry);

    if (foundii != all_full_names.end() && foundii->fullfilename == curfull) {
      // fond it, make it off the list
      (*foundii).foundMaker = true;
    } else {
      // it's new, queue it for addition
      new_files.push_back(curfull);
    }
  }

  // sort the new file list
  std::sort(new_files.begin(), new_files.end());

  // remove OLD (do this backwards)
  for (i=sizei-1; i>=0; --i) {
    if (!all_full_names[i].foundMaker) {
      //qDebug() << "removefile: " << *all_full_names[i].fullfilename;
      nukeset.insert(dm_filelist.dm_all_files[
          all_full_names[i].originalIndex
          ]);

      dm_filelist.dm_all_files.erase(dm_filelist.dm_all_files.begin() + all_full_names[i].originalIndex);
    }
    //all_full_names[i] = &dm_filelist.dm_all_files[i]->fullfilename();
    //found_marker[i] = false;
  }

/*qDebug() << "dm_all_files";
for (i=0; i<dm_filelist.dm_all_files.size(); ++i)
  qDebug() << dm_filelist.dm_all_files[i]->fullfilename();
qDebug() << "dm_filtered_files";
for (i=0; i<dm_filelist.dm_filtered_files.size(); ++i)
  qDebug() << dm_filelist.dm_filtered_files[i]->fullfilename();*/

  // go through the filtered list and make sure all the elements are still
  // in the dm_all_files too
  /*k = 0;
  if (dm_filelist.dm_all_files.empty()) {
    dm_filelist.dm_filtered_files.clear();
qDebug() << "clear-all-filtered-files";
    did_something = true;
  } else
    for (i=0; i<dm_filelist.dm_all_files.size(); ++i) {
  qDebug() << "FOR" << i;
      if (k >= dm_filelist.dm_filtered_files.size())
        break;  // done, nothing left to filter
  qDebug() << "K" << k;
      while (k < dm_filelist.dm_filtered_files.size() &&
          dm_filelist.dm_all_files[i].get() != dm_filelist.dm_filtered_files[k].get()) {
  qDebug() << "filtered-removefile: " << dm_filelist.dm_all_files[i]->fullfilename();
        did_something = true;
        dm_filelist.dm_filtered_files.erase(dm_filelist.dm_filtered_files.begin() + k);
      }//while
      ++k;
    }//while
qDebug() << "done-half";*/
//REDO THE AVBOE

  /*k = 0;
  while (k < dm_filelist.dm_filtered_files.size()) {
    ReloadEntry targetentry;
    targetentry.fullfilename = &dm_filelist.dm_filtered_files[k]->fullfilename();

    foundii = std::lower_bound(all_full_names.begin(), all_full_names.end(), targetentry);
    bool keep_it = false;

    if (foundii != all_full_names.end() && *foundii->fullfilename == *targetentry.fullfilename) {
      if ((*foundii).foundMaker)
        keep_it = true;
    }

    if (keep_it)
      ++k;
    else {
      did_something = true;
qDebug() << "filtered-removefile: " << dm_filelist.dm_filtered_files[k]->fullfilename();
      dm_filelist.dm_filtered_files.erase(dm_filelist.dm_filtered_files.begin() + k);
    }
  }*/

/*qDebug() << "NUKESET";
for (std::set<cache_ptr<FileEntry> >::iterator oo= nukeset.begin(); oo != nukeset.end(); ++oo)
qDebug() << (*oo)->fullfilename();
qDebug() << "/NUKESET";*/

  k = 0;
  while (k < dm_filelist.dm_filtered_files.size()) {
    bool keep_it = true;

    if (nukeset.count(dm_filelist.dm_filtered_files[k]) > 0)
      keep_it = false;

    if (keep_it)
      ++k;
    else {
      did_something = true;
qDebug() << "FileListReloader" << "filtered-removefile: " << dm_filelist.dm_filtered_files[k]->fullfilename();
      dm_filelist.dm_filtered_files.erase(dm_filelist.dm_filtered_files.begin() + k);
    }
  }

  // add NEW
  std::shared_ptr<hydra::Token> q;
  q = dm_filelist.baseQueryToken();

  for (iinew = new_files.begin(); iinew != new_files.end(); ++iinew) {
    desktop::cache_ptr<FileEntry> entry = dm_filelist.dm_cache->getEntry(*iinew);
    dm_filelist.dm_all_files.push_back(entry);

//qDebug() << "newfile: " << *iinew;
    if (!dm_filelist.hasBaseQuery() || q->isMatch(entry->recordItem().tags)) {
qDebug() << "FileListReloader" << "filtered-newfile: " << *iinew;
      did_something = true;
      dm_filelist.dm_filtered_files.push_back(entry);
    }
  }

  if (!did_something)
    return;

  // make any signals
  // and reselect the file

  int selfile = dm_filelist.selectedFileIndex();

  // copied from FileListLoader
  dm_filelist.reset();

  if (selfile != -1 && dm_filelist.numFiles() > 0) {
    if (selfile >= dm_filelist.numFiles())
      selfile = dm_filelist.numFiles() - 1;
    dm_filelist.setSelectedFile(selfile);
  }
}

