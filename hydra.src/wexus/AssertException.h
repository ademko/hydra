
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_ASSERT_H__
#define __INCLUDED_WEXUS_ASSERT_H__

#include <assert.h>

#include <QVariant>
#include <QString>

#include <wexus/Exception.h>

namespace wexus
{
  /**
   * An assert exception. These are critical and shouldn't be user-presented.
   *
   * @author Aleksander Demko
   */ 
  class AssertException : public wexus::Exception
  {
    public:
      /// constructor
      AssertException(const char *msg);
      /// for QString
      AssertException(const QString &msg);
  };

  /**
   * If b is false, an AssertException will be thrown
   * with the given msg.
   *
   * @author Aleksander Demko
   */
  void assertThrowMsg(bool b, const char *msg);

#define assertThrow_QUOTEME_(x) #x
#define assertThrow_QUOTEME(x) assertThrow_QUOTEME_(x)

  /**
   * assertThrow is always defined, debug mode or not.
   *
   * @author Aleksander Demko
   */ 
#define assertThrow(b) \
  wexus::assertThrowMsg((b), "assertThrow failed at: " __FILE__ ":" assertThrow_QUOTEME(__LINE__) )
// so we can switch between them sometimes :)
#define assertThrow__(b) assert(b)

  class AssertThrower
  {
    bool operator = (bool b);
    const QVariant & operator = (const QVariant &v);
    const QString & operator = (const QString &s);
    // int version too? conflicts with bool?
    // ActiveRecord version?
  };

  /**
   * The global instance of AssertThrower that you can use.
   *
   * @author Aleksander Demko
   */ 
  extern AssertThrower required;
}

#endif

