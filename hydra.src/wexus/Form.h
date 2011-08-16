
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_FORM_H__
#define __INCLUDED_WEXUS_FORM_H__

#include <wexus/Widgets.h>
#include <wexus/ValidationExpr.h>
#include <wexus/Context.h>
#include <wexus/ActiveRecord.h>

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
     * Constructor.
     *
     * @author Aleksander Demko
     */ 
    Form(const QString &formname, const QString &rawurl = pathTo(), int method = Method_Post);
    /**
     * Constructor that uses the given ActiveRecord for default
     * values.
     *
     * @author Aleksander Demko
     */ 
    Form(ActiveRecord &rec, const QString &rawurl = pathTo(), int method = Method_Post);
    /// destructor
    ~Form();

    /**
     * A basicl text field.
     *
     * @author Aleksander Demko
     */
    wexus::HTMLString textField(const QString &fieldName, const QVariant &defaultVal = QVariant(), int sz = 30, int maxlen = 60, const ValidationExpr &valExpr = ValidationExpr()) const;

    /**
     * The submit button. fieldName is optional and not often used.
     *
     * @author Aleksander Demko
     */ 
    wexus::HTMLString submitButton(const QString &desc, const QString &fieldName = QString()) const;

  public:
    static void testFlashValidators(const QVariantMap &params,
        const QVariantMap &flash, Context::Errors &errors);

  protected:
    /**
     * Emits the form field to output
     *
     * @author Aleksander Demko
     */
    wexus::HTMLString fullFieldName(const QString &fieldName) const;

    /**
     * Returns the current value from the form. If there isn't
     * one, then this returns the current value of fieldName
     * in the active record. If there is no activerecord
     * for this form, then this returns QVariant (an inValid
     * QVariant)
     *
     * @author Aleksander Demko
     */ 
    QVariant formValue(const QString &fieldName) const;

  private:
    void outputHeader(const QString &rawurl, int method);

  protected:
    QString dm_formname;
    ActiveRecord *dm_rec; // might be null if one wasn't supplied
};

#endif

