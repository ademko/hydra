
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_FILEOP_H__
#define __INCLUDED_HYDRA_FILEOP_H__

#include <QString>

namespace hydra {
// returns true on success
bool linkFile(const QString &srcfile, const QString &destfile);
// returns true on success
bool moveFile(const QString &srcfile, const QString &destfile);
// returns true on success
bool delFile(const QString &srcfile);
// returns true on success
bool copyFile(const QString &srcfile, const QString &destfile);
/**
 * A "smart" copy. If the file exists, this function will fail.
 * Otherwise, it will try to LINK the file. If that files,
 * it will do a copy operation
 *
 * @return true on success
 * @author Aleksander Demko
 */
bool smartCopy(const QString &srcfile, const QString &destfile);
// returns true on success
bool delDir(const QString &dirpath);
} // namespace hydra

#endif
