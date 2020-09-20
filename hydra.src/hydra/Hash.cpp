
/*
 * Copyright (c) 2005 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Hash.h>

#include <QCryptographicHash>
#include <QFile>

QString hydra::calcFileHash(const QString &filename) {
    QString ret;

    QFile inf(filename);

    if (!inf.open(QIODevice::ReadOnly))
        return ret;

    char buf[4096];
    int len;
    bool null_file = true;
    QCryptographicHash hasher(QCryptographicHash::Sha1);

    // do the hashing
    while ((len = inf.read(buf, 4096)) > 0) {
        null_file = false;
        hasher.addData(buf, len);
    }

    if (null_file)
        return ret;

    return hasher.result().toHex().constData();
}
