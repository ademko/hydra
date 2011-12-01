
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

// this file is soley for doxygen
// it contains no code

/**
  \page wexususerpage Wexus User Reference

  This page contains help on various modules and
  parameters available in the stock wexusserver
  system.

  \section wexusserversec wexusserver program

  You run wexusserver like so:
  \code
    wexusserver sitedir
  \endcode

  If the sitedir doesn't contain any site.ini or app.ini files,
  it will be hosted using the basic file server module.

  \section siteinisec site.ini

  The root sitedir may contain a \b site.ini file. This file contains site-wide
  options.

    - \b httpport (default 8080) The http port to use.
    - \b httpthreads (default 4) The default number of worker threads to use.


  \section appinisec app.ini

  Any subdirectory of the sitedir may have an \b app.ini file.
  This will launch one or more apps in the give direectory.

  The ini file has sections, one per app. You start a new section with [brackets].
  You can put a descriptive name within the [brackets]. The apps
  will be loaded in a sorted order.

  Common user settings:
   - \b app Required. The app type to launch. See \ref wexusappspage
   - \b headerdir (default headers/) Sets common files below.

  Common files:
   - \b database.sqlite The default sqlite3 database to use for core functions.
   - \b headers/ directory. Can contain \b header.html and \b footer.html
     which will be used as a common header and footer for all output.

  For programmers, there are some computed fields to: \b mountpoint (e.g. blog),
  \b appdir (the current dir), and \b sitedir (the sitedir)


  Example app.ini:

  \code

  [1redirector]

  app = ca.demko.redirect

  link1 = "/src dest"

  [2fileer]

  app = ca.demko.file

  [3mainblooger]

  app = ca.demko.blog

  \endcode

 */

/**
  \page wexusappspage Wexus Apps

  Stock apps that can be started by app.ini.

  \section cademkofileapp Static File Server

  Type \b ca.demko.file

  Serves static files directory the user. Useful for .html, .jpg
  (and other images), etc.

  Options:
   - \b dir1 \b dir2 \b dir3 etc. (default dir1=.) One or more directories to server.
     They are all relative to the app.ini directory.
   - \b options1 etc. (default empty). Provides a space-seperated list
     of options for each corresponding directory. This string can contain
     any of the following:
       - \b IndexHtml Show index.html files if present.
       - \b AutoDirIndex Automatically generate directory listings when needed.
       - \b AllowAllMimeTypes Server all files, not just a safe subset.

  \section cademkoredirectapp Redirector

  Type \b ca.demko.redirect

  Redirects some urls to some other urls. Useful for transitions
  and building permanent links.

  Options:
   - \b link1 \b link2 etc. (no default, e.g. "/src dest").
     Specifies one or more redirect mappings. The mappins are of
     the form "/src dest". This will remap the given \b src
     url to the \b dest url. The dest url can be relative or
     absolute.
 
 \section cademkoblogapp Blog Host

 Type \b ca.demko.blog

 A simply web log (blog) server.

 This will server all the markdown-formatted \b .txt files in
 the \b appdir as blog entries. The filenames should be of the
 form \b YYYYMM_title.txt or \b YYYYMMDD_title.txt

 */

