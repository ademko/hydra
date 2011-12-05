
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MarkDown.h>

#include <QDebug>

#include <wexus/Widgets.h>

using namespace wexus;

namespace { class MatchTerm {
  public:
    enum {
      Want_Title = 1,
      Want_FirstPara = 2,
    };
  public:
    const int wantType;
    HTMLString htmlString;
    bool foundIt;

  public:
    MatchTerm(int _wantType) : wantType(_wantType), foundIt(false) { }
}; }

namespace { class ParaBuffer : public QByteArray {
  public:
    ParaBuffer(QByteArray *output = 0, MatchTerm *mterm = 0);

    void resetOutput(QByteArray *output) { dm_output = output; }
    void resetPartialOutput(MatchTerm *mterm) { dm_matchterm = mterm; }
    QByteArray & sourceOutput(void) const { return *dm_output; }

    void setType(const QString &type) { dm_para_type = type; }
    void flushPara(void);

  private:
    QString dm_para_type;
    QByteArray *dm_output;
    MatchTerm *dm_matchterm;
}; }

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

    ParaContext emitDifference(const ParaContext &next, ParaBuffer &output);

    const QByteArray & contexts(void) const { return dm_contexts; }

    /**
     * returns true if the top normal state is c
     *
     * @author Aleksander Demko
     */ 
    bool isTop(char c) const;

  protected:
    QByteArray dm_contexts;     // types are '>' (quote) ' ' (code) '*' (list) 'H' 'h' (horizrule) 'X' (endpara)
    // internal codes 'c' list contination
}; }

namespace { class markdown
{
  public:
    QByteArray process(const QByteArray &input, int flags, MatchTerm *mterm = 0);

  protected:
    bool styleJumped(int returnState);
    bool ampJumped(int returnState);
    bool escapeJumped(int returnState);
    bool linkJumped(int returnState);

    void encodedPushBack(void) { encodedPushBack(c); }
    void encodedPushBack(char ccc, QByteArray *out = 0);

    static char styleToHtml(char style_c);

  protected:
    MatchTerm *matchterm;

    const QByteArray *input;
    QByteArray output; // this is the return array
    ParaBuffer para_buf;  // para_buf feeds into output
    int index;
    int flags;
    enum {
      FindingStart,
      InPara_Basic,
      InPara_WithStyleChecking,
      InPara_CheckForStyle,
      InPara_HashCheck,
      InPara_StartOfLine,
      Style_Start,
      Style_Processing,
      Style_GotEnd,
      Amp_Processing,
      Escape_Processing,
      Link_Start,
      Link_Processing,
      Link_Ending,
      Title_Hashing,
      Title_GettingName,
      Title_EndHashes,
    };
    int state;
    char c;

    char style_c;
    int style_returnstate;

    int amp_returnstate;
    int amp_index;

    int escape_returnstate;
    int escape_index;

    int link_returnstate;
    int link_index;
    bool link_havetwoparen;

    int title_level;

    ParaContext paracontext;
}; }

//
//
// ParaBuffer
//
//

ParaBuffer::ParaBuffer(QByteArray *output, MatchTerm *mterm)
  : dm_output(output), dm_matchterm(mterm)
{
  reserve(1024);
  dm_para_type.reserve(4);
}

void ParaBuffer::flushPara(void)
{
//qDebug() << "flushPara w/ size=" << size() << "type=" << dm_para_type << "data:" << *this;
  if (isEmpty()) {    // do nothing on empties
    dm_para_type.clear();   // still need to clear this, though
    return;
  }

  // TIE THIS IN
  // DO FULL ABORTING (exception based? or flags?)
  if (dm_matchterm) {
    if (dm_matchterm->wantType == MatchTerm::Want_Title && !dm_para_type.isEmpty() && dm_para_type[0] == 'H') {
      dm_matchterm->htmlString = *this;
      dm_matchterm->foundIt = true;
    }
    if (dm_matchterm->wantType == MatchTerm::Want_FirstPara && this->size() > 50 && dm_para_type == "P") {
      dm_matchterm->htmlString = *this;
      dm_matchterm->foundIt = true;
    }
  }

  if (!dm_para_type.isEmpty())
    *dm_output += "<" + dm_para_type + ">";
  *dm_output += *this;
  if (!dm_para_type.isEmpty())
    *dm_output += "</" + dm_para_type + ">\n";

  dm_para_type.clear();

  clear();
}

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
    GotEnd,
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
    } else {
      c = '\n';
      state = GotEnd;
    }
    if (prevctx_index < previous_context.dm_contexts.size()) {
      prevctx = previous_context.dm_contexts[prevctx_index];
      if (prevctx == 'X')
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
          } else {
            state = GotEnd;
            goto retry_c;
          }
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
            ret.dm_contexts[ret.dm_contexts.size()-1] = 'h';
            lastokindex = index;
            state = EatRule;
          } else {
            state = GotEnd;
            goto retry_c;
          }
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
            ret.dm_contexts.push_back('c');
            prevctx_index++;
            state = Ready;
            lastokindex = index;
          } else if (prevctx == ' ' || prevctx == 0) {
            // starting a code segment, exit after the next char too
            ret.dm_contexts.push_back(' ');
            lastokindex = index;
            state = GotEnd;
          } else
            state = GotEnd;
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
          } else {
            state = GotEnd;
            goto retry_c;
          }
          break;
        }
      case EatRule:
        {
          if (c == ' ' || isBulletChar(c) || c == '=') {
            lastokindex = index;
          } else {
            state = GotEnd;
            goto retry_c;
          }
          break;
        }
      case GotEnd:
        {
          if (c == '\n') { // blank line
            ret.dm_contexts.push_back('X');
          }
          state = DoneState;
          break;
        }
    }//switch
  }//while

  // special case
  // if we have list continous (that is, 4-spaces), just eliminate them all or covnert them to '*'
  {
    enum {
      FindingFirstOne,
      Replacing,
      Done
    };
    int state = FindingFirstOne;
    int j_of_c;
    int j=ret.dm_contexts.size()-1;
    bool found_cont = false;
    int nukefrom = ret.dm_contexts.size();
    while (j>=0 && state != Done) {
      j_of_c = j;
      char c = ret.dm_contexts[j_of_c];
      j--;
retry_j:
      switch (state) {
        case FindingFirstOne:
          state = Replacing;
          if (c == 'c') {
            found_cont = true;
            goto retry_j;
          }
          break;
        case Replacing:
          if (c == 'c') {
            if (found_cont)
              nukefrom = j;
            else
              ret.dm_contexts[j_of_c] = '*';  // switch it back to a continuation
          } else
            state = Done;
          break;
      }//switch
    }//while
    if (nukefrom < ret.dm_contexts.size())
      ret.dm_contexts.truncate(nukefrom);
  }

  index = lastokindex;

  return ret;
}

void ParaContext::appendEndPara(void)
{
  dm_contexts.push_back('X');
}

ParaContext ParaContext::emitDifference(const ParaContext &next, ParaBuffer &output)
{
  int i;
  int canceled_i, canceled_next;
  bool alive = next.dm_contexts.isEmpty() || next.dm_contexts[0] != 'X';
  bool seensecondarystar = false;

//qDebug() << __FUNCTION__ << dm_contexts << "vs." << next.dm_contexts;

  canceled_i = 0;
  canceled_next = 0;
  while (alive && canceled_i < dm_contexts.size()) {
    switch (dm_contexts[canceled_i]) {
      case ' ': {
                  // code prefix must have another code prefix
                  if (canceled_next < next.dm_contexts.size())
                      // &&next.dm_contexts[canceled_next] == ' ')
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
                  // list prefix must match another list prefix
                  if (canceled_next < next.dm_contexts.size() &&
                      (next.dm_contexts[canceled_next] == '*')) {
                    canceled_next++;
                    seensecondarystar = true;
                  } else if (seensecondarystar)   // this seensecondarystar stuff we need to properly match continuations
                    alive = false;
                  break;
                }

    }//switch
    if (canceled_next < next.dm_contexts.size() && next.dm_contexts[canceled_next] == 'X')
      alive = false;
    if (alive)
      canceled_i++;
  }//while

  // we need to precount our operations
  int downshifts = 0;
  int upshifts = 0;

  for (i=dm_contexts.size()-1; i>=canceled_i; --i)
    if (dm_contexts[i] == ' ' || dm_contexts[i] == '>' || dm_contexts[i] == '*')
      downshifts++;
  for (i=canceled_next; i<next.dm_contexts.size(); ++i)
    if (next.dm_contexts[i] == ' ' || next.dm_contexts[i] == '>' || next.dm_contexts[i] == '*')
      upshifts++;

//qDebug() << "STATUS" << "downshifts" << downshifts << "upshifts" << upshifts << "canceled_i" << canceled_i << "canceled_next" << canceled_next;

  bool nowatcode = next.isTop(' ');
  // if the two were the same and we are in a list, it must mean we are
  // between list items
  bool listitemchange = false;
  
  {
    bool left = isTop('*');
    bool right = next.isTop('*');

    if (left && right && downshifts == 0 && upshifts == 0)
      listitemchange = true;
    if (left && right && downshifts > 0 && upshifts == 0)
      listitemchange = true;
    // can easily merge those two cases
  }

  if (downshifts>0 || upshifts>0) {
    if (downshifts == 0)
      output.setType("P");    // upshifts must be >0
    output.flushPara();
  } else if (listitemchange)
    output.flushPara();

  // unwindow the current context
  for (i=dm_contexts.size()-1; i>=canceled_i; --i)
    switch (dm_contexts[i]) {
      case ' ': output.sourceOutput() += "</CODE></PRE>\n"; break;
      case '>': output.sourceOutput() += "</BLOCKQUOTE>\n"; break;
      case '*': output.sourceOutput() += "</LI></UL>\n"; break;
      case 'X': assert(false); break; // you can never unwindow from the null ParaContext!
    }

  if (listitemchange)
    output.sourceOutput() += "</LI>\n<LI>";

  bool endedpara = false;
  int havearule = 0;  // 1 or 2 is the rule leve

  // window up to the next context
  for (i=canceled_next; i<next.dm_contexts.size(); ++i)
    switch (next.dm_contexts[i]) {
      case ' ': output.sourceOutput() += "<PRE><CODE>"; break;
      case '>': output.sourceOutput() += "<BLOCKQUOTE>"; break;
      case '*': output.sourceOutput() += "<UL><LI>"; break;
      case 'H': havearule = 1; break;
      case 'h': havearule = 2; break;
      //case 'H': output.sourceOutput() += "<HR />\n"; break;
      case 'X': endedpara = true ; break;
    }

  if (havearule > 0) {
//qDebug() << "HAVEARULE";
    if (output.isEmpty())
      output.sourceOutput() += "<HR />\n";
    else {
      output.setType("H" + QString::number(havearule));
      output.flushPara();
    }
    endedpara = true;
  } 
  
  if (endedpara) {
    // see if we just need a para change without any level change
//qDebug() << "ENDEDPARA?" << downshifts << "upshifts" << upshifts << "nowatcode" << nowatcode;
    if (downshifts == 0 && upshifts == 0 && !nowatcode)
      output.setType("P");
    output.flushPara();
    if (downshifts == 0 && upshifts == 0 && !nowatcode)
      output.setType("P");
  }

  if (downshifts>0 && upshifts == 0 && !nowatcode) {
    // see if we shifted down into a previous level
//qDebug() << "SHIFT_DOWN_TO_PREV" << downshifts << "upshifts" << upshifts;
    output.setType("P");
  }

  if (upshifts>0) {
    // see if we shifted into a new level
    output.setType("");
  }

  ParaContext ret;

  // the return context is everything that we didnt cancel
  // here vs everything that is new there
  for (i=0; i<canceled_i; ++i)
    ret.dm_contexts.push_back(dm_contexts[i]);
  for (i=canceled_next; i<next.dm_contexts.size(); ++i)
    if (next.dm_contexts[i] != 'X' && next.dm_contexts[i] != 'H')
      ret.dm_contexts.push_back(next.dm_contexts[i]);

//qDebug() << __FUNCTION__ << "returning" << ret.dm_contexts;
  return ret;
}

bool ParaContext::isTop(char c) const
{
  for (int i=dm_contexts.size()-1; i>=0; --i) {
    int cur = dm_contexts[i];
    if (cur == c)
      return true;
    if (cur == ' ' || cur == '>' || cur == '*')
      return false;
    // must be a X or H, which we can skip down
  }
  return false;
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

bool markdown::escapeJumped(int returnState)
{
  if (c == '\\') {
    escape_returnstate = returnState;
    escape_index = index;
    state = Escape_Processing;
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

char markdown::styleToHtml(char style_c)
{
  return style_c == '*' ? 'B' : 'I';
}

QByteArray markdown::process(const QByteArray &input, int flags, MatchTerm *mterm)
{
  this->input = &input;
  this->flags = flags;
  index = 0;
  state = FindingStart;

  matchterm = mterm;

  output.reserve(input.size()*2);
  para_buf.resetOutput(&output);
  para_buf.resetPartialOutput(mterm);

  while (index <= input.size()) {
    if (index < input.size())
      c = input[index];
    else
      c = '\n'; // fake eof
    index++;
retry_c:
    if (mterm && mterm->foundIt)
      return output;    // abort now, we found our target string
    if (c == '\r')    // we completely filter these out
      continue;
    switch (state) {
      case FindingStart: {
                   if (c == '\n')
                     continue;

                   state = InPara_StartOfLine;
                   goto retry_c;
                   break;
                 }
      case InPara_StartOfLine: {
                                 index--; // backtrack
                                 ParaContext nextctx = ParaContext::parseLineStart(input, index, paracontext);
                                 nextctx = paracontext.emitDifference(nextctx, para_buf);
                                 paracontext = nextctx;

                                 state = InPara_HashCheck;

                                 break;
                               }
      case InPara_HashCheck: {
                               if (c == '#') {
                                 para_buf.setType("P");
                                 para_buf.flushPara();
                                 title_level = 0;
                                 state = Title_Hashing;
                               } else {
                                 if (paracontext.isTop(' '))
                                   state = InPara_Basic;
                                 else {
                                   para_buf.push_back(' ');
                                   state = InPara_CheckForStyle;// we actually want style and link checking at the start of the line too
                                 }
                               }
                               goto retry_c;
                               break;
                             }
      case InPara_WithStyleChecking: {
                                       bool skipbreak = false;
                                       if (c == ' ' || c == '(') {  // more special cases or broaden this list?
                                         para_buf.push_back(c);
                                         state = InPara_CheckForStyle;
                                       } else if (escapeJumped(InPara_WithStyleChecking)) {
                                         // nothing needed
                                       } else if (ampJumped(InPara_WithStyleChecking)) {
                                         // nothing needed
                                       } else 
                                         skipbreak = true;

                                       if (!skipbreak)
                                         break;
                                     }
      case InPara_Basic: {
                     if (c == '\n') {
                       if (paracontext.isTop(' ') ||
                           (!para_buf.isEmpty() && para_buf[para_buf.size()-1] != '\n'))
                         para_buf.push_back('\n');
                       state = InPara_StartOfLine;
                     } else
                       encodedPushBack();
                     break;
                   }
      case InPara_CheckForStyle: {
                               if (styleJumped(InPara_WithStyleChecking)) {
                                 // nothing needed
                               } else if (linkJumped(InPara_WithStyleChecking)) {
                                 // nothing needed
                               } else {
                                 state = InPara_WithStyleChecking;
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
                                 state = Style_GotEnd;
                               } else if (ampJumped(Style_Processing)) {
                                 // do nothing
                               } else
                                 encodedPushBack();
                               break;
                             }
      case Style_GotEnd: {
                           if (!QChar(c).isLetterOrNumber()) {
                             // done styling
                             para_buf += QString("</") + styleToHtml(style_c) + ">";
                             state = style_returnstate;
                             goto retry_c;
                           } else {
                             // not a space char, flush the style char and continue normal style processing
                             para_buf.push_back(style_c);
                             state = Style_Processing;
                             goto retry_c;
                           }
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
      case Escape_Processing: {
                                state = escape_returnstate;
                                switch (c) {
                                  case '\\':
                                  case '`':
                                  case '*':
                                  case '_':
                                  case '{':
                                  case '}':
                                  case '[':
                                  case ']':
                                  case '(':
                                  case ')':
                                  case '#':
                                  case '+':
                                  case '-':
                                  case '.':
                                  case '!':
                                    para_buf += c;
                                    break;
                                  default:
                                    para_buf += '/';
                                    // no idea what this was, reprocess it
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
                              para_buf.setType("H" + QString::number(title_level));
                              state = Title_GettingName;
                              goto retry_c;
                            }
                            break;
                          }
      case Title_GettingName: {
                                if (c == '#' || c == '\n') {
                                  para_buf.flushPara();
                                  if (c == '#')
                                    state = Title_EndHashes;
                                  else
                                    state = FindingStart;
                                } else
                                  encodedPushBack();
                                break;
                              }
      case Title_EndHashes: {
                              if (c != '#') {
                                state = FindingStart;
                                goto retry_c;
                              }
                              break;
                            }
    }
  }

  if (!para_buf.isEmpty()) {
    // finish off the last para
    para_buf.flushPara();
  }

  return output;
}

//
//
// MarkDown
//
//

QByteArray MarkDown::process(const QByteArray &markdown, int flags)
{
  ::markdown md;

  return md.process(markdown, flags);
}

HTMLString MarkDown::title(const QByteArray &markdown)
{
  MatchTerm mterm(MatchTerm::Want_Title);

  ::markdown md;

  // throw away the rendered result
  md.process(markdown, Format_Titles, &mterm);

  return mterm.htmlString;   // even if we didnt find it, return the default emptry string
}

HTMLString MarkDown::firstPara(const QByteArray &markdown)
{
  MatchTerm mterm(MatchTerm::Want_FirstPara);

  ::markdown md;

  // throw away the rendered result
  md.process(markdown, Format_Titles, &mterm);

  return mterm.htmlString;   // even if we didnt find it, return the default emptry string
}

