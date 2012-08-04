
/*
 * Copyright (c) 2010-2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/MainWindow.h>

#include <assert.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QMenuBar>
#include <QSplitter>
#include <QFileDialog>
#include <QPushButton>
#include <QKeySequence>
#include <QTimer>
#include <QStackedWidget>
#include <QDockWidget>
#include <QThreadPool>
#include <QDebug>

#include <hydra/FileIterator.h>
#include <hydra/Engine.h>
#include <hydra/RotateCode.h>

#include <desktop/MainImageView.h>
#include <desktop/VCRView.h>
#include <desktop/InfoView.h>
#include <desktop/ThumbsView.h>
#include <desktop/ListView.h>
#include <desktop/WideListView.h>
#include <desktop/QueryView.h>
#include <desktop/ShuffleView.h>
#include <desktop/SearchView.h>
#include <desktop/TagEditorView.h>
#include <desktop/DirView.h>
#include <desktop/RunnableEvent.h>

using namespace hydra;
using namespace desktop;

MainPlugin::registry_type MainPlugin::registry;

MainState::MainState(void)
  : dm_leftlist(&dm_fileentrycache),
  dm_futureloader(&dm_thumbcache, &dm_leftlist)
{
}

MainState::~MainState()
{
  dm_thumbcache.flushJobs();
  dm_leftlist.flushJobs();
}

//
//
// MainWindow
//
//

MainWindow::MainWindow(short initMode, std::shared_ptr<MainState> state)
  : dm_state(state), dm_leftlist(dm_state->dm_leftlist),
    dm_fullscreenview(0), dm_slideshow_active(false)
{
  //dm_leftlist.setPeer(&dm_rightlist);
  //dm_rightlist.setPeer(&dm_leftlist);

  //setMinimumSize(600, 400);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Hydra Desktop");

  dm_lasttag = "tag";

  initPlugins();

  initActions();
  initGui(initMode);

  // init the title
  onCurrentChanged();

  // this is not needed as the next connect() covers this case too
  //connect(dm_leftlist.selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(onCurrentChanged()));
  // needed to keep the counters up to date too
  connect(dm_leftlist.selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onCurrentChanged()));

  startTimer(8000);
}

MainWindow::~MainWindow()
{
  // remove all the widgets from 
  // this needs to be done before the caches are destroyed
  setCentralWidget(0);
}

void MainWindow::buildMenuFile(QMenu &outmenu)
{
  outmenu.addAction(dm_file_directory);
  outmenu.addAction(dm_file_directory_r);
  //outmenu.addSeparator();
  outmenu.addAction(dm_file_directory_popout);
  outmenu.addSeparator();
  outmenu.addAction(dm_file_about);
  outmenu.addSeparator();
  outmenu.addAction(dm_file_quit);
}

void MainWindow::buildMenuImage(QMenu &outmenu)
{
  outmenu.addAction(dm_image_first);
  outmenu.addAction(dm_image_prev);
  outmenu.addAction(dm_image_next);
  outmenu.addAction(dm_image_last);
  outmenu.addSeparator();
  outmenu.addAction(dm_fullscreen_action);
  outmenu.addAction(dm_slideshow_action);

  // move the following to an Edit menu eventually, I think
  outmenu.addSeparator();
  outmenu.addAction(dm_rotate_reset);
  outmenu.addAction(dm_rotate_left);
  outmenu.addAction(dm_rotate_right);
}

void MainWindow::buildMenuSelection(QMenu &outmenu)
{
  for (int x=0; x<dm_selectionmenu.size(); ++x)
    outmenu.addAction(dm_selectionmenu[x]);
}

void MainWindow::buildMenuView(QMenu &outmenu)
{
  for (int x=0; x<dm_viewmenu.size(); ++x)
    outmenu.addAction(dm_viewmenu[x]);
}

void MainWindow::buildMenuPlugin(QMenu &outmenu)
{
  if (dm_pluginmenu.empty()) {
    QAction *a = outmenu.addAction("(no plug-ins installed)");
    a->setEnabled(false);
    return;
  }
  for (int x=0; x<dm_pluginmenu.size(); ++x)
    outmenu.addAction(dm_pluginmenu[x]);
}

void MainWindow::buildMenuWindow(QMenu &outmenu)
{
  for (int x=0; x<dm_windowmenu.size(); ++x)
    outmenu.addAction(dm_windowmenu[x]);
}

void MainWindow::popupWidgetAsDockWidget(QWidget *w)
{
  QDockWidget *dw = new QDockWidget(this);
  dw->setWidget(w);

  addDockWidget(Qt::TopDockWidgetArea, dw);
  dw->setFloating(true);
  dw->setWindowTitle(w->windowTitle());
}

void MainWindow::advanceAndPreloadSelectedFile(void)
{
  int next = dm_leftlist.selectedFileIndex()+1;

  if (next >= dm_leftlist.numFiles())
    next = 0;

  dm_leftlist.setSelectedFile(next);

  QTimer::singleShot(100, this, SLOT(onTimePreloadNextImage()));
}

QDockWidget * MainWindow::dockifyWidget(QWidget *v)
{
  QDockWidget *dw = new QDockWidget(this);
  dw->setWidget(v);
  dw->setWindowTitle(v->windowTitle());

  return dw;
}

void MainWindow::timerEvent(QTimerEvent * event)
{
  if (!dm_slideshow_active)
    return;

  dm_leftlist.setSelectedFile(dm_leftlist.selectedFileIndex() + 1);
}

void MainWindow::toggleFullScreen(void)
{
  if (dm_fullscreenview) {
    delete dm_fullscreenview;
    dm_fullscreenview = 0;
  } else {
    dm_fullscreenview = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache);

    dm_fullscreenview->showFullScreenMode();
  }

  dm_fullscreen_action->setChecked(dm_fullscreenview != 0);
}

void MainWindow::toggleSlideShow(void)
{
  dm_slideshow_active = !dm_slideshow_active;
  dm_slideshow_action->setChecked(dm_slideshow_active);
}

void MainWindow::onCurrentChanged(void)
{
  if (!dm_leftlist.isValid())
    return;

  int numFiles = dm_leftlist.numFiles();
  int i;
  int numselected = 0;

  for (i=0; i<numFiles; ++i)
    if (dm_leftlist.selectionModel()->isSelected(dm_leftlist.index(i,0)))
      numselected++;

  QString newtitle = dm_leftlist.selectedFile().justname();

  newtitle += " #" + QString::number(dm_leftlist.selectedFileIndex() + 1);

  newtitle += " (";

  if (numselected > 1)
    newtitle += QString::number(numselected) + "/";
  newtitle += QString::number(numFiles) + ") - Hydra Desktop";

  setWindowTitle(newtitle);
}

#define COPYRIGHT QString::fromUtf8("\302\251")
void MainWindow::onAbout(void)
{
  QDialog d(this);
  QVBoxLayout *lay;
  QLabel *lab;
  QPushButton *but;

  d.setWindowTitle("About Hydra Desktop");

  lay = new QVBoxLayout;

  lab = new QLabel("Hydra Desktop");
  lab->setScaledContents(true);
  lay->addWidget(lab);
  lab = new QLabel("Copyright " + COPYRIGHT + " " HYDRA_COPYRIGHT_STRING ", Aleksander B. Demko");
  lay->addWidget(lab);
  lab = new QLabel("Version: " HYDRA_VERSION_STRING);
  lay->addWidget(lab);
  lab = new QLabel("Built: " __DATE__ " " __TIME__);
  lay->addWidget(lab);
  lab = new QLabel("Options:"
#if QT_POINTER_SIZE > 4
      " 64-bit"
#else
      " 32-bit"
#endif
#ifndef NDEBUG
      " Debug"
#else
      " Optimized"
#endif
      " Qt_" QT_VERSION_STR
      );
  lay->addWidget(lab);

  but = new QPushButton("Close");
  lay->addWidget(but);
  connect(but, SIGNAL(clicked()), &d, SLOT(accept()));

  d.setLayout(lay);

  //d.show();
  d.exec();
}

void MainWindow::onQuit(void)
{
  QApplication::quit();
}

void MainWindow::onMenuOpenDir(void)
{
  QString newdir(QFileDialog::getExistingDirectory(this, "Open a folder", dm_leftlist.baseDir()));
  int id = sender()->property("id").toInt();

  if (newdir.isEmpty())
    return;

  FileListLoader loader(dm_leftlist);

  loader.setBaseDir(newdir);
  loader.setBaseRecurse(id == 1);
}

void MainWindow::onMenuOpenDirBrowser(void)
{
  popupWidgetAsDockWidget(new DirView(&dm_leftlist));
}

void MainWindow::onMenuImage(void)
{
  int id = sender()->property("id").toInt();

  switch (id) {
    case 0:
      dm_leftlist.setSelectedFile(0);
      break;
    case 1:
      dm_leftlist.setSelectedFile(dm_leftlist.selectedFileIndex()-1);
      break;
    case 2:
      dm_leftlist.setSelectedFile(dm_leftlist.selectedFileIndex()+1);
      break;
    case 3:
      dm_leftlist.setSelectedFile(static_cast<int>(dm_leftlist.numFiles()) - 1);
      break;
  }
}

void MainWindow::onMenuAnyPlugin(void)
{
  int id = sender()->property("id").toInt();

  if (id>=0 && id < dm_allplugins.size()) {
    dm_allplugins[id]->go(this, &dm_leftlist);
    return;
  }
}

void MainWindow::onMenuWindow(void)
{
  int id = sender()->property("id").toInt();

  // id == -1 was previousLayout()
  assert(id >= 0);

  MainWindow *newmain = new MainWindow(id, dm_state);

  newmain->show();
}

void MainWindow::onMenuRotate(void)
{
  if (!dm_leftlist.isValid())
    return;

  // does tag editing

  FileEntry &entry(dm_leftlist.selectedFile());
  FileItemRecord &item(entry.recordItem());

  Engine::instance()->regetFileItem(item);

  bool imageChanged = false;
  bool did_something = false;

  int id = sender()->property("id").toInt();
  if (id == 0) {
    did_something = item.tags.eraseKey("rotate", &imageChanged);
  } else {
    // rotate operation
    int newCode = rotateCodeAdd(entry.rotateCode(), id==1 ? -1 : 1);

    did_something = item.tags.insertKey("rotate", QString::number(rotateCodeToDegrees(newCode)), &imageChanged);
  }

  if (did_something)
    Engine::instance()->saveFileItem(item, QDateTime::currentDateTime());

  if (imageChanged) {
    entry.reloadRotateCode();
    // the image changed (rotate operation or something)
    // invalidate it, causing all its files to be reloaded
    entry.reloadRotateCode();
    dm_leftlist.emitImageChange(dm_leftlist.selectedFileIndex());
  }

  dm_leftlist.emitChangedTagsOne(dm_leftlist.selectedFileIndex());
}

void MainWindow::onTimePreloadNextImage(void)
{
  if (!dm_leftlist.isValid())
    return;

  int next = dm_leftlist.selectedFileIndex()+1;

  if (next >= dm_leftlist.numFiles())
    next = 0;

  dm_state->dm_imagecache.getImage(dm_leftlist.fileAt(next).fullfilename());
}

void MainWindow::initActions(void)
{
  QAction *action;

  dm_file_directory = new QAction(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), "&Open Folder", this);
  dm_file_directory->setShortcuts(QKeySequence::Open);
  dm_file_directory->setStatusTip("Open a folder from disk");
  dm_file_directory->setProperty("id", 0);
  connect(dm_file_directory, SIGNAL(triggered(bool)), this, SLOT(onMenuOpenDir()));

  dm_file_directory_r = new QAction(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), "Open Folder (and &sub-folders)", this);
  dm_file_directory_r->setStatusTip("Open a folder from disk (and all its sub-folders)");
  dm_file_directory_r->setProperty("id", 1);
  connect(dm_file_directory_r, SIGNAL(triggered(bool)), this, SLOT(onMenuOpenDir()));

  dm_file_directory_popout = new QAction(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), "Open Folder Browser Window", this);
  dm_file_directory_popout->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
  connect(dm_file_directory_popout, SIGNAL(triggered(bool)), this, SLOT(onMenuOpenDirBrowser()));

  dm_file_about = new QAction("&About", this);
  dm_file_about->setStatusTip("About this application");
  connect(dm_file_about, SIGNAL(triggered(bool)), this, SLOT(onAbout()));

  dm_file_quit = new QAction("&Quit", this);
  dm_file_quit->setStatusTip("Quit the application");
  connect(dm_file_quit, SIGNAL(triggered(bool)), this, SLOT(onQuit()));

  dm_fullscreen_action = new QAction("&Fullscren view", this);
  dm_fullscreen_action->setStatusTip("Enter/exit full screen viewing mode");
  dm_fullscreen_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  dm_fullscreen_action->setCheckable(true);
  connect(dm_fullscreen_action, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen()));

  addAction(dm_fullscreen_action);

  dm_slideshow_action = new QAction("&Slide show", this);
  dm_slideshow_action->setStatusTip("Start/stop an automated slide show");
  dm_slideshow_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
  dm_slideshow_action->setCheckable(true);
  connect(dm_slideshow_action, SIGNAL(triggered(bool)), this, SLOT(toggleSlideShow()));

  addAction(dm_slideshow_action);

  dm_rotate_reset = new QAction("Rotate reset", this);
  dm_rotate_reset->setStatusTip("Reset the rotation");
  dm_rotate_reset->setProperty("id", 0);
  connect(dm_rotate_reset, SIGNAL(triggered(bool)), this, SLOT(onMenuRotate()));
  dm_rotate_left = new QAction("Rotate left", this);
  dm_rotate_left->setStatusTip("Reset the image left");
  dm_rotate_left->setProperty("id", 1);
  connect(dm_rotate_left, SIGNAL(triggered(bool)), this, SLOT(onMenuRotate()));
  dm_rotate_right = new QAction("Rotate right", this);
  dm_rotate_right->setStatusTip("Reset the image right");
  dm_rotate_right->setProperty("id", 2);
  connect(dm_rotate_right, SIGNAL(triggered(bool)), this, SLOT(onMenuRotate()));

  dm_image_first = new QAction(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward), "&First", this);
  dm_image_first->setStatusTip("Show the first image");
  dm_image_first->setProperty("id", 0);
  connect(dm_image_first, SIGNAL(triggered(bool)), this, SLOT(onMenuImage()));
  dm_image_prev = new QAction(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward), "&Previous", this);
  dm_image_prev->setStatusTip("Show the previous image");
  dm_image_prev->setProperty("id", 1);
  connect(dm_image_prev, SIGNAL(triggered(bool)), this, SLOT(onMenuImage()));
  dm_image_next = new QAction(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward), "&Next", this);
  dm_image_next->setStatusTip("Show the next image");
  dm_image_next->setProperty("id", 2);
  connect(dm_image_next, SIGNAL(triggered(bool)), this, SLOT(onMenuImage()));
  dm_image_last = new QAction(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward), "&Last", this);
  dm_image_last->setStatusTip("Show the last image");
  dm_image_last->setProperty("id", 3);
  connect(dm_image_last, SIGNAL(triggered(bool)), this, SLOT(onMenuImage()));

  dm_selectionmenu.reserve(12);
  dm_viewmenu.reserve(12);
  dm_pluginmenu.reserve(12);

  for (int x=0; x<dm_allplugins.size(); ++x) {
    action = new QAction(dm_allplugins[x]->description(), this);
    action->setProperty("id", x);

    switch (dm_allplugins[x]->menuType()) {
      case MainPlugin::menuSelection: dm_selectionmenu.push_back(action); break;
      case MainPlugin::menuView: dm_viewmenu.push_back(action); break;
      case MainPlugin::menuPlugin: dm_pluginmenu.push_back(action); break;
      default: assert(false);
    }
    connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuAnyPlugin()));
  }

  dm_windowmenu.reserve(6);

  /*action = new QAction("Previous Layout", this);
  action->setProperty("id", -1);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);*/

  // not that useful
  //action = new QAction("Full View", this);
  //action->setProperty("id", FULL_VIEW);
  //dm_windowmenu.push_back(action);
  //connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  action = new QAction("Image View", this);
  action->setProperty("id", VCR_VIEW);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);

  action = new QAction("Thumb Strip", this);
  action->setProperty("id", STRIP_VIEW);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);

  action = new QAction("Thumbnails", this);
  action->setProperty("id", THUMBS_VIEW);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);

  action = new QAction("Folder and List", this);
  action->setProperty("id", LISTIMAGE_VIEW);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);

  action = new QAction("Thumbnail Catalog", this);
  action->setProperty("id", CATALOG_VIEW);
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
  dm_windowmenu.push_back(action);
  connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));

  addAction(action);

  // not that useful
  //action = new QAction("Big List ", this);
  //action->setProperty("id", 10);
  //dm_windowmenu.push_back(action);
  //connect(action, SIGNAL(triggered(bool)), this, SLOT(onMenuWindow()));
}

void MainWindow::buildMenuBar(QMenuBar &outmenu)
{
  QMenu *menu;

  menu = new QMenu("&File");
  buildMenuFile(*menu);
  outmenu.addMenu(menu);

  menu = new QMenu("&Image");
  buildMenuImage(*menu);
  outmenu.addMenu(menu);

  menu = new QMenu("&Selection");
  buildMenuSelection(*menu);
  outmenu.addMenu(menu);

  menu = new QMenu("&Views");
  buildMenuView(*menu);
  outmenu.addMenu(menu);

  menu = new QMenu("&Plug-ins");
  buildMenuPlugin(*menu);
  outmenu.addMenu(menu);

  menu = new QMenu("&Window");
  buildMenuWindow(*menu);
  outmenu.addMenu(menu);
}

void MainWindow::initGui(short initMode)
{
  QWidget *focuswidget = 0;

  setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
  setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);

  setCentralWidget(initLayout(initMode, focuswidget));

  // build menu
  QMenuBar *bar = new QMenuBar;

  buildMenuBar(*bar);

  setMenuBar(bar);

  if (focuswidget)
    focuswidget->setFocus();
}

static bool allpluginLessthan(const std::shared_ptr<MainPlugin> &left,
    const std::shared_ptr<MainPlugin> &right)
{
  return left->description() < right->description();
}

void MainWindow::initPlugins(void)
{
  dm_allplugins.reserve(MainPlugin::registry.size());

  for (int x=0; x<MainPlugin::registry.size(); ++x)
    dm_allplugins.push_back(MainPlugin::registry.create(x));
  qSort(dm_allplugins.begin(), dm_allplugins.end(), allpluginLessthan);
}

QWidget * MainWindow::initLayout(short ty, QWidget * &focuswidget)
{
  QLayout *retlay = 0;

  switch (ty)
  {
    case FULL_VIEW:
      {
        return focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache);
      }
    case VCR_VIEW:
      {
        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache), 1);

        QHBoxLayout *sublay = new QHBoxLayout;
        sublay->addStretch(1);
        sublay->addWidget(new InfoView(&dm_leftlist, &dm_state->dm_imagecache));
        sublay->addStretch(1);
        lay->addLayout(sublay);

        lay->addWidget(new VCRView(this, &dm_leftlist));
        retlay = lay;
        break;
      }
    case STRIP_VIEW:
      {
        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache), 1);

        //lay->addWidget(new InfoView(&dm_leftlist));
        QHBoxLayout *sublay = new QHBoxLayout;
        sublay->addStretch(1);
        sublay->addWidget(new InfoView(&dm_leftlist, &dm_state->dm_imagecache));
        sublay->addStretch(1);
        lay->addLayout(sublay);

        lay->addWidget(new VCRView(this, &dm_leftlist));
        retlay = lay;

        addDockWidget(Qt::BottomDockWidgetArea, dockifyWidget(new ThumbsView(this, &dm_leftlist, &dm_state->dm_thumbcache, false)));
        break;
      }
    case CATALOG_VIEW:
      {
        addDockWidget(Qt::LeftDockWidgetArea, dockifyWidget(new ThumbsView(this, &dm_leftlist, &dm_state->dm_thumbcache, true)));

        QVBoxLayout *vlay = new QVBoxLayout;
        vlay->addWidget(focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache), 1);

        // remove this if the search vcr thing is not wanted
        QHBoxLayout *sublay = new QHBoxLayout;
        sublay->addStretch(1);
        sublay->addWidget(new SearchSelectView(&dm_leftlist));
        sublay->addStretch(1);
        vlay->addLayout(sublay);

        QWidget *vlay_widget = new QWidget;
        vlay_widget->setLayout(vlay);

        addDockWidget(Qt::RightDockWidgetArea, dockifyWidget(new TagEditorView(this, &dm_leftlist)));

        /*add this back if I want the filter list
        {
          QVBoxLayout *query_lay = new QVBoxLayout;
          QWidget *query_widget = new QWidget;

          query_lay->addWidget(new ShuffleView(&dm_leftlist));
          query_lay->addWidget(new QueryView(&dm_leftlist));
          //not sure if i should add this here to add it under the mainimageview
          //as in LISTIMAGE_VIEW
          //query_lay->addWidget(new SearchSelectView(&dm_leftlist));
          query_lay->addStretch(1);

          query_widget->setLayout(query_lay);
          query_widget->setWindowTitle("Filter");

          addDockWidget(Qt::RightDockWidgetArea, dockifyWidget(query_widget));
        }*/

        return vlay_widget;
      }
    case LISTIMAGE_VIEW:
      {
        QDockWidget *first, *second;

        addDockWidget(Qt::LeftDockWidgetArea, dockifyWidget(new DirView(&dm_leftlist)));
        addDockWidget(Qt::LeftDockWidgetArea, first = dockifyWidget(new ListView(this, &dm_leftlist)));
        addDockWidget(Qt::LeftDockWidgetArea, second = dockifyWidget(new ThumbsView(this, &dm_leftlist, &dm_state->dm_thumbcache, true)));

        tabifyDockWidget(first, second);

        QVBoxLayout *vlay = new QVBoxLayout;
        vlay->addWidget(focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache), 1);

        QHBoxLayout *sublay = new QHBoxLayout;
        //sublay->addWidget(new InfoView(&dm_leftlist));
        //sublay->addWidget(new VCRView(&dm_leftlist));
        sublay->addStretch(1);
        sublay->addWidget(new SearchSelectView(&dm_leftlist));
        sublay->addStretch(1);
        vlay->addLayout(sublay);

        QWidget *vlay_widget = new QWidget;
        vlay_widget->setLayout(vlay);

        addDockWidget(Qt::RightDockWidgetArea, first = dockifyWidget(new TagEditorView(this, &dm_leftlist)));

        {
          QVBoxLayout *query_lay = new QVBoxLayout;
          QWidget *query_widget = new QWidget;

          query_lay->addWidget(new ShuffleView(&dm_leftlist));
          query_lay->addWidget(new QueryView(&dm_leftlist));
          query_lay->addStretch(1);

          query_widget->setLayout(query_lay);
          query_widget->setWindowTitle("Filter");

          addDockWidget(Qt::RightDockWidgetArea, second = dockifyWidget(query_widget));
        }

        //tabifyDockWidget(first, second);

        return vlay_widget;
      }
    case 10:
      {
        QVBoxLayout *vlay = new QVBoxLayout;
        vlay->addWidget(focuswidget = new MainImageView(this, &dm_leftlist, &dm_state->dm_imagecache), 1);

        QHBoxLayout *sublay = new QHBoxLayout;
        sublay->addWidget(new InfoView(&dm_leftlist, &dm_state->dm_imagecache));
        sublay->addWidget(new VCRView(this, &dm_leftlist));
        vlay->addLayout(sublay);

        QWidget *vlay_widget = new QWidget;
        vlay_widget->setLayout(vlay);

        QSplitter *splitter = new QSplitter(Qt::Horizontal);
        splitter->addWidget(vlay_widget);
        splitter->addWidget(new WideListView(&dm_leftlist));

        return splitter;
      }
    case THUMBS_VIEW:
        // old basic view:
        //return new ThumbsView(this, &dm_leftlist, &dm_state->dm_thumbcache, true, true);
      {
        addDockWidget(Qt::LeftDockWidgetArea, dockifyWidget(new DirView(&dm_leftlist)));

        return new ThumbsView(this, &dm_leftlist, &dm_state->dm_thumbcache, true, true);
      }
    default: assert(false); return 0;
  }

  assert(retlay);

  QWidget *ret = new QWidget;

  ret->setLayout(retlay);

  return ret;
}

