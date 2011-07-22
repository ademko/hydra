
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Widgets.h>

#include <wexus/Context.h>

#include <QDebug>

using namespace wexus;

QString wexus::urlTo(void)
{
  return Context::instance()->request().request();
}

HTMLString wexus::linkTo(const QString &desc, const QString &rawurl)
{
  return HTMLString("<A HREF=\"" + rawurl + "\">" + HTMLString::encode(desc) + "</A>");
}

void wexus::redirectTo(const QString &rawurl)
{
  Context::instance()->reply().redirectTo(rawurl);
}

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
  //if (!valExpr.isNull())
    //VarPath(setFlash)["validation"][dm_formname][fieldName] = valExpr.toVariant();
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

