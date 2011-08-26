
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MarkDown.h>

#include <QDebug>

#include <wexus/Widgets.h>

using namespace wexus;

namespace { class markdown
{
  public:
    QByteArray process(const QByteArray &input, int flags);

  protected:
    bool styleJumped(int returnState);
    bool ampJumped(int returnState);
    bool linkJumped(int returnState);

    void encodedPushBack(void) { encodedPushBack(c); }
    void encodedPushBack(char ccc);

    void flushPara(void);

  protected:
    const QByteArray *input;
    QByteArray ret;
    QByteArray para_buf;
    int index;
    int flags;
    enum {
      FindingStart,
      InPara,
      InPara_OnNewLine,
      Style_Start,
      Style_Processing,
      Amp_Processing,
      Link_Start,
      Link_Processing,
      Link_Ending,
    };
    int state;
    char c;

    char style_c;
    int style_returnstate;

    int amp_returnstate;
    int amp_index;

    int link_returnstate;
    int link_index;
    bool link_havetwoparen;
}; }

static char styleToHtml(char style_c)
{
  return style_c == '*' ? 'B' : 'I';
}

bool markdown::styleJumped(int returnState)
{
  if ((c == '*' || c == '_') && (index == 0 || (*input)[index-1] != ' ')) {
    style_c = c;
    style_returnstate = returnState;
    state = Style_Start;
    return true;
  } else
    return false;
}

bool markdown::ampJumped(int returnState)
{
  if (c == '&') {
    amp_returnstate = returnState;
    amp_index = index;
    state = Amp_Processing;
    return true;
  } else
    return false;
}

bool markdown::linkJumped(int returnState)
{
  if (c == '[' && flags & MarkDown::Format_Links) {
    link_returnstate = returnState;
    link_index = index;
    state = Link_Start;
    return true;
  } else
    return false;
}

void markdown::encodedPushBack(char ccc)
{
  switch (ccc) {
    case '<': para_buf += "&lt;"; break;
    case '>': para_buf += "&gt;"; break;
    case '"': para_buf += "&quot;"; break;
    default: para_buf.push_back(ccc); break;
  }
}

void markdown::flushPara(void)
{
  ret += "<P>";
  ret += para_buf;
  ret += "</P>\n";
}

QByteArray markdown::process(const QByteArray &input, int flags)
{
  this->input = &input;
  this->flags = flags;
  index = 0;
  state = FindingStart;

  ret.reserve(input.size()*2);
  para_buf.reserve(1024);

  while (index < input.size()) {
    c = input[index];
    index++;
retry_c:
    if (c == '\r')    // we completely filter these out
      continue;
    switch (state) {
      case FindingStart: {
                   if (QChar(c).isSpace())
                     continue;
                   // para start char, go!
                   para_buf.clear();
                   state = InPara;
                   goto retry_c;
                   break;
                 }
      case InPara: {
                     if (c == '\n')
                       state = InPara_OnNewLine;
                     else if (styleJumped(InPara)) {
                       // nothing needed
                     } else if (ampJumped(InPara)) {
                       // nothing needed
                     } else if (linkJumped(InPara)) {
                       // nothing needed
                     } else
                       encodedPushBack();
                     break;
                   }
      case InPara_OnNewLine: {
                               if (c == '\n') {
                                 // done this paragraph
                                 flushPara();
                                 state = FindingStart;
                               } else {
                                 state = InPara;
                                 goto retry_c;
                               }
                               break;
                             }
      case Style_Start: {
                          if (QChar(c).isSpace() || c == style_c) {
                            // abort the styling, as there is a space after the style char
                            para_buf.push_back(style_c);
                            state = style_returnstate;
                            goto retry_c;
                          } else {
                            para_buf += QString("<") + styleToHtml(style_c) + ">";
                            if (ampJumped(Style_Processing))
                              ;// do nothing
                            else {
                              encodedPushBack();
                              state = Style_Processing;
                            }
                          }
                          break;
                        }
      case Style_Processing: {
                               if (c == style_c) {
                                 // done styling
                                 para_buf += QString("</") + styleToHtml(style_c) + ">";
                                 state = style_returnstate;
                               } else if (ampJumped(Style_Processing)) {
                                 // do nothing
                               } else
                                 encodedPushBack();
                               break;
                             }
      case Amp_Processing: {
                        if (c >= 'a' && c <= 'z' && (index - amp_index) < 5) {
                          // do nothing
                        } else {
                          // exiting
                          // emit this code verbatim
                          if (c == ';')
                            para_buf += '&';  // &code; thing TODO future, precheck this so that only accepted codes are ok?
                          else
                            para_buf += "&amp;";
                          c = input[amp_index];
                          index = amp_index+1;
                          state = amp_returnstate;
                          goto retry_c;
                        }
                        break;
                      }
      case Link_Start: {
                         link_havetwoparen = c == '[';
                         state = Link_Processing;
                         if (link_havetwoparen)
                           link_index = index;
                         else
                           goto retry_c; // reprocess this char NOW
                         break;
                       }
      case Link_Processing: {
                              if (c == ']') {
                                // ending
                                QByteArray linkaddr=input.mid(link_index, index-link_index-1);
                                para_buf += "<A HREF=\"" + linkaddr + "\">" + linkaddr + "</A>";

                                if (link_havetwoparen)
                                  state = Link_Ending;
                                else
                                  state = link_returnstate;
                              }//else, stay in this state
                              break;
                            }
      case Link_Ending: {
                          state = link_returnstate;
                          if (c != ']')
                            goto retry_c;
                          break;
                        }
    }
  }

  if (!para_buf.isEmpty()) {
    // finish off the last para
    flushPara();
  }

  return ret;
}

QByteArray MarkDown::process(const QByteArray &input, int flags)
{
  markdown md;
  return md.process(input, flags);
}

