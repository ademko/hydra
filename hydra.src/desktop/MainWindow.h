
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRADESKTOP_MAINWINDOW_H__
#define __INCLUDED_HYDRADESKTOP_MAINWINDOW_H__

#include <hydra/TR1.h>

#include <QMainWindow>

#include <hydra/Registry.h>
#include <desktop/FileList.h>
#include <desktop/ImageCache.h>
#include <desktop/ThumbCache.h>
#include <desktop/View.h>
#include <desktop/RunnableEvent.h>
#include <desktop/FutureLoader.h>

namespace desktop
{
  class MainPlugin;

  class MainImageView;  //fwd

  class MainState;
  class MainWindow;
}

class QStackedWidget; //fwd

class desktop::MainPlugin
{
  public:
    typedef hydra::Registry<MainPlugin> registry_type;
    static registry_type registry;
  public:
    enum {
      // when menu it should be in
      menuSelection = 1,
      menuView,
      menuPlugin,
    };
  public:
    virtual ~MainPlugin() { }

    virtual QString description(void) const = 0;
    virtual int menuType(void) const { return menuSelection; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList *filelist) = 0;
};

/**
 * This is shared state by all the MainWindows
 *
 * @author Aleksander Demko
 */ 
class desktop::MainState
{
  private:
    desktop::RunnableEventProcessor dm_runnableeventprocessor;

  public:
    //FileItemCache dm_fileitemcache;
    FileEntryCache dm_fileentrycache;
    ImageCache dm_imagecache;
    ThreadedThumbCache dm_thumbcache;

    FileList dm_leftlist;//, dm_rightlist;

    FutureLoader dm_futureloader;

  public:
    /// ctor
    MainState(void);
    /// dtor
    ~MainState();
};

class desktop::MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    static const short FULL_VIEW = 0;
    static const short VCR_VIEW = 1;
    static const short STRIP_VIEW = 2;
    static const short CATALOG_VIEW = 3;
    static const short LISTIMAGE_VIEW = 4;    // aka Folder and List
    static const short THUMBS_VIEW = 11;

  public:
    MainWindow(short initMode, std::shared_ptr<MainState> state
        = std::shared_ptr<MainState>(new MainState()));

    virtual ~MainWindow();

    MainState & mainState(void) const { return *dm_state; }
    std::shared_ptr<MainState> mainStatePtr(void) const { return dm_state; }

    // used by hydra.cpp, mostly

    desktop::FileList & leftList(void) { return dm_leftlist; }
    const desktop::FileList & leftList(void) const { return dm_leftlist; }

    //desktop::FileList & rightList(void) { return dm_rightlist; }
    //const desktop::FileList & rightList(void) const { return dm_rightlist; }

    void buildMenuFile(QMenu &outmenu);
    void buildMenuImage(QMenu &outmenu);
    void buildMenuSelection(QMenu &outmenu);
    void buildMenuView(QMenu &outmenu);
    void buildMenuPlugin(QMenu &outmenu);
    void buildMenuWindow(QMenu &outmenu);

    void popupWidgetAsDockWidget(QWidget *w);

    /**
     * Increment the current file, aswell as preload the next file
     * in the cache.
     *
     * In the future, this should be abtracted away to handle any
     * FileList or ImageCache
     *
     * @author Aleksander Demko
     */
    void advanceAndPreloadSelectedFile(void);

    /**
     * Sets the "last tag". The last tag is the last keyword
     * tag that was added to a record.
     *
     * @author Aleksander Demko
     */ 
    void setLastTag(const QString &lt) { dm_lasttag = lt; }

    /// returns the last tag
    const QString &lastTag(void) const { return dm_lasttag; }

  protected:
    QDockWidget * dockifyWidget(QWidget *v);
    virtual void timerEvent(QTimerEvent * event);

  public slots:
    void toggleFullScreen(void);
    void toggleSlideShow(void);

  private slots:
    void onCurrentChanged(void);
    void onAbout(void);
    void onQuit(void);
    void onMenuOpenDir(void);
    void onMenuOpenDirBrowser(void);
    void onMenuImage(void);
    void onMenuAnyPlugin(void);
    void onMenuWindow(void);
    void onMenuRotate(void);

    void onTimePreloadNextImage(void);

  private:
    void initActions(void);
    void buildMenuBar(QMenuBar &outmenu);
    void initGui(short initModevoid);
    void initPlugins(void);

    QWidget * initLayout(short ty, QWidget * &focuswidget);

  private:
    std::shared_ptr<MainState> dm_state;

    FileList &dm_leftlist;

    MainImageView *dm_fullscreenview;

    bool dm_slideshow_active;

    // menus
    QAction *dm_file_directory, *dm_file_directory_r, *dm_file_directory_popout,  *dm_file_about, *dm_file_quit;
    QAction *dm_fullscreen_action, *dm_slideshow_action;
    QAction *dm_rotate_reset, *dm_rotate_left, *dm_rotate_right;      // future: move this to an edit menu?
    QAction *dm_image_first, *dm_image_prev, *dm_image_next, *dm_image_last; // future: random, slideshow, full screen

    std::vector<QAction*> dm_selectionmenu, dm_viewmenu, dm_pluginmenu, dm_windowmenu;

    // plugins
    std::vector<std::shared_ptr<MainPlugin> > dm_allplugins;

    // the last applied tag
    QString dm_lasttag;
};

#endif

