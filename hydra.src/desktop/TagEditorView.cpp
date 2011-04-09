
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/TagEditorView.h>

#include <QDebug>

#include <hydra/Engine.h>
#include <desktop/MainWindow.h>

using namespace hydra;
using namespace desktop;

class MyCheckBox : public QCheckBox
{
  public:
    MyCheckBox(const QString &title)
      : QCheckBox(title) { }

  protected:
    virtual void nextCheckState(void) {
      setCheckState(
          checkState() == Qt::Unchecked ?
          Qt::Checked : Qt::Unchecked
          );
    }

};

TagEditorView::TagEditorView(MainWindow *mainwin, FileList *_filelist)
  : View(mainwin, _filelist)
{
  setWindowTitle("Tag Editor");
  initGui();
  updateEditor();
}

void TagEditorView::onImageChange(FileList *fl, int fileIndex)
{
  if (fileIndex != fl->selectedFileIndex())
    return;
  updateEditor();
}

void TagEditorView::updateEditor(void)
{
  typedef std::map<QString, int> union_type;
  union_type tag_union;
  int totalSelectedFiles = 0;

  for (std::set<QString>::const_iterator ii=dm_recently_added.begin(); ii != dm_recently_added.end(); ++ii)
    tag_union[*ii] = 0;

  for (int i=0; i<fileList()->numFiles(); ++i)
    if (fileList()->selectionModel()->isSelected(fileList()->index(i,0))) {
      hydra::FileItemRecord::tags_t &tags = fileList()->fileAt(i).recordItem().tags;

      ++totalSelectedFiles;
      //qDebug() << fileList()->fileAt(i).fullfilename();
      for (hydra::FileItemRecord::tags_t::const_iterator ii=tags.begin(); ii != tags.end(); ++ii) {
        tag_union[*ii]++;

        if (dm_recently_added.count(*ii) == 0)
          dm_recently_added.insert(*ii);
      }//for
    }//if


  //for (union_type::iterator ii=tag_union.begin(); ii!=tag_union.end(); ++ii)
    //qDebug() << ii->first << ii->second;

  // remove all the checkboxes that arent in this list (will this every do anything though?)

  // add all the new checkboxes we need
  for (union_type::iterator ii=tag_union.begin(); ii!=tag_union.end(); ++ii)
    if (dm_boxes.count(ii->first) == 0) {
      QCheckBox *cb = new MyCheckBox(ii->first);

      cb->setTristate(true);
      connect(cb, SIGNAL(clicked(bool)), this, SLOT(onCheckBox(bool)));

      dm_boxes[ii->first] = cb;

      // insert it into the layout such that it is still sorted
      //dm_containerLayout->addWidget(cb);
      // find where this new checkbox belongs

      int index = 0;
      boxes_t::iterator bb;
      for (bb=dm_boxes.begin(), index = 0; bb != dm_boxes.end(); ++bb)
        if (bb->first == ii->first)
          break;  // found it
        else
          ++index;

      if (bb != dm_boxes.end())
        dm_containerLayout->insertWidget(index, cb);
      else
        dm_containerLayout->addWidget(cb);
    }

  // set the state of all the checkboxes
  for (boxes_t::iterator bb=dm_boxes.begin(); bb != dm_boxes.end(); ++bb) {
    if (tag_union.count(bb->first) == 0 || tag_union[bb->first] == 0)
      bb->second->setCheckState(Qt::Unchecked);
    else if (tag_union[bb->first] == totalSelectedFiles)
      bb->second->setCheckState(Qt::Checked);
    else
      bb->second->setCheckState(Qt::PartiallyChecked);
  }

//dm_containerLayout->activate();
  //dm_containerWidget->adjustSize();
//qDebug() << " L " << dm_containerLayout->geometry() << " bb=" << dm_containerLayout->boundingBox();
//qDebug() << " W " << dm_containerWidget->geometry();
  /*if (!dm_containerWidget->geometry().contains(dm_containerLayout->boundingBox())) {
    dm_containerWidget->resize(dm_containerLayout->boundingBox().size());
  }*/

  dm_containerLayout->update();
}

void TagEditorView::onAdd(void)
{
  QString newtag(dm_addedit->text());

  if (newtag.isEmpty())
    return;

  if (dm_recently_added.count(newtag) == 0) {
    dm_recently_added.insert(newtag);
  }

  setTag(newtag);
  //updateEditor();

  dm_addedit->clear();
}

void TagEditorView::onClear(void)
{
  QModelIndexList selected = fileList()->selectionModel()->selectedIndexes();
  QDateTime now(QDateTime::currentDateTime());

  for (QModelIndexList::const_iterator ii=selected.begin(); ii != selected.end(); ++ii) {
    FileEntry &entry(fileList()->fileAt(*ii));
    FileItemRecord &item(entry.recordItem());

    Engine::instance()->regetFileItem(item);

    bool imageChanged = false;
    if (item.tags.clearTags(&imageChanged)) {
      Engine::instance()->saveFileItem(item, now);
      if (imageChanged) {
        // the image changed (rotate operation or something)
        // invalidate it, causing all its files to be reloaded
        entry.reloadRotateCode();
        fileList()->emitImageChange(ii->row(), this);
      }
    }
  }

  updateEditor();

  fileList()->emitChangedTagsAll();
}

void TagEditorView::onCheckBox(bool state)
{
  QObject * src = sender();

  // find the checkbox that sent this
  for (boxes_t::const_iterator ii=dm_boxes.begin(); ii != dm_boxes.end(); ++ii)
    if (ii->second == src) {
      if (state)
        setTag(ii->first);
      else
        unsetTag(ii->first);
      // found it
      break;
    }

  //updateEditor();
}

void TagEditorView::onDataChanged(void)
{
  updateEditor();
}

void TagEditorView::initGui(void)
{
  Ui_TagEditorDialog2::setupUi(this);

  dm_containerWidget = new QWidget;
  dm_containerLayout = new FlowLayout(true);

  dm_containerWidget->setLayout(dm_containerLayout);

  dm_scrollarea->setWidget(dm_containerWidget);

  connect(fileList()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(updateEditor()));
  connect(dm_addbut, SIGNAL(clicked()), this, SLOT(onAdd()));
  connect(dm_clearbut, SIGNAL(clicked()), this, SLOT(onClear()));
  connect(fileList(), SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
      this, SLOT(onDataChanged()));
}


