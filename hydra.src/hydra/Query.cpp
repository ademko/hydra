
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Query.h>

#include <assert.h>

#include <vector>

using namespace hydra;

namespace hydra {
class WildcardString;
class TrueToken;
class EmptyToken;
class TagToken;
class NotToken;
class AndToken;
class OrToken;
class XOrToken;
} // namespace hydra

using namespace std;

class hydra::WildcardString : public QString {
  public:
    /**
     * constructor. This class only holds a pointer to the given
     * string, so do not destroy it!
     *
     * @author Aleksander Demko
     */
    WildcardString(const QString &s);

    /// has any wild cards?
    bool HasWildcards(void) const { return dm_hasstar; }

    /// does this string match against this one?
    bool isMatch(const QString &test) const;

  private:
    bool dm_hasstar;
    QString dm_sub_string;
};

hydra::WildcardString::WildcardString(const QString &s) : QString(s) {
    dm_hasstar = !s.isEmpty() && s[s.size() - 1] == '*';
    if (dm_hasstar)
        dm_sub_string = s.left(s.size() - 1);
}

bool hydra::WildcardString::isMatch(const QString &test) const {
    if (dm_hasstar)
        return test.startsWith(dm_sub_string);
    else
        return *this == test;
}

//
// TrueToken
//

class hydra::TrueToken : public hydra::Token {
  public:
    virtual bool isMatch(const hydra::tags_t &) { return true; }
};

//
// EmptyToken
//

class hydra::EmptyToken : public hydra::Token {
  public:
    virtual bool isMatch(const hydra::tags_t &t) { return t.empty(); }
};

//
// TagToken
//

class hydra::TagToken : public hydra::Token {
  public:
    TagToken(const QString &tag);
    virtual bool isMatch(const hydra::tags_t &t);

  private:
    WildcardString dm_tag;
};

TagToken::TagToken(const QString &tag) : dm_tag(tag) {}

bool TagToken::isMatch(const hydra::tags_t &t) {
    if (dm_tag.HasWildcards()) {
        for (tags_t::const_iterator ii = t.begin(); ii != t.end(); ++ii)
            if (dm_tag.isMatch(*ii))
                return true;
        return false; // not found
    } else
        return t.find(dm_tag) != t.end();
}

//
// NotToken
//

class hydra::NotToken : public hydra::Token {
  public:
    NotToken(shared_ptr<Token> t);
    virtual bool isMatch(const hydra::tags_t &t);

  private:
    shared_ptr<Token> dm_tok;
};

hydra::NotToken::NotToken(shared_ptr<Token> t) : dm_tok(t) {}

bool hydra::NotToken::isMatch(const hydra::tags_t &t) {
    return !dm_tok->isMatch(t);
}

//
// AndToken
//

class hydra::AndToken : public hydra::Token {
  public:
    AndToken(shared_ptr<Token> left, shared_ptr<Token> right);
    virtual bool isMatch(const hydra::tags_t &t);

  private:
    shared_ptr<Token> dm_left, dm_right;
};

hydra::AndToken::AndToken(shared_ptr<Token> left, shared_ptr<Token> right)
    : dm_left(left), dm_right(right) {}

bool hydra::AndToken::isMatch(const hydra::tags_t &t) {
    return dm_left->isMatch(t) && dm_right->isMatch(t);
}

//
// OrToken
//

class hydra::OrToken : public hydra::Token {
  public:
    OrToken(shared_ptr<Token> left, shared_ptr<Token> right);
    virtual bool isMatch(const hydra::tags_t &t);

  private:
    shared_ptr<Token> dm_left, dm_right;
};

hydra::OrToken::OrToken(shared_ptr<Token> left, shared_ptr<Token> right)
    : dm_left(left), dm_right(right) {}

bool hydra::OrToken::isMatch(const hydra::tags_t &t) {
    return dm_left->isMatch(t) || dm_right->isMatch(t);
}

//
// XOrToken
//

class hydra::XOrToken : public hydra::Token {
  public:
    XOrToken(shared_ptr<Token> left, shared_ptr<Token> right);
    virtual bool isMatch(const hydra::tags_t &t);

  private:
    shared_ptr<Token> dm_left, dm_right;
};

hydra::XOrToken::XOrToken(shared_ptr<Token> left, shared_ptr<Token> right)
    : dm_left(left), dm_right(right) {}

bool hydra::XOrToken::isMatch(const hydra::tags_t &t) {
    return (dm_left->isMatch(t) ^ dm_right->isMatch(t)) != 0;
}

int hydra::scoreValue(const tags_t &tags) {
    for (tags_t::const_iterator ii = tags.begin(); ii != tags.end(); ++ii) {
        assert(!ii->isEmpty());
        if ((*ii)[0] == SCORE_CHAR)
            return ii->size();
    }

    return 0;
}

bool hydra::isQueryChar(const QChar &c) {
    return c.isLetterOrNumber() || c == '_' || c == '-' || c == '*';
}

bool hydra::parseQueryTokens(const QString &s,
                             std::shared_ptr<hydra::Token> &tok) {
    typedef std::vector<shared_ptr<Token>> tokstack;
    tokstack st;
    QString curtok;
    int i;

    st.reserve(4);
    curtok.reserve(28);

    for (i = 0; i <= s.size(); ++i) // note the <=!
        if (i == s.size() || s[i] == ' ') {
            // end of token
            if (!curtok.isEmpty()) {
                // process the current token
                if (curtok == "all") {
                    st.push_back(shared_ptr<Token>(new TrueToken()));
                } else if (curtok == "empty") {
                    st.push_back(shared_ptr<Token>(new EmptyToken()));
                } else if (curtok == "not") {
                    if (st.size() < 1)
                        return false; // parse error
                    shared_ptr<Token> a(st.back());
                    st.pop_back();
                    st.push_back(shared_ptr<Token>(new NotToken(a)));
                } else if (curtok == "and") {
                    if (st.size() < 2)
                        return false; // parse error
                    shared_ptr<Token> b(st.back());
                    st.pop_back();
                    shared_ptr<Token> a(st.back());
                    st.pop_back();
                    st.push_back(shared_ptr<Token>(new AndToken(a, b)));
                } else if (curtok == "or") {
                    if (st.size() < 2)
                        return false; // parse error
                    shared_ptr<Token> b(st.back());
                    st.pop_back();
                    shared_ptr<Token> a(st.back());
                    st.pop_back();
                    st.push_back(shared_ptr<Token>(new OrToken(a, b)));
                } else if (curtok == "xor") {
                    if (st.size() < 2)
                        return false; // parse error
                    shared_ptr<Token> b(st.back());
                    st.pop_back();
                    shared_ptr<Token> a(st.back());
                    st.pop_back();
                    st.push_back(shared_ptr<Token>(new XOrToken(a, b)));
                } else {
                    st.push_back(shared_ptr<Token>(new TagToken(curtok)));
                }
                curtok.clear();
            } // if
        } else
            curtok.append(s[i]); // build current token

    // left over tokens? not a critical error, but enough that it should be
    if (st.size() > 1)
        return false;

    // return whats on top of the stack
    if (st.empty())
        tok = shared_ptr<Token>(new TrueToken);
    else
        tok = st.back();

    return true;
}
