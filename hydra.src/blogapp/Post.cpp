
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <blogapp/Post.h>

#include <QDebug>

#include <wexus/Application.h>
#include <wexus/Context.h>

blogapp::Post::Post(void)
  : ActiveFile(
      wexus::Context::application()->settings()["appdir"].toString(),
      QRegExp("*.txt", Qt::CaseInsensitive, QRegExp::Wildcard))
{
}

bool blogapp::Post::onLoad(const QString &fullfilename)
{
  if (id.isEmpty())
    return true;

  QRegExp r = QRegExp("^(\\d\\d\\d\\d)(\\d\\d)(\\d\\d)?_(.+)\\.txt$");

  if (!r.exactMatch(id)) {
    // see if its just a basic text file then
    QRegExp txt = QRegExp("^(.+)\\.txt$");
    if (txt.exactMatch(id)) {
      QFileInfo info(fullfilename);
      QDate mod(info.lastModified().date());
      year = mod.year();
      month = mod.month();
      day = mod.day();
      QStringList l = r.capturedTexts();
      title = l[1];
      return true;
    }
    return false;
  }

  QStringList l = r.capturedTexts();
  bool ok;

  assert(l.size() == 5);

  year = l[1].toInt(&ok);
  if (!ok)
    return false;
  month = l[2].toInt(&ok);
  if (!ok)
    return false;
  day = 1;
  if (!l[3].isEmpty()) {
    day = l[3].toInt(&ok);
    if (!ok)
      return false;
  }

  title = l[l.size()-1];
  
  return true;
}

