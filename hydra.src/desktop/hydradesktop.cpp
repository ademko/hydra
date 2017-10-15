
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <QApplication>

#include <QFileInfo>
#include <QDebug>

#include <hydra/ArgumentParser.h>
#include <hydra/Engine.h>
#include <hydra/Thumb.h>
#include <hydra/FileIterator.h>

#include <desktop/MainWindow.h>

/*
 Loading logic

 case 0: no params
  load dir = .
  show first file
  thumb strip view
 case 1: a file
  load dir = parent_dir_of_file
  show that file
  single image view
 case 2: a dir
  load dir = that dir
  show first file
  list view

  multiple files?
*/

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  QString initDir("."), initFile;
  short initMode(desktop::MainWindow::STRIP_VIEW);

  hydra::ArgumentParser parser;

  parser.next();    // eat the program name

  if (parser.hasNext()) {
    initDir = hydra::makeAbsolute(parser.next());

    //qDebug() << "param " << initDir;
    if (QFileInfo(initDir).isDir()) {
      // case 2 (a dir)
      initMode = desktop::MainWindow::LISTIMAGE_VIEW;
    } else {
      // case 1 (a file)
      initMode = desktop::MainWindow::VCR_VIEW;
      initFile = initDir;
      initDir = QFileInfo(initDir).path();
    }
  }
    // else case 0 (nothing given)

  hydra::Engine eng;
  std::shared_ptr<desktop::MainState> state(new desktop::MainState);
  desktop::MainWindow *mw = new desktop::MainWindow(initMode, state);

  hydra::Thumb::mkThumbDir();

  mw->showMaximized();

  //app.processEvents();

  //qDebug() << "initDir " << initDir;
  //qDebug() << "initFile " << initFile;

  {
    desktop::FileListLoader loader(mw->leftList(), 0, true);

    loader.setBaseDir(initDir);
  }

  if (!initFile.isEmpty())
    mw->leftList().setSelectedFile(mw->leftList().findFileIndexByName(initFile));

  return app.exec();
}

