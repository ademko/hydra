
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
   * An HTML form.
   * This emits a complete form via the output() stream.
   *
   * @author Aleksander Demko
   */ 
  class Form;
}

/**
 * A general HTML form.
 *
 * @author Aleksander Demko
 */ 
class wexus::Form
{
  public:
    enum {
      Method_Post,
      Method_Get,
      Method_Upload,
    };

  public:
    /**
     * Constrcutor.
     *
     * @author Aleksander Demko
     */ 
    Form(const QString &formname, const QString &rawurl = urlTo(), int method = Method_Post);
    /// destructor
    ~Form();

    /**
     * A basicl text field.
     *
     * @author Aleksander Demko
     */
    wexus::HTMLString textField(const QString &fieldName, const QVariant &defaultVal = QVariant(), int sz = 30, int maxlen = 60) const;

    /**
     * The submit button. fieldName is optional and not often used.
     *
     * @author Aleksander Demko
     */ 
    wexus::HTMLString submitButton(const QString &desc, const QString &fieldName = QString()) const;

  protected:
    /**
     * Emits the form field to output
     *
     * @author Aleksander Demko
     */
    wexus::HTMLString fullFieldName(const QString &fieldName) const;

    QVariant formValue(const QString &fieldName) const;

  private:
    QString dm_formname;
};

#endif

