
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
  /**
   * Comptes the a url to this location.
   *
   * @author Aleksander Demko
   */
  QString pathTo(void);

  // internal function for linkTo
  // throws on errors
  // returns a raw url
  QString memberFunctionToUrl(const QString controllertype, const MemberFunction &mfn, const QVariant *_params);
  // internal function for linkTo
  // throws on errors
  // returns a raw url
  // _params can be a QVariant (id= assumed) or a map
  QString variantParamsToUrl(const QVariant &_params);

  template <class CONTROLLER>
    QString pathTo(void (CONTROLLER::*mfn)(void)) {
      return memberFunctionToUrl(typeToString<CONTROLLER>(), MemberFunction(mfn), 0);
    }
  template <class CONTROLLER>
    QString pathTo(void (CONTROLLER::*mfn)(void), const QVariant &_params) {
      return memberFunctionToUrl(typeToString<CONTROLLER>(), MemberFunction(mfn), &_params);
    }

  /**
   * Returns the HTML code for a link to rawurl with the given
   * description.
   *
   * @author Aleksander Demko
   */ 
  wexus::HTMLString linkTo(const QString &desc, const QString &rawurl);

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

