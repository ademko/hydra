
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_WIDGETS_H__
#define __INCLUDED_WEXUS_WIDGETS_H__

#include <wexus/HTMLString.h>

#include <QVariant>

namespace wexus
{
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

  /**
   * Redirectst the user to the given url.
   * This must be called before any output, etc.
   *
   * @author Aleksander Demko
   */ 
  void redirectTo(const QString &rawurl);

  /**
   * Renders any errors.
   * Returns true if there are any errors.
   *
   * @author Aleksander Demko
   */ 
  bool renderErrors(void);
}

#endif

