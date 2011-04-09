
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_FILELIST_H__
#define __INCLUDED_HYDRADESKTOP_FILELIST_H__

#include <assert.h>

#include <vector>

#include <QString>
#include <QMutex>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QProgressDialog>
#include <QFileSystemWatcher>

#include <hydra/TR1.h>
#include <hydra/Records.h>
#include <hydra/Query.h>

#include <desktop/FileEntry.h>
#include <desktop/FileEntryCache.h>

namespace desktop
{
  class FileEntryCache; //forward

  class FileList;
  class FileEntryLessThan;
  class FileListLoader;
  class FileListReloader;

  class FileListListener; //forward

  class FileSystemWatcher;
}

/**
 * A class that monitors the file system and updates the FileList as needed
 *
 * @author Aleksander Demko
 */ 
class desktop::FileSystemWatcher : public QObject
{
    Q_OBJECT

  public:
    FileSystemWatcher(FileList *parent);

    void setWatchDir(const QString &dir);

  private slots:
    void onDirChanged(void);
    void onTimer(void);

  private:
    FileList *dm_parent;
    QFileSystemWatcher dm_watcher;
    QString dm_currentdir;

    bool dm_queued;
    bool dm_requeue;
};

/**
 * Holds a file (image) list. Not all the images may be loaded.
 * Can be backed by a FileMonitor.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileList : public QAbstractListModel
{
  public:
    /// constructor
    FileList(FileEntryCache *_cache);
    /// destructor
    virtual ~FileList();

    // QAbstractListModel
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    // may be null
    FileList *peer(void) const { return dm_peer; }

    // sets the peer, may take null
    void setPeer(FileList *peer);

    // base stuff

    const QString & baseDir(void) const { return dm_base_dir; }
    bool isBaseRecurse(void) const { return dm_base_recurse; }
    bool isBaseShuffle(void) const { return dm_base_shuffle; }
    bool hasBaseQuery(void) const { return !dm_base_query.isEmpty(); }
    const QString & baseQueryString(void) const { return dm_base_query; }
    std::shared_ptr<hydra::Token> baseQueryToken(void) const { return dm_base_query_token; }

    // file stuff

    /**
     * Does the given index refer to a valid file entry?
     *
     * @author Aleksander Demko
     */ 
    bool isValid(int index) const { return index>=0 && index<dm_filtered_files.size(); }

    /**
     * Is there a currently selected file?
     *
     * @author Aleksander Demko
     */ 
    bool isValid(void) const { return isValid(selectedFileIndex()); }

    /**
     * Return the FileEntry at the given index
     *
     * @author Aleksander Demko
     */ 
    const FileEntry & fileAt(int index) const {
      assert(isValid(index));
      return *dm_filtered_files[index];
    }

    /**
     * Return the FileEntry at the given index
     *
     * @author Aleksander Demko
     */ 
    FileEntry & fileAt(int index) {
      assert(isValid(index));
      return *dm_filtered_files[index];
    }

    /**
     * Return the FileEntry at the given QModelIndex
     *
     * @author Aleksander Demko
     */ 
    const FileEntry & fileAt(const QModelIndex & index) const {
      return fileAt(index.row());
    }

    /**
     * Return the FileEntry at the given QModelIndex
     *
     * @author Aleksander Demko
     */ 
    FileEntry & fileAt(const QModelIndex & index) {
      return fileAt(index.row());
    }

    /**
     * Returns the number of files in the list (basically a size() function)
     *
     * @author Aleksander Demko
     */ 
    int numFiles(void) const { return static_cast<int>(dm_filtered_files.size()); }

    /**
     * Returns the currently selected file's index.
     * Returns -1 if nothing is currently selected
     *
     * helper for selectionModel.
     *
     * @author Aleksander Demko
     */ 
    int selectedFileIndex(void) const;

    /**
     * Returns the currently selected file index, which must be valid.
     *
     * Also a helper for selectionModel()
     *
     * @author Aleksander Demko
     */ 
    FileEntry & selectedFile(void) { return fileAt(selectedFileIndex()); }

    /**
     * Finds the file index that has the given fullfilename.
     * Returns -1 on failure.
     *
     * @author Aleksander Demko
     */ 
    int findFileIndexByName(const QString &fullfilename) const;

    /**
     * Sets the currently selected file. Will do nothing if the index
     * is out of range, or is already the current file.
     * This will fire an event if the current file is changed.
     *
     * Helper for selectionModel()
     *
     * @author Aleksander Demko
     */
    void setSelectedFile(int index);

    // quickcopy an existing list
    //void quickCopy(FileList *src);

    // possibly obsolete, will keep around for now
    void addListener(FileListListener *v);
    // possibly obsolete, will keep around for now
    void removeListener(FileListListener *v);

    /**
     * Calls onBaseChange on all the Views that are listenting to the FileList.
     *
     * This works through thr FileListListener system.
     *
     * @author Aleksander Demko
     */ 
    void emitBaseChange(FileListListener *source = 0);

    /**
     * Calls onImageChange on all the Views that are listenting to the FileList.
     *
     * This is usually called when the image is changed, usually because its rotation
     * has been changed.
     *
     * This works through thr FileListListener system.
     *
     * @author Aleksander Demko
     */ 
    void emitImageChange(int fileIndex, FileListListener *source = 0);

    /**
     * Returns the internal selection model.
     * This keeps tracks of which files are selected (often more than one.)
     * Use isSelected(fileList()->index(x,0)) to test.
     *
     * @author Aleksander Demko
     */ 
    QItemSelectionModel * selectionModel(void) { return &dm_selectionmodel; }


    /**
     * Smits a signal for the data model that all the tags changed.
     * This translates into a QAbstractListModel::dataChanged() call.
     *
     * This works through the QAbstractListModel system.
     *
     * @author Aleksander Demko
     */ 
    void emitChangedTagsAll(void);

    /**
     * Smits a signal for the data model that all the tags changed
     * for this particular index.
     * This translates into a QAbstractListModel::dataChanged() call.
     *
     * This works through the QAbstractListModel system.
     *
     * @author Aleksander Demko
     */ 
    void emitChangedTagsOne(int idx);

    /// signals all the jobs to die quickly
    void flushJobs(void);

    /**
     * Using the worker/background thread pool system, enqueue
     * a file info load for the given file.
     *
     * Upon completion, a tag emit event will be sent.
     *
     * @author Aleksander Demko
     */ 
    void enqueueLoad(const QString &fullfilename, int idx);

  private:
    static inline bool EntryListComp(const desktop::cache_ptr<FileEntry> &lhs,
        const desktop::cache_ptr<FileEntry> &rhs) {
      return lhs->fullfilename() < rhs->fullfilename();
    }

    void metaLoaderFunc(const QString &fullfilename, int idx);
    void metaLoaderFuncCommit(const QString &fullfilename, int idx, const QString &hash);

  private:
    friend class desktop::FileListLoader;
    friend class desktop::FileListReloader;

    // related to metaLoaderFunc
    QMutex dm_pendingjobs_lock;
    std::set<QString> dm_pendingjobs;

    FileEntryCache *dm_cache;

    FileList *dm_peer;

    QString dm_base_dir;
    bool dm_base_recurse, dm_base_shuffle;
    QString dm_base_query;
    std::shared_ptr<hydra::Token> dm_base_query_token;

    typedef std::vector<desktop::cache_ptr<FileEntry> > EntryList;
    EntryList dm_filtered_files, dm_all_files;

    typedef std::list<desktop::FileListListener *> listeners_t;
    listeners_t dm_listeners;

    QItemSelectionModel dm_selectionmodel;

    FileSystemWatcher dm_watcher;
};

/**
 * A flexible sorting functor suitable for passing to qSort.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileEntryLessThan
{
  public:
    enum {
      colJustName,
      colFullFileName,
      colRecordTags,
      colFileLastModified,
      colFileSize,
    };

  public:
    FileEntryLessThan(short col, Qt::SortOrder order = Qt::AscendingOrder);

    bool operator()(const desktop::cache_ptr<desktop::FileEntry> &left, const desktop::cache_ptr<desktop::FileEntry> &right) const;

    static bool tagsLessThan(const hydra::FileItemRecord::tags_t &left,
        const hydra::FileItemRecord::tags_t &right);

  private:
    short dm_col;
    Qt::SortOrder dm_order;
};

/**
 * A class that is used to populate and load entries into a FileList
 * while taking care of signal emission and consolidation.
 *
 * You instantite the class, and set various options. Note that the options are
 * by default, based on the existing filelist. Upon destruction, the list will be loaded
 * and signals sent.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileListLoader
{
  public:
    /// constructor
    /// future: progress dialog option/system?
    FileListLoader(FileList &filelist, FileListListener *source = 0, bool resetSelectionTo0 = true);
    /// destructor will commit the changes and emit some signals
    ~FileListLoader();

    /// sets the base dir
    void setBaseDir(const QString &fulldirname);
    /// sets weither directory recursion will be applied
    /// this may be removed in the future as dir-monitoting in this case
    /// may be impossible
    void setBaseRecurse(bool recurse = true);
    /// either to shuffle the loaded list
    void setBaseShuffle(bool shuffle = true);
    /// sets the query that will be used to filter the list
    /// returns true on succesful parse
    bool setBaseQuery(const QString &query);

    /// simply flag that the list, with its current params, needs to be reloaded
    void setReload(void);

    /// remove all files from the current list
    void clearList(void);

  private:
    /**
     * Reads a dir and sets the first file as current.
     *
     * @return the number of entries scanned
     * @author Aleksander Demko
     */ 
    int appendListDir(const QString &dirname);

  private:
    FileList &dm_filelist;
    FileListListener *dm_source;

    bool dm_resetselection;

    bool dm_changed_base;
    bool dm_changed_base_query;
    bool dm_changed_shuffle;
    bool dm_changed_list;

    std::auto_ptr<QProgressDialog> dm_progdlg;
};

/**
 * Does a "fast" update/merge with the files on disk.
 *
 * @author Aleksander Demko
 */ 
class desktop::FileListReloader
{
  public:
    /// constructor
    FileListReloader(FileList &filelist, FileListListener *source = 0);
    /// destructor
    ~FileListReloader();

  private:
    FileList &dm_filelist;
    FileListListener *dm_source;
};

#endif

