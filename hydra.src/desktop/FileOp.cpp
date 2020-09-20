
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/FileOp.h>

#include <assert.h>

#include <hydra/Engine.h>
#include <hydra/Records.h>

#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>

using namespace hydra;
using namespace desktop;

//
// FileOp
//

FileOp::~FileOp() {}

bool FileOp::opFile(int op, const QString &srcfile, const QString &destfile) {
    switch (op) {
    case Op_Del:
        return delFile(srcfile);
    case Op_Move:
        return moveFile(srcfile, destfile);
    case Op_Copy:
        return copyFile(srcfile, destfile);
    }
    assert(false);
    return false;
}

//
// FileOpGuiTest
//

FileOpGuiTest::FileOpGuiTest(QWidget *parent) : dm_parent(parent) {
    dm_overwrite_all = false;
    dm_overwrite_none = false;
    dm_skip_errors = false;
}

bool FileOpGuiTest::delFile(const QString &srcfile) {
    // there arent really any user interations for deletes
    return true;
}

bool FileOpGuiTest::moveFile(const QString &srcfile, const QString &destfile) {
    // the move gui logic is the same as the copy logic
    return copyFile(srcfile, destfile);
}

bool FileOpGuiTest::copyFile(const QString &srcfile, const QString &destfile) {
    QFileInfo srcinfo(srcfile), destinfo(destfile);
    const char *error_msg = 0;

    if (srcinfo == destinfo) {
        error_msg = "You cannot copy or move file onto itself: ";
        goto error_section;
    }

    if (destinfo.isDir()) {
        if (dm_overwrite_none)
            return false;
        error_msg = "Destination exists and is a directory: ";
        goto error_section;
    }

    // does the file exist?
    if (destinfo.exists()) {
        if (dm_overwrite_none)
            return false;

        if (!destinfo.isWritable()) {
            error_msg = "File exists, but is write-protected: ";
            goto error_section;
        }

        if (!dm_overwrite_all) {
            // exists and is writable, confirm with the user
            QMessageBox msg;
            QAbstractButton *yes_all, *no, *no_all, *cancel;

            msg.setText("The destination file already exists");
            msg.setInformativeText("The file " + destfile +
                                   " already exists. Overwrite it?");

            no_all = msg.addButton("Never overwrite", QMessageBox::AcceptRole);
            no = msg.addButton("Don't overwrite", QMessageBox::AcceptRole);
            yes_all = msg.addButton("Overwrite All", QMessageBox::AcceptRole);
            msg.addButton("Overwrite", QMessageBox::AcceptRole);
            cancel = msg.addButton(QMessageBox::Cancel);

            msg.exec();

            if (yes_all == msg.clickedButton())
                dm_overwrite_all = true;
            if (no == msg.clickedButton())
                return false;
            if (no_all == msg.clickedButton()) {
                dm_overwrite_none = true;
                return false;
            }
            if (cancel == msg.clickedButton())
                throw UserCanceled();
        }
    }

    // done!
    return true;

error_section:
    if (dm_skip_errors)
        return false;

    QMessageBox msg;
    QAbstractButton *skip, *skip_all, *cancel;

    msg.setText("File error");
    msg.setInformativeText(error_msg + destfile);

    skip_all = msg.addButton("Ignore all errors", QMessageBox::AcceptRole);
    skip = msg.addButton("Ignore", QMessageBox::AcceptRole);
    msg.addButton("Try Again", QMessageBox::AcceptRole);
    cancel = msg.addButton(QMessageBox::Cancel);

    msg.exec();

    if (skip == msg.clickedButton())
        return false;
    if (skip_all == msg.clickedButton()) {
        dm_skip_errors = true;
        return false;
    }
    if (cancel == msg.clickedButton())
        throw UserCanceled();

    // recursive retry otherwise
    return copyFile(srcfile, destfile);
}

bool FileOpGuiTest::delDir(const QString &dirname) {
    // there arent really any user interations for deletes
    return true;
}

//
// FileOpDisk
//

FileOpDisk::FileOpDisk(void) {}

bool FileOpDisk::delFile(const QString &srcfile) {
    return hydra::delFile(srcfile);
}

bool FileOpDisk::moveFile(const QString &srcfile, const QString &destfile) {
    hydra::delFile(destfile);
    return hydra::moveFile(srcfile, destfile);
}

bool FileOpDisk::copyFile(const QString &srcfile, const QString &destfile) {
    hydra::delFile(destfile);
    return hydra::copyFile(srcfile, destfile);
}

bool FileOpDisk::delDir(const QString &dirname) {
    return hydra::delDir(dirname);
}

//
// FileOpDB
//

FileOpDB::FileOpDB(void) {}

bool FileOpDB::delFile(const QString &srcfile) {
    return Engine::instance()->erasePath(srcfile);
}

bool FileOpDB::moveFile(const QString &srcfile, const QString &destfile) {
    FilePathRecord pathrec;
    int code = Engine::instance()->getFileItem(srcfile, 0, 0, &pathrec);

    // in a move operation, the srcfile probably already disapeared
    // (as we do the file operation first)
    // so we need to be mindfull of this
    if (!(code == Engine::Load_OK || code == Engine::Load_ErrorFileMissing))
        return false;

    Engine::instance()->erasePath(srcfile);

    // only do the add if it would be a fast, already-have-the-hash operation
    if (pathrec.hash.isEmpty())
        return false;
    else
        return Engine::instance()->addFile(destfile, &pathrec.hash);
}

bool FileOpDB::copyFile(const QString &srcfile, const QString &destfile) {
    FilePathRecord pathrec;

    if (Engine::instance()->getFileItem(srcfile, 0, 0, &pathrec) !=
        Engine::Load_OK)
        return false;

    return Engine::instance()->addFile(destfile, &pathrec.hash);
}

bool FileOpDB::delDir(const QString &dirname) {
    // dir information is not sotred in the db
    return true;
}

int desktop::guiFileOpFiles(QWidget *parent, int op,
                            const QStringList &srcfiles, const QString &destDir,
                            FileOp *handler) {
    int i = 0, done = 0;
    QString dest_name;

    assert(!srcfiles.empty());

    assert(handler == 0); // not yet tested functionality

    if (op != FileOp::Op_Del && !QFileInfo(destDir).isDir())
        return 0;

    const char *title = "Deleting Files";

    if (op == FileOp::Op_Move)
        title = "Moving Files";
    if (op == FileOp::Op_Copy)
        title = "Copying Files";

    QProgressDialog prog(title, "", 0, srcfiles.size(), parent);
    // dm_progdlg.setWindowTitle(title);
    prog.setWindowModality(Qt::ApplicationModal);

    FileOpGuiTest tester(&prog);
    FileOpDisk disker;
    FileOpDB dber;

    try {
        for (QStringList::const_iterator ii = srcfiles.begin();
             ii != srcfiles.end(); ++ii) {
            i++;

            prog.setLabelText(*ii);
            prog.setValue(i);

            if (op != FileOp::Op_Del)
                dest_name = destDir + "/" + QFileInfo(*ii).fileName();

            if (!tester.opFile(op, *ii, dest_name) ||
                !disker.opFile(op, *ii, dest_name))
                continue;

            // week on the error handling...
            dber.opFile(op, *ii, dest_name);

            // week on the error handling...
            if (handler)
                handler->opFile(op, *ii, dest_name);

            done++;
        }
    } catch (const FileOpGuiTest::UserCanceled &) {
    }

    return done;
}
