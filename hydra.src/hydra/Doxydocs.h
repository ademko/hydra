
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

// this file is soley for doxygen
// it contains no code

/**
 \mainpage Table of Contents

 Core manuals:
  - \ref hydrapage

 Other manuals may also be available. See the "Related Pages" for a full list.
 */

/**
  \page hydrapage Hydra Library Reference

  This is the hydra library reference. It's still
  under construction.

  \section engineoversec Engine Overview

  The singleton \ref hydra::Engine class encapsulates
  the core connection to the hydra database.

  This engine class maintains three logic key-value
  databases, using the \ref hydra::DB class.

  The \ref hydra::DB represents a key-value database,
  currently implemented as a table in a SQLite file.
  This may change in the future.

  \section recordsoversec Records

  Hydra uses a key-value database concept for its storage.
  Keys are usually filenames, UUIDs or hashes. Values
  are always \ref hydra::Record classes that are serialized
  to the databsae.

  Currently, there are three record types:
  \ref hydra::FileItemRecord, \ref hydra::FileHashRecord
  and \ref hydra::FilePathRecord.
  These three records are stored in three \ref hydra::DB
  classes, all maintained by \ref hydra::Engine.
  You typically never access the \ref hydra::DB classes
  directly, and instead use all the methods provided
  by the \ref hydra::Engine class.
 */

