
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_WIDGETS_H__
#define __INCLUDED_WEXUS_WIDGETS_H__

#include <wexus/HTMLString.h>
#include <wexus/MemberFunction.h>

#include <QVariant>

namespace wexus
{
  class ActiveRecord;//fwd

  /**
   * Comptes the a url to this location.
   *
   * @author Aleksander Demko
   */
  QString urlTo(void);

  /**
   * Returns the HTML code for a link to rawurl with the given
   * description.
   *
   * @author Aleksander Demko
   */ 
  wexus::HTMLString linkTo(const QString &desc, const QString &rawurl);

  // internal function for linkTo
  // throws on errors
  // returns a raw url
  QString memberFunctionToUrl(const QString controllertype, const MemberFunction &mfn);
  // internal function for linkTo
  // throws on errors
  // returns a raw url
  QString recToIdUrl(wexus::ActiveRecord &rec);

  template <class CONTROLLER>
    wexus::HTMLString linkTo(const QString &desc, void (CONTROLLER::*mfn)(void)) {
      // de-inline this function
      return linkTo(desc, memberFunctionToUrl(typeToString<CONTROLLER>(), MemberFunction(mfn)));
    }

  template <class CONTROLLER>
    wexus::HTMLString linkTo(const QString &desc, void (CONTROLLER::*mfn)(void), wexus::ActiveRecord &rec) {
      // de-inline this function
      return linkTo(desc, memberFunctionToUrl(typeToString<CONTROLLER>(), MemberFunction(mfn))
          + recToIdUrl(rec));
    }

  /**
   * Redirectst the user to the given url.
   * This must be called before any output, etc.
   *
   * An empty string (the default) redirects to self. This is a bonus
   * feature to this function that is not in HTTPReply::redirectTo().
   *
   * @author Aleksander Demko
   */ 
  void redirectTo(const QString &rawurl = QString());

  /**
   * Renders any errors.
   * Returns true if there are any errors.
   *
   * @author Aleksander Demko
   */ 
  void renderErrors(void);

  /**
   * Renders flash["notice"]
   *
   * @author Aleksander Demko
   */ 
  void renderNotice(void);
}

#endif

