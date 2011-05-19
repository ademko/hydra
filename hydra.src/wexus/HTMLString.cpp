
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTMLString.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

template <class C> inline int getUnicode(C c) { return c; }
template <> inline int getUnicode<QChar>(QChar c) { return c.unicode(); }

template <class ITER, class OUT>
  static void htmlEncode(ITER ii, ITER endii, OUT &ret)
{
  bool lastblank = false;

  for (; ii != endii; ++ii) {
    if (*ii == ' ') {
      // blank gets extra work,
      // this solves the problem you get if you replace all
      // blanks with &nbsp;, if you do that you loss 
      // word breaking
      if (lastblank) {
        lastblank = false;
        ret.append("&nbsp;");
      } else {
        lastblank = true;
        ret.append(' ');
      }
    } else {
      lastblank = false;

      // HTML Special Chars
      if (*ii == '"')
        ret.append("&quot;");
      else if (*ii == '&')
        ret.append("&amp;");
      else if (*ii == '<')
        ret.append("&lt;");
      else if (*ii == '>')
        ret.append("&gt;");
      else if (*ii == '\n')
        // Handle Newline
        ret.append("&lt;br/&gt;");
      else {
        int unicode = getUnicode(*ii);
        if (unicode < 160 )
          // nothing special only 7 Bit
          ret.append(*ii);
        else {
          // Not 7 Bit use the unicode system
          ret.append("&#");
          ret.append(QString::number(unicode));
          ret.append(';');
        }
      }
    }//else
  }//for
}

//
//
// HTMLString
//
//

HTMLString::HTMLString(void)
{
}

HTMLString::HTMLString(const QString &s)
  : QString(s)
{
}

HTMLString HTMLString::encode(const QString &s)
{
  HTMLString ret;

  ret.reserve(s.size() + 10);

  htmlEncode(s.begin(), s.end(), ret);

  return ret;
}

//
//
// HTMLEncoderDevice
//
//

HTMLEncoderDevice::HTMLEncoderDevice(QIODevice *outputdev)
  : dm_dev(outputdev)
{
  dm_enabled = true;
  assert(dm_dev);

  dm_buf.reserve(1024*4);

  // this is required when subclassing QIODevice
  // (in addition to implementing readData and writeData
  setOpenMode(QIODevice::WriteOnly);
}

qint64 HTMLEncoderDevice::readData(char * data, qint64 maxSize)
{
  return -1;
}

qint64 HTMLEncoderDevice::writeData(const char * data, qint64 maxSize)
{
  if (!dm_enabled)
    return dm_dev->write(data, maxSize);

  dm_buf.clear();

  htmlEncode(data, data+maxSize, dm_buf);

  if (!dm_buf.isEmpty()) {
    qint64 did = dm_dev->write(dm_buf.constData(), dm_buf.size());
    if (did > maxSize)
      return maxSize;   // never return more than maxSize... this might confuse the caller
    else
      return did;
  }
  
  return 0;
}

QTextStream & operator << (QTextStream &o, const wexus::HTMLString &htmlString)
{
  // see if the QTextStream has a HTMLEncoderDevice device in it
  HTMLEncoderDevice *htmldev = dynamic_cast<HTMLEncoderDevice*>(o.device());

  if (htmldev) {
    // flush an encoded stuff still in the QTextStream
    o.flush();
    htmldev->setEnabled(false); // temporarily turn off the encoder
  }

  QTextStream &ret = o << static_cast<const QString &>(htmlString);

  if (htmldev) {
    // flush this non-encoded write<< I just did
    o.flush();
    // renenable encoding
    htmldev->setEnabled(true);
  }

  return ret;
}

