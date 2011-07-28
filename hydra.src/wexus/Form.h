
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FORM_H__
#define __INCLUDED_WEXUS_FORM_H__

#include <wexus/Widgets.h>
#include <wexus/ValidationExpr.h>

namespace wexus
{
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
    wexus::HTMLString textField(const ValidationExpr &valExpr, const QString &fieldName, const QVariant &defaultVal = QVariant(), int sz = 30, int maxlen = 60) const;

    /**
     * The submit button. fieldName is optional and not often used.
     *
     * @author Aleksander Demko
     */ 
    wexus::HTMLString submitButton(const QString &desc, const QString &fieldName = QString()) const;

  public:
    static void testFlashValidators(const QVariantMap &params,
        const QVariantMap &flash, QStringList &errors);

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

