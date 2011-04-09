
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/InfoView.h>

#include <QHBoxLayout>
#include <QTextStream>

#include <hydra/FileIterator.h>

using namespace hydra;
using namespace desktop;

InfoView::InfoView(FileList *_filelist, ImageCache *_loader)
  : View(_filelist), dm_loader(_loader)
{
  dm_label = new QLabel;

  QHBoxLayout *lay = new QHBoxLayout;

  lay->addWidget(dm_label);

  setLayout(lay);

  connect(_filelist->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
      this, SLOT(onCurrentChanged()));
  connect(_filelist, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
      this, SLOT(onCurrentChanged()));

  onCurrentChanged();
}

void InfoView::onCurrentChanged(void)
{
  //qDebug() << __FUNCTION__ << fileList()->isValid();

  if (!fileList()->isValid())
    return;

  QString s;
  QTextStream out(&s);

  desktop::cache_ptr<QImage> img = dm_loader->getImage(fileList()->selectedFile().fullfilename());

  out << '#' << (fileList()->selectedFileIndex()+1) << "/" << fileList()->numFiles()
    << "  "
    << hydra::justName(fileList()->selectedFile().fullfilename());

  if (img.get()) {
    // insert some image dimension info
    out.setRealNumberPrecision(2);
    out
      << ' ' << img->width() << 'x' << img->height()
      << " (" << (img->width()*img->height()/1000000.0) << "MP)";
  }
  out << " {";

  FileItemRecord &item = fileList()->selectedFile().recordItem();

  for (FileItemRecord::tags_t::const_iterator ii=item.tags.begin();
      ii != item.tags.end(); ++ii)
    out << ' ' << *ii;

  out << " }";

  dm_label->setText(s);
}

