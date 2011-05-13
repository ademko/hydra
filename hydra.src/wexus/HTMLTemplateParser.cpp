
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/HTMLTemplateParser.h>

using namespace wexus;

/*
 Going to try to support all the same ERB like tags:
 http://www.ruby-doc.org/stdlib/libdoc/erb/rdoc/

  <% Ruby code -- inline with output %>
  <%= Ruby expression -- replace with result %>
  <%# comment -- ignored -- useful in testing %>
  <%% or %%> -- replace with <% or %> respectively

  maybe in the future:

  % a line of Ruby code -- treated as <% line %> (optional -- see ERB.new)
  %% replaced with % if first thing on a line and % processing is used
 */

void HTMLTemplateParser::parse(QIODevice &input, TemplateTokenList &outlist)
{
  enum state_t {
    IN_HTML,
    GOT_OPEN,
    GOT_OPEN_PER,
    GOT_CLOSE_PER,
    IN_BLOCK,
  };

  QByteArray buf;
  state_t state = IN_HTML;
  char c;
  char block_type = ' ';  // one of ' ' '=' '#' or '%'
  int lineno = 1;

  buf.reserve(8*1024);

  while (input.getChar(&c)) {
    if (c == '\n')
      lineno++;
    switch (state) {
      case IN_HTML:
        if (c == '<')
          state = GOT_OPEN;
        else
          buf.push_back(c);
        break;
      case GOT_OPEN:
        if (c == '<') // <<
          buf.push_back('<');
        else if (c == '%')
          state = GOT_OPEN_PER;
        else {
          buf.push_back('<');
          buf.push_back(c);
          state = IN_HTML;
        }
        break;
      case GOT_OPEN_PER:
        // flush the current IN_HTML
        outlist.push_back(std::shared_ptr<TemplateToken>(new TemplateToken(lineno, 'L', buf)));
        buf.clear();
        // we'return in a code block, determine its type
        switch (c) {
          case '#':
          case '=':
          case '%':
            block_type = c;
            break;
          default:
            block_type = ' ';
            buf.push_back(c);
            break;
        }
        state = IN_BLOCK;
        break;
      case IN_BLOCK:
        if (c == '%')
          state = GOT_CLOSE_PER;
        else
          buf.push_back(c);
        break;
      case GOT_CLOSE_PER:
        if (c == '%')
          buf.push_back('%'); // %% sequence
        else if (c == '>') {
          // done IN_BLOCK
          if (block_type == '#')
            ; // do nothing with comment blocks, just eat them
          else if (block_type == '%')
            // convert % to literal blocks
            outlist.push_back(std::shared_ptr<TemplateToken>(new TemplateToken(lineno, 'L', "<%" + buf + ">")));
          else
            outlist.push_back(std::shared_ptr<TemplateToken>(new TemplateToken(lineno, block_type, buf)));
          buf.clear();
          state = IN_HTML;
        } else {
          // false %
          buf.push_back('%');
          buf.push_back(c);
          state = IN_BLOCK;
        }
    }//switch(state)
  }//while

  // done, flush what is remaining, if any
  if (!buf.isEmpty())
    outlist.push_back(std::shared_ptr<TemplateToken>(new TemplateToken(lineno, 'L', buf)));
}

