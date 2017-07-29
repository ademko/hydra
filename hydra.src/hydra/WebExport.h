
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_WEBEXPORT_H__
#define __INCLUDED_HYDRA_WEBEXPORT_H__

#include <QString>
#include <QUuid>
#include <QTextStream>

#include <stdint.h>
#include <vector>
#include <map>
#include <hydra/TR1.h>

namespace hydra
{
  class WebExport;
  class FileItemRecord; //forward
};

/**
 * A class that exports a collection pics for web site building.
 *
 * @author Aleksander Demko
 */ 
class hydra::WebExport
{
  public:
    /**
     * Export a site to the given website output directory.
     *
     * @author Aleksander Demko
     */ 
    WebExport(const QString &outputdir, QTextStream &out);

    /**
     * Sets the title of the exported web site
     *
     * @author Aleksander Demko
     */ 
    void setTitle(const QString &title);

    /**
     * Adds a given file to the output list.
     *
     * @author Aleksander Demko
     */ 
    void addFile(const QString &fullfilename, const QString &basedir,
        hydra::FileItemRecord &item, const QString &filehash);

    /**
     * Commit and actually build the web site.
     *
     * Returns the number of files made, or -1 on error.
     *
     * @author Aleksander Demko
     */ 
    int commitWebSite(void);

    /**
     * Commit and actually build the web site.
     *
     * Returns the number of files made, or -1 on error.
     *
     * @author Aleksander Demko
     */ 
    int commitFileCopy(void);
    
  private:
    struct DirEntry;
    struct FileEntry;

    // returns the full file name of the given thumbnail
    QString thumbFileName(const QString &hash, unsigned long w, unsigned long h);

    void addDirComponents(const QString &dir);

    // connects the parents to children in dm_basedirs
    void buildDirTree(void);

    void computeDirCounts(DirEntry &dir);

    int writeImageFiles(void);

    bool writeImageHtml(int myid, int randomId, int numpeers, const FileEntry &entry);
    void sortDirIndex(DirEntry &entry);
    bool writeDirIndex(const DirEntry &entry);
    
    /**
     * Call writeImageHtml for each subimages in the given directory
     */
    void writeAllImageHtmls(DirEntry &entry);

    void fileCopyMakeDirs(const QString dir = "");
    int fileCopyCopyFiles(void);

  private:
    QTextStream &dm_out;

    QString dm_outdir, dm_title;

    typedef std::vector<std::shared_ptr<DirEntry> > DirSet;

    static bool DirEntryLT(const std::shared_ptr<DirEntry> &lhs,
        const std::shared_ptr<DirEntry> &rhs);

    typedef std::vector<std::shared_ptr<FileEntry> > FileSet;

    static bool FileEntryLT(const std::shared_ptr<FileEntry> &lhs,
        const std::shared_ptr<FileEntry> &rhs);

    static void writeHydraImg(QTextStream &out, int id, const FileEntry &entry);

    struct DirEntry {
      bool isroot;
      ptrdiff_t totalfiles;

      QString basedir;       // the map KEY
      // derived:
      QString urlname;       // basedir, but suitable for url display
      QString justname;      // just the last name

      std::weak_ptr<DirEntry> parent;    // parent dir, null for the root node

      // they all store map keys, ofcourse
      DirSet subdirs;       // sub dirs
      FileSet subimages;     // images (thumbable) in this dir
      FileSet subfiles;      // non-image files in this dir

      DirEntry(const QString &_basedir);

    };

    typedef std::map<QString, std::shared_ptr<DirEntry> > DirMap;

    struct FileEntry {
      QUuid id;       // db id

      std::weak_ptr<DirEntry> parent;    // parent dir

      int rotateCode;        // rotattion to apply, if any

      QString fullfilename;  // full on disk filename
      QString justname;      // just the filename (as on disk)
      QString basedir;       // basic web path (the parent directory)
      QString basejustname;  // justname, but possibly adjusted for conflicts
      QString basefilename;  // the baedir + filename, also, this is the map KEY
      QString title;         // title from the db
      QString desc;          // desc from the db
      int filetype;          // same as in the item record
      QString filehash;      // binary hash for this file

      // these are all set by calc()
      QString urlnamebase;   // core url name (/ -> , substiution), from basefilename
      QString urlhtml;       // urlnamebase + .html
      QString urlorigimage;  // urlnamebase
      QString urlviewimage;  // VIEW, + urlnamebase
      QString urlthumbimage; // THUMB, + urlnamebase

      uint64_t original_file_size;    // in bytes

      void calc(void);        // calculates various computed strings after the others are set
    };

    typedef std::map<QString, std::shared_ptr<FileEntry> > FileMap;

    FileMap dm_basefiles;   // built incrementally when caller adds files

    DirMap dm_basedirs; // built from dm_basefiles upton BuildDirTree()
};

#endif

