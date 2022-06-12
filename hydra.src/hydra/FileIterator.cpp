
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/FileIterator.h>

#include <QDebug>
#include <QFileInfo>

using namespace hydra;

QString hydra::makeAbsolute(const QString &s) {
    return QFileInfo(s).canonicalFilePath();
}

QString hydra::justName(const QString &s) { return QFileInfo(s).fileName(); }

bool hydra::isNormalFile(const QString &justname) {
    return !justname.isEmpty() && justname[0] != '.' && justname[0] != '_' &&
           justname[0] != ',';
}

bool hydra::isNormalDirectory(const QString &justdirname) {
    return !justdirname.isEmpty() && justdirname[0] != '.';
}

bool hydra::isImageFile(const QString &justname) {
    QString ext(QFileInfo(justname).suffix().toUpper());

    return ext == "JPG" || ext == "PNG" || ext == "GIF" || ext == "BMP" || ext == "WEBP";
}

bool hydra::mkDir(const QString &name) {
    // this is so weird
    if (QFileInfo(name).isAbsolute())
        return QDir("/").mkdir(name);
    else
        return QDir().mkdir(name);
}

//
//
// FileIterator
//
//

FileIterator::FileIterator(const QString &fileOrDir, bool allfiles,
                           bool recurse)
    : dm_allfiles(allfiles) {
    dm_istraversing = true;

    if (QFileInfo(fileOrDir).isFile()) {
        dm_current = makeAbsolute(fileOrDir);
        dm_istraversing = false;
        return;
    }

    dm_iterator = std::make_unique<QDirIterator>(
        fileOrDir, (recurse ? QDirIterator::Subdirectories
                            : QDirIterator::NoIteratorFlags));

    loadNextFile();
}

QString FileIterator::next(void) {
    QString ret(dm_current);

    if (hasNext())
        loadNextFile();

    return ret;
}
void FileIterator::loadNextFile(void) {
    if (!dm_istraversing) {
        dm_current.clear();
        return;
    }

    while (dm_iterator->hasNext()) {
        QString f = dm_iterator->next();
        QFileInfo info(f);

        if (info.isFile() && (dm_allfiles || isNormalFile(info.fileName())) &&
            info.size() > 0) {
            dm_current = makeAbsolute(f);
            return;
        }
    }

    // don't have anymore to check
    dm_current.clear();
}
