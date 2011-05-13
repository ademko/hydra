
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/TemplateTokenList.h>

void wexus::compressList(TemplateTokenList &list)
{
  if (list.empty())
    return;

  TemplateTokenList::iterator ii, nn;

  ii = list.begin();

  for (ii=list.begin(); ii != list.end(); ++ii) {
    if ((*ii)->type() != 'L')
      continue;

    do {
      nn = ii;
      nn++;

      if (nn == list.end())
        return;
      if ((*nn)->type() != 'L') 
        break;
      // do a merge
      (*ii)->data() += (*nn)->data();
      list.erase(nn);
    } while (true);
  }
}

