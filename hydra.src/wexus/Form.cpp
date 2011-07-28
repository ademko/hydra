
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Form.h>

#include <wexus/Context.h>
#include <wexus/VarPath.h>

using namespace wexus;

Form::Form(const QString &formname, const QString &rawurl, int method)
  : dm_formname(formname)
{
  output() << "<FORM ";

  switch (method) {
    case Method_Post: output() << "METHOD=\"POST\""; break;
    case Method_Get: output() << "METHOD=\"GET\""; break;
    case Method_Upload: output() << "METHOD=\"POST\" ENCTYPE=\"multipart/form-data\""; break;
  }

  output() << "ACTION=\"" << rawurl << "\">\n";
}

Form::~Form()
{
  output() << "</FORM>\n";
}

wexus::HTMLString Form::textField(const ValidationExpr &valExpr, const QString &fieldName, const QVariant &defaultVal, int sz, int maxlen) const
{
  HTMLString ret;
  QString def = formValue(fieldName).toString();

  ret.reserve(100);
 
  ret += "<INPUT TYPE=\"TEXT\" NAME=\"";
  ret += fullFieldName(fieldName);
  ret += "\" VALUE=\"";
  if (def.isEmpty())
    ret += HTMLString::encode(defaultVal.toString());
  else
    ret += HTMLString::encode(def);
  ret += "\" SIZE=\"";
  ret += QString::number(sz);
  ret += "\" MAXLENGTH=\"";
  ret += QString::number(maxlen);
  ret += "\" />\n";
    
  // save the validation expression
  if (!valExpr.isNull()) {
    VarPath p(Context::instance()->setFlash);
  
    p["validators"][dm_formname][fieldName] = valExpr.toVariant();
  }

  return ret;
}

wexus::HTMLString Form::submitButton(const QString &desc, const QString &fieldName) const
{
  HTMLString ret;

  ret.reserve(128);
 
  ret += "<INPUT TYPE=\"SUBMIT\" VALUE=\""; 
  ret += HTMLString::encode(desc);
  if (!fieldName.isEmpty()) {
    ret += "\" NAME=\"";
    ret += fullFieldName(fieldName);
  }
  ret += "\" />\n";
    
  return ret;

}

static void testFlashValidatorsDriver(const QVariantMap &params, const QVariantMap &validators, QStringList &errors)
{
  QVariantMap::const_iterator ii, endii;
  QVariantMap::const_iterator vv;

  endii = params.end();
  for (ii = params.begin(); ii != endii; ++ii) {
    // check if there is a validator for this param
    vv = validators.find(ii.key());
    if (vv == validators.end())
      continue;
    // if its a map, we recurse
    if (vv->type() == QVariant::Map)
      testFlashValidatorsDriver(asVariantMap(*ii), asVariantMap(*vv), errors);
    else {
      // its not a map, execute the validator
      ValidationExpr validexpr = ValidationExpr::fromVariant(*vv);

      validexpr.test(*ii, &errors);
    }
  }
}

void Form::testFlashValidators(const QVariantMap &params, const QVariantMap &flash, QStringList &errors)
{
  if (!flash.contains("validators"))
    return;
  testFlashValidatorsDriver(params, asVariantMap(flash["validators"]), errors);
}

wexus::HTMLString Form::fullFieldName(const QString &fieldName) const
{
  HTMLString ret;
  // html encode the field and form names?

  if (dm_formname.isEmpty())
    ret = fieldName;
  else
    ret = dm_formname + "[" + fieldName + "]";

  return ret;
}

QVariant Form::formValue(const QString &fieldName) const
{
//qDebug() << "formValue" << dm_formname << fieldName << Context::instance()->params;
  if (dm_formname.isEmpty())
    return Context::instance()->params[fieldName];
  else
    return Context::instance()->params[dm_formname].toMap()[fieldName];
}

