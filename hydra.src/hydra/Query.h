
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_QUERY_H__
#define __INCLUDED_HYDRA_QUERY_H__

#include <set>
#include <hydra/TR1.h>

#include <QString>

namespace hydra
{
  typedef std::set<QString> tags_t;

  class Token;

  static const char SCORE_CHAR = '$';

  /**
   * Finds and returns the length of the score tag.
   * If none, 0 is returned.
   * 
   * @author Aleksander Demko
   */ 
  int scoreValue(const tags_t &tags);

  bool isQueryChar(const QChar &c); 

  /**
   * Parse a given string into a token tree.
   * Returns true on success, false on failure. tok is no modified on failures.
   * Empty strings return an always true query.
   *
   * @author Aleksander Demko
   */ 
  bool parseQueryTokens(const QString &s, std::shared_ptr<hydra::Token> &tok);
}

/**
 * Core token/query object for token trees (basically, this represents
 * a query expression
 * @author Aleksander Demko
 */ 
class hydra::Token
{
  public:
    virtual ~Token() { }

    virtual bool isMatch(const hydra::tags_t &t) = 0;
};

#endif

