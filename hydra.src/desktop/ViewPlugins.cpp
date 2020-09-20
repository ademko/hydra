
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/DirView.h>
#include <desktop/ListView.h>
#include <desktop/MainImageView.h>
#include <desktop/MainWindow.h>
#include <desktop/QueryView.h>
#include <desktop/SearchView.h>
#include <desktop/TagEditorView.h>
#include <desktop/TagSelectView.h>
#include <desktop/ThumbsView.h>
#include <desktop/VCRView.h>

using namespace desktop;

class TagEditor2Plugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Tag Editor"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(
            new TagEditorView(mainwindow, filelist));
    }
};

static hydra::Register<TagEditor2Plugin> reg01__alt;

// not that useful
/*
class TagEditorPlugin : public desktop::MainPlugin
{
  public:
    virtual QString description(void) const { return "Old Tag Editor"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow, desktop::FileList
*filelist)
    {
      mainwindow->popupWidgetAsDockWidget(new TagEditorView(filelist));
    }
};

static hydra::Register<TagEditorPlugin> reg01;
*/

class FilterPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Filter by Tags"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new QueryView(filelist));
    }
};

static hydra::Register<FilterPlugin> reg02;

class TagSelectPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Select by Tags"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new TagSelectView(filelist));
    }
};

static hydra::Register<TagSelectPlugin> reg02_9384;

class SearchPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Search by Tags"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new SearchView(filelist));
    }
};

static hydra::Register<SearchPlugin> reg03;

class ListPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "File List"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new ListView(mainwindow, filelist));
    }
};

static hydra::Register<ListPlugin> reg009812;

class DirPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Folders"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new DirView(filelist));
    }
};

static hydra::Register<DirPlugin> reg234997;

class ThumbsPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Thumbnails"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(
            new ThumbsView(mainwindow, filelist,
                           &mainwindow->mainState().dm_thumbcache, true));
    }
};

static hydra::Register<ThumbsPlugin> reg7712;

class ThumbsStripPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Thumb Strip"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(
            new ThumbsView(mainwindow, filelist,
                           &mainwindow->mainState().dm_thumbcache, false));
    }
};

static hydra::Register<ThumbsStripPlugin> reg7712992;

class MainImagePlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Image"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        mainwindow->popupWidgetAsDockWidget(new MainImageView(
            mainwindow, filelist, &mainwindow->mainState().dm_imagecache));
    }
};

static hydra::Register<MainImagePlugin> reg7712bb;

class MainImageVCRPlugin : public desktop::MainPlugin {
  public:
    virtual QString description(void) const { return "Image with Controls"; }
    virtual int menuType(void) const { return menuView; }
    virtual void go(desktop::MainWindow *mainwindow,
                    desktop::FileList *filelist) {
        QVBoxLayout *lay = new QVBoxLayout;
        lay->addWidget(
            new MainImageView(mainwindow, filelist,
                              &mainwindow->mainState().dm_imagecache),
            1);

        QHBoxLayout *sublay = new QHBoxLayout;
        sublay->addStretch(1);
        // sublay->addWidget(new InfoView(filelist));
        // sublay->addStretch(1);
        lay->addLayout(sublay);

        lay->addWidget(new VCRView(mainwindow, filelist));

        QWidget *w = new QWidget;

        w->setWindowTitle("Image with Controls");
        w->setLayout(lay);

        mainwindow->popupWidgetAsDockWidget(w);
    }
};

static hydra::Register<MainImageVCRPlugin> reg7712bb_vcr;
