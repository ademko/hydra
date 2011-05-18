
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTMLString.h>

using namespace wexus;

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

  bool lastblank = false;

  for (QString::const_iterator ii=s.begin(); ii != s.end(); ++ii) {
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
        int unicode = ii->unicode();
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

  return ret;
}

