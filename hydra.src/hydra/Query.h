
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
   * Parse a given search/query string into a token tree.
   * Returns true on success, false on failure. tok is no modified on failures.
   * Empty strings return an always true query that matches everything.
   * The resulting hydra::Token can they be used to test
   * against hydra::Tags
   *
   * @author Aleksander Demko
   */ 
  bool parseQueryTokens(const QString &s, std::shared_ptr<hydra::Token> &tok);
}

/**
 * Core token/query object for token trees. This is an object
 * reprentation of a query that can then be used to
 * test against tag sets for matches.
 *
 * @author Aleksander Demko
 */ 
class hydra::Token
{
  public:
    virtual ~Token() { }

    /**
     * Does this token tree match against the
     * given tags?
     *
     * @author Aleksander Demko
     */ 
    virtual bool isMatch(const hydra::tags_t &t) = 0;
};

#endif

