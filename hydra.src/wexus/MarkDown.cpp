
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MarkDown.h>

#include <QDebug>

#include <wexus/Widgets.h>

using namespace wexus;

// repsents a nesting of paragraph contexts
namespace { class ParaContext {
  public:
    /**
     * Given the paragraph line, parse out a line and return the paragraph stack representation
     *
     * @author Aleksander Demko
     */ 
    static ParaContext parseLineStart(const QByteArray &input, int &index, const ParaContext &previous_context);

    /**
     * Creates a special null ParaContext that could be used a paragraph
     * seperator. (it always unwinds)
     *
     * @author Aleksander Demko
     */ 
    void appendEndPara(void);

    static bool isBulletChar(char c) { return c == '*' || c == '+' || c== '-'; }

    bool operator ==(const ParaContext &rhs) const { return dm_contexts == rhs.dm_contexts; }
    bool operator !=(const ParaContext &rhs) const { return dm_contexts != rhs.dm_contexts; }

    ParaContext emitDifference(const ParaContext &next, QByteArray &output);

    const QByteArray & contexts(void) const { return dm_contexts; }

  protected:
    QByteArray dm_contexts;     // types are '>' (quote) ' ' (code) '*' (list) 'H' (horizrule) 'P' (endpara)
}; }

namespace { class markdown
{
  public:
    QByteArray process(const QByteArray &input, int flags);

  protected:
    bool styleJumped(int returnState);
    bool ampJumped(int returnState);
    bool linkJumped(int returnState);

    void encodedPushBack(void) { encodedPushBack(c); }
    void encodedPushBack(char ccc, QByteArray *out = 0);

    void flushPara(void);

    static char styleToHtml(char style_c);

  protected:
    const QByteArray *input;
    QByteArray ret;
    QByteArray para_buf;
    int index;
    int flags;
    enum {
      FindingStart,
      InPara,
      InPara_StartOfLine,
      InPara_OnNewLine,
      Style_Start,
      Style_Processing,
      Amp_Processing,
      Link_Start,
      Link_Processing,
      Link_Ending,
      Title_Hashing,
      Title_GettingName,
      Title_EndHashes,
      Underlining,
    };
    int state;
    char c;

    int newlines;

    char style_c;
    int style_returnstate;

    int amp_returnstate;
    int amp_index;

    int link_returnstate;
    int link_index;
    bool link_havetwoparen;

    int title_level;
    int title_index;

    ParaContext paracontext;

    int line_prefixcount;
}; }

//
//
// ParaContext
//
//

ParaContext ParaContext::parseLineStart(const QByteArray &input, int &index, const ParaContext &previous_context)
{
  ParaContext ret;
  enum {
    Start,
    Need4Space,
    Ready,
    GotSpace,
    GotDigit,
    GotIndent,
    GotEquals,
    EatRule,
    DoneState,
  };
  //int startindex = index;
  int lastokindex = index;
  int state=Start;
  char c;
  int spacecount;
  char lastBulletChar = ' ';
  int prevctx_index = 0;
  char prevctx = 0;

  while (state != DoneState) {
    if (index < input.size()) {
      c = input[index];
      ++index;
    } else
      state = DoneState;
    if (prevctx_index < previous_context.dm_contexts.size()) {
      prevctx = previous_context.dm_contexts[prevctx_index];
      if (prevctx == 'P')
        prevctx = 0;    // simplifies things
    } else
      prevctx = 0;
retry_c:

    switch (state) {
      case Start:
        {
          if (c == ' ') {
            state = Need4Space;
            spacecount = 1;
          } else {
            state = Ready;
            goto retry_c;
          }
          break;
        }
      case Need4Space:
        {
          if (c == ' ') {
            spacecount++;
            if (spacecount == 4) {
              state = GotIndent;
              goto retry_c;
            }
          } else
            state = DoneState;
          break;
        }
      case Ready:
        {
          if (c == ' ') {
            spacecount = 1;
            state = GotSpace;
          } else if (c == '=') {
            state = GotEquals;
          } else if (c == '\t') {
            state = GotIndent;
            goto retry_c;
          } else if (c == '>' && (prevctx == '>' || prevctx == 0)) { // can only start a new quote or continue an existing one
            ret.dm_contexts.push_back('>');
            prevctx_index++;
            lastokindex = index;
          } else if (isBulletChar(c) && c != lastBulletChar && (prevctx =='*' || prevctx == 0)) {  // cant have ** etc beside each other in a list
            lastBulletChar = c;
            ret.dm_contexts.push_back('*');
            prevctx_index++;
            lastokindex = index;
          } else if (isBulletChar(c) && c == lastBulletChar) {
            // what was a bullet is really a horiz rule
            ret.dm_contexts[ret.dm_contexts.size()-1] = 'H';
            lastokindex = index;
            state = EatRule;
          } else if (c == '\n') {
            // blank line
            ret.dm_contexts.push_back('P');
            // dont update lastokindex
            state = DoneState;
          } else
            state = DoneState;
          break;
        }
      case GotSpace:
        {
          if (c == ' ') {
            spacecount++;
            if (spacecount == 4) {
              state = GotIndent;
              goto retry_c;
            }
          } else {
            state = Ready;
            goto retry_c;
          }
          break;
        }
      case GotIndent:
        {
          if (prevctx == '*') {
            // currently in a list context, so this indent can count as a continuous
            ret.dm_contexts.push_back('*');
            prevctx_index++;
            state = Ready;
            lastokindex = index;
          } else if (prevctx == ' ' || prevctx == 0) {
            ret.dm_contexts.push_back(' ');
            state = DoneState;
            lastokindex = index;
          } else
            state = DoneState;
          break;
        }
      case GotEquals:
        {
          if (c == '=') {
            // got a rule, end
            ret.dm_contexts.push_back('H');
            lastokindex = index;
            state = EatRule;
          } else if (c == ' ') {
            // nothing
          } else
            state = DoneState;
          break;
        }
      case EatRule:
        {
          if (c == ' ' || isBulletChar(c) || c == '=') {
            lastokindex = index;
          } else
            state = DoneState;
          break;
        }
    }
  }//while

  index = lastokindex;

  /*if (ret.dm_contexts.size() >= 2 && isBulletChar(ret.dm_contexts[0])
      && ret.dm_contexts[0] == ret.dm_contexts[1]) {
    // two list items in a row, this is not allowed
    // and is probably a titleunderline or horizontal rule
    // lets quick abort now
    index = startindex;
    ret = ParaContext();
    return Result_Bar;
  }*/

  return ret;
}

void ParaContext::appendEndPara(void)
{
  dm_contexts.push_back('P');
}

ParaContext ParaContext::emitDifference(const ParaContext &next, QByteArray &output)
{
  int i;
  int canceled_i, canceled_next;

  bool alive = next.dm_contexts.isEmpty() || next.dm_contexts[0] != 'P';

qDebug() << __FUNCTION__ << dm_contexts << "vs." << next.dm_contexts;

  canceled_i = 0;
  canceled_next = 0;
  while (alive && canceled_i < dm_contexts.size()) {
    switch (dm_contexts[canceled_i]) {
      case ' ': {
                  // code prefix must have another code prefix
                  if (canceled_next < next.dm_contexts.size() &&
                      next.dm_contexts[canceled_next] == ' ')
                    canceled_next++;
                  else
                    alive = false;
                  break;
                }
      case '>': {
                  // quote prefix CAN match another code prefix
                  if (canceled_next < next.dm_contexts.size() &&
                      next.dm_contexts[canceled_next] == '>')
                    canceled_next++;
                  // no else
                  break;
                }
      case '*': {
                  // quote prefix CAN match another code prefix
                  if (canceled_next < next.dm_contexts.size() &&
                      (next.dm_contexts[canceled_next] == '*'
                       || next.dm_contexts[canceled_next] == ' '))
                    canceled_next++;
                  // no else
                  break;
                }

    }//switch
    if (alive)
      canceled_i++;
    if (canceled_next < next.dm_contexts.size() && next.dm_contexts[canceled_next] == 'P')
      alive = false;
  }//while

  // unwindow the current context
  for (i=dm_contexts.size()-1; i>=canceled_i; --i)
    switch (dm_contexts[i]) {
      case ' ': output += "</PRE>\n"; break;
      case '>': output += "</BLOCKQUOTE>\n"; break;
      case '*': output += "</LI></UL>\n"; break;
      case 'P': assert(false); break; // you can never unwindow from the null ParaContext!
    }

  // if the two were the same and we are in a list, it must mean we are
  // between list items
  if (dm_contexts == next.dm_contexts && dm_contexts.size() > 0 && dm_contexts[dm_contexts.size()-1] == '*')
    output += "</LI>\n<LI>";

  // window up to the next context

  for (i=canceled_next; i<next.dm_contexts.size(); ++i)
    switch (next.dm_contexts[i]) {
      case ' ': output += "<PRE>"; break;
      case '>': output += "<BLOCKQUOTE>"; break;
      case '*': output += "<UL><LI>"; break;
      case 'H': output += "<HR />\n"; break;
    }

  ParaContext ret;

  // the return context is everything that we didnt cancel
  // here vs everything that is new there
  for (i=0; i<canceled_i; ++i)
    ret.dm_contexts.push_back(dm_contexts[i]);
  for (i=canceled_next; i<next.dm_contexts.size() && next.dm_contexts[i] != 'P'; ++i)
    ret.dm_contexts.push_back(next.dm_contexts[i]);

qDebug() << __FUNCTION__ << "returning" << ret.dm_contexts;
  return ret;
}

//
//
// markdown
//
//

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

void markdown::encodedPushBack(char ccc, QByteArray *buf)
{
  if (!buf)
    buf = &para_buf;
  switch (ccc) {
    case '<': *buf += "&lt;"; break;
    case '>': *buf += "&gt;"; break;
    case '"': *buf += "&quot;"; break;
    case '&': *buf += "&amp;"; break;
    default: (*buf).push_back(ccc); break;
  }
}

void markdown::flushPara(void)
{
  ParaContext nextctx;

  nextctx.appendEndPara();

  paracontext.emitDifference(nextctx, para_buf);
  paracontext = ParaContext();

  ret += "<P>";
  ret += para_buf;
  ret += "</P>\n";

  para_buf.clear();
}

char markdown::styleToHtml(char style_c)
{
  return style_c == '*' ? 'B' : 'I';
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
                   if (c == '\n')
                     continue;
                   // para start char, go!
                   if (c == '#' && flags & MarkDown::Format_Titles) {
                     state = Title_Hashing;
                     title_level = 0;
                     goto retry_c;
                   }
                   para_buf.clear();
                   newlines = 0;

                   state = InPara_StartOfLine;
                   goto retry_c;
                   break;
                 }
      case InPara_StartOfLine: {
                                 index--; // backtrack
                                 ParaContext nextctx = ParaContext::parseLineStart(input, index, paracontext);
                                 nextctx = paracontext.emitDifference(nextctx, para_buf);
                                 paracontext = nextctx;

                                 state = InPara;

                                 break;
                               }
      case InPara: {
                     if (c == '\n') {
                       newlines++;
                       state = InPara_OnNewLine;
                     } else if (styleJumped(InPara)) {
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
                               } else if ((c == '=' || c == '-') && newlines == 1) {
                                 title_level = (c == '=') ? 1 : 2;
                                 state = Underlining;
                               } else {
                                 para_buf.push_back('\n');
                                 state = InPara_StartOfLine;
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
      case Title_Hashing: {
                            if (c == '#')
                              title_level++;
                            else {
                              title_index = index;
                              state = Title_GettingName;
                              goto retry_c;
                            }
                            break;
                          }
      case Title_GettingName: {
                                if (c == '#' || c == '\n') {
                                  // done the title
                                  QByteArray title=input.mid(title_index-1, index-title_index);
                                  ret += "<H" + QString::number(title_level) + ">";
                                  for (int i=0; i<title.size(); ++i)
                                    encodedPushBack(title[i], &ret);
                                  ret += "</H" + QString::number(title_level) + ">\n";
                                  if (c == '#')
                                    state = Title_EndHashes;
                                  else
                                    state = FindingStart;
                                }
                                break;
                              }
      case Title_EndHashes: {
                              if (c != '#') {
                                state = FindingStart;
                                goto retry_c;
                              }
                              break;
                            }
      case Underlining: {
                          if (c == '\n') {
                            ret += "<H" + QString::number(title_level) + ">";
                            for (int i=0; i<para_buf.size(); ++i)
                              encodedPushBack(para_buf[i], &ret);
                            ret += "</H" + QString::number(title_level) + ">\n";
                            state = FindingStart;
                          }
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

//
//
// MarkDown
//
//

QByteArray MarkDown::process(const QByteArray &input, int flags)
{
  markdown md;
  return md.process(input, flags);
}

