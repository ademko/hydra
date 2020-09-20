
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/DB.h>

#include <assert.h>

#include <sqlite3.h>

#include <QDebug>

// is this the right version to check against?
#if SQLITE_VERSION_NUMBER >= 3003000
#define sqlite3_prepare_vX(a, b, c, d, e)                                      \
    sqlite3_prepare_v2((a), (b), (c), (d), (e))
#else
#define sqlite3_prepare_vX(a, b, c, d, e)                                      \
    sqlite3_prepare((a), (b), (c), (d), (e))
#endif

using namespace hydra;

//
//
// DB::Connection
//
//

DB::Connection::~Connection() {
    if (handle)
        sqlite3_close(
            handle); // this should be called even if there was an error on open
}

//
//
// DB::Statement
//
//

DB::Statement::~Statement() {
    if (handle)
        sqlite3_finalize(handle);
}

//
//
// DB::ResetOnExit
//
//

DB::ResetOnExit::~ResetOnExit() {
    assert(dm_handle);
    sqlite3_reset(dm_handle);
    sqlite3_clear_bindings(dm_handle);
}

// sizeof(wchar_t) == 4 under linux?
// maybe its better to stick with utf-8

//
//
// DB
//
//

DB::DB(const QString &filename, const QString &tablescope)
    : dm_connection(new Connection), dm_table(tablescope) {
#ifndef NDEBUG
    int ret =
#endif
        sqlite3_open(filename.toUtf8().constData(), &dm_connection->handle);

    dm_put_hotbuf.reserve(8 * 1024);
    // dm_get_hotbuf.reserve(8*1024);    // dm_get_hotbuf will always use
    // fromRawData

    assert(ret == 0); // switch this to exception throwing

    sqlite3_busy_timeout(dm_connection->handle, 5000); // 5 second busy timeout

    dm_connection->fileName = filename;

    // huge performance boost
    // risks data loss?
    // but probably required for the way we use/abuse sqlite
    sqlite3_exec(dm_connection->handle, "pragma synchronous=0", 0, 0, 0);
    sqlite3_exec(dm_connection->handle, "pragma journal_mode=off", 0, 0, 0);

    assert(!dm_table.isEmpty());

    initDB();
    initPrepared();
}

DB::DB(DB &linked_db, const QString &tablescope)
    : dm_connection(linked_db.dm_connection), dm_table(tablescope) {
    assert(!dm_table.isEmpty());

    dm_put_hotbuf.reserve(8 * 1024);
    // dm_get_hotbuf.reserve(8*1024);    // dm_get_hotbuf will always use
    // fromRawData

    initDB();
    initPrepared();
}

DB::~DB() {}

bool DB::contains(const char *key) {
    ResetOnExit R(dm_has_statement.handle);
    assert(key);

    sqlite3_bind_text(dm_has_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);

    int ret;
    ret = sqlite3_step(dm_has_statement.handle);
    return ret == SQLITE_ROW;
}

bool DB::erase(const char *key) {
    ResetOnExit R(dm_erase_statement.handle);
    assert(key);

    assert(key);
    sqlite3_bind_text(dm_erase_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);

#ifndef NDEBUG
    int ret =
#endif
        sqlite3_step(dm_erase_statement.handle);
    assert(ret == SQLITE_OK || ret == SQLITE_DONE);

    return 0 < sqlite3_changes(dm_connection->handle);
}

bool DB::insert(const char *key, const std::string &value) {
    ResetOnExit R(dm_put_statement.handle);
    assert(key);

    sqlite3_bind_text(dm_put_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);
    sqlite3_bind_blob(dm_put_statement.handle, 2, &value[0],
                      static_cast<int>(value.size()), SQLITE_STATIC);

    int ret;
    ret = sqlite3_step(dm_put_statement.handle);

    return (ret == SQLITE_OK || ret == SQLITE_DONE);
}

bool DB::get(const char *key, std::string &value) {
    ResetOnExit R(dm_get_statement.handle);
    assert(key);

    sqlite3_bind_text(dm_get_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);

    int ret;
    ret = sqlite3_step(dm_get_statement.handle);
    if (ret != SQLITE_ROW)
        return false;

    // ret is SQLITE_ROW

    int bytes = sqlite3_column_bytes(dm_get_statement.handle, 0);
    assert(bytes >= 0);

    if (bytes == 0)
        value.clear();
    else
        value.assign(static_cast<const char *>(
                         sqlite3_column_blob(dm_get_statement.handle, 0)),
                     bytes);

    return true;
}

bool DB::insert(const char *key, const hydra::Record &value) {
    assert(key);

    dm_put_hotbuf.resize(0);
    QDataStream out_stream(&dm_put_hotbuf, QIODevice::Truncate |
                                               QIODevice::WriteOnly |
                                               QIODevice::Unbuffered);

    try {
        out_stream << value;
    } catch (Record::Exception &) {
        return false;
    }

    ResetOnExit R(dm_put_statement.handle);
    // rather than call Put(.., std::string), we do this for performance)

    sqlite3_bind_text(dm_put_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);
    sqlite3_bind_blob(dm_put_statement.handle, 2, dm_put_hotbuf.constData(),
                      dm_put_hotbuf.size(), SQLITE_STATIC);

    int ret;
    ret = sqlite3_step(dm_put_statement.handle);

    return (ret == SQLITE_OK || ret == SQLITE_DONE);
}

bool DB::get(const char *key, hydra::Record &value) {
    ResetOnExit R(dm_get_statement.handle);
    assert(key);

    sqlite3_bind_text(dm_get_statement.handle, 1, key,
                      static_cast<int>(strlen(key)), SQLITE_STATIC);

    int ret;
    ret = sqlite3_step(dm_get_statement.handle);
    if (ret != SQLITE_ROW)
        return false;

    // ret is SQLITE_ROW

    // READ INTO A MEMORY BUFFER HERE

    int bytes = sqlite3_column_bytes(dm_get_statement.handle, 0);
    assert(bytes >= 0);

    if (bytes > 0) {
        const void *blobby = sqlite3_column_blob(dm_get_statement.handle, 0);
        char *buf_start = reinterpret_cast<char *>(const_cast<void *>(blobby));
        // char *buf_end = buf_start + bytes;

        dm_get_hotbuf = QByteArray::fromRawData(buf_start, bytes);
    } else
        dm_get_hotbuf = QByteArray(); // make it valid, but empty

    try {
        QDataStream in_streamer(&dm_get_hotbuf,
                                QIODevice::ReadOnly | QIODevice::Unbuffered);

        in_streamer >> value;

        if (in_streamer.status() != QDataStream::Ok)
            throw Record::IOException();

        return true;
    } catch (Record::Exception &) {
        return false;
    }
}

void DB::initDB(void) {
    // std::string stmt("create table if not exists " + dm_table + " (key text
    // not null primary key, value blob not null)");
    std::string stmt("create table " +
                     std::string(dm_table.toUtf8().constData()) +
                     " (key text not null primary key, value blob not null)");
    // int ret;

    // ret =
    sqlite3_exec(dm_connection->handle, stmt.c_str(), 0, 0, 0);
    // assert(!ret);   // this may fail for older versions that dont support "if
    // not exists", and failing in this case is ok
}

void DB::initPrepared(void) {
    std::string stmt_string;
    std::string table(dm_table.toUtf8());
#ifndef NDEBUG
    int ret;
#endif

    // has
    stmt_string = "select key from " + table + " where key = ?";
#ifndef NDEBUG
    ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_has_statement.handle, 0);
    assert(ret == 0);
    assert(dm_has_statement.handle);

    // put
    stmt_string =
        "insert or replace into " + table + " (key, value) values (?,?)";
#ifndef NDEBUG
    ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_put_statement.handle, 0);
    assert(ret == 0);
    assert(dm_put_statement.handle);

    // get
    stmt_string = "select value from " + table + " where key = ?";
#ifndef NDEBUG
    ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_get_statement.handle, 0);
    assert(ret == 0);
    assert(dm_get_statement.handle);

    // erase
    stmt_string = "delete from " + table + " where key = ?";
#ifndef NDEBUG
    ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_erase_statement.handle, 0);
    assert(ret == 0);
    assert(dm_erase_statement.handle);
}

//
//
// Cursor
//
//

Cursor::Cursor(hydra::DB &db)
    : dm_isvalid(false), dm_connection(db.dm_connection) {
    std::string stmt_string("select key, value from ");

    stmt_string += db.dm_table.toUtf8().constData();

#ifndef NDEBUG
    int ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_select_statement.handle, 0);
    assert(ret == 0);
    assert(dm_select_statement.handle);
}

Cursor::Cursor(hydra::DB &db, const QString &prefix)
    : dm_isvalid(false), dm_connection(db.dm_connection) {
    std::string stmt_string("select key, value from ");

    stmt_string += db.dm_table.toUtf8().constData();

    stmt_string += " where key glob ?";

#ifndef NDEBUG
    int ret =
#endif
        sqlite3_prepare_vX(dm_connection->handle, stmt_string.c_str(),
                           static_cast<int>(stmt_string.size()),
                           &dm_select_statement.handle, 0);
    assert(ret == 0);
    assert(dm_select_statement.handle);

    dm_prefix_string = (prefix + "*").toUtf8(); // should prefix be sanitized?

    sqlite3_bind_text(dm_select_statement.handle, 1,
                      dm_prefix_string.constData(), dm_prefix_string.size(),
                      SQLITE_STATIC);

    // qDebug() << "PREFIX=" << dm_prefix_string;
}

bool Cursor::next(void) {
    int ret;

    ret = sqlite3_step(dm_select_statement.handle);

    dm_isvalid = ret == SQLITE_ROW;

    return dm_isvalid;
}

const char *Cursor::getKey(void) {
    assert(dm_isvalid);
    return reinterpret_cast<const char *>(
        sqlite3_column_text(dm_select_statement.handle, 0));
}

void Cursor::get(std::string &value) {
    assert(dm_isvalid);

    int bytes = sqlite3_column_bytes(dm_select_statement.handle, 1);
    assert(bytes >= 0);

    if (bytes == 0)
        value.clear();
    else
        value.assign(static_cast<const char *>(
                         sqlite3_column_blob(dm_select_statement.handle, 1)),
                     bytes);
}

void Cursor::get(QString &value) {
    assert(dm_isvalid);

    int bytes = sqlite3_column_bytes(dm_select_statement.handle, 1);
    assert(bytes >= 0);

    if (bytes == 0)
        value.clear();
    else
        value = QString::fromUtf8(static_cast<const char *>(sqlite3_column_blob(
                                      dm_select_statement.handle, 1)),
                                  bytes);
}

bool Cursor::get(hydra::Record &value) {
    int bytes = sqlite3_column_bytes(dm_select_statement.handle, 1);
    assert(bytes >= 0);

    if (bytes > 0) {
        const void *blobby = sqlite3_column_blob(dm_select_statement.handle, 1);
        char *buf_start = reinterpret_cast<char *>(const_cast<void *>(blobby));
        // char *buf_end = buf_start + bytes;

        dm_get_hotbuf = QByteArray::fromRawData(buf_start, bytes);
        // buf->Seek(0, wxFromStart);
    } else
        dm_get_hotbuf = QByteArray(); // make it valid, but empty

    try {
        QDataStream in_streamer(&dm_get_hotbuf,
                                QIODevice::ReadOnly | QIODevice::Unbuffered);

        in_streamer >> value;

        if (in_streamer.status() != QDataStream::Ok)
            throw Record::IOException();

        return true;
    } catch (Record::Exception &) {
        return false;
    }
}
