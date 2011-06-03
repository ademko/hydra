
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/FormParams.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

FormParams::ParamNotFoundException::ParamNotFoundException(const QString &fieldname)
  : wexus::HTTPHandler::Exception("field not found: " + fieldname)
{
}

FormParams::FormDecodeException::FormDecodeException(void)
  : wexus::HTTPHandler::Exception("FormDecodeException")
{
}

FormParams::FormParams(void)
{
  dm_req = 0;
  dm_processedreq = false;
}

void FormParams::setupRequest(wexus::HTTPRequest *req)
{
  dm_req = req;
  assert(req);

  dm_processedreq = false;

  dm_params.clear();
}

bool FormParams::has(const QString &paramName)
{
  parseRequest();

  return dm_params.find(paramName) != dm_params.end();
}

/*const QVariant & FormParams::param(const QString &paramName)
{
  parseRequest();

  params_t::const_iterator ii = dm_params.find(paramName);

  if (ii == dm_params.end())
    return dm_emptyvariant;
  else
    return ii->second;
}*/

const QVariant & FormParams::operator[](const QString &paramName)
{
  parseRequest();

  params_t::const_iterator ii = dm_params.find(paramName);

  if (ii == dm_params.end())
    throw ParamNotFoundException(paramName);
  else
    return ii->second;
}

void FormParams::parseRequest(void)
{
  if (dm_processedreq)
    return;
  dm_processedreq = true;

  assert(dm_req);

  if (!dm_req->query().isEmpty()) {
    decodeAndParse(dm_req->query().begin(), dm_req->query().end());
  }

  // parse the POST stuff?
}

void FormParams::decodeAndParse(QString::const_iterator encodedBegin, QString::const_iterator encodedEnd)
{
  // currently iterating over the name?
  bool is_name = true;

  QString name, value, hexstr;

  name.reserve(64);
  value.reserve(64);
  hexstr.resize(2);

  QString::const_iterator ii;
  for (ii = encodedBegin; ii != encodedEnd; ii++) {
    switch (ii->toAscii()) {
      // convert all + chars to space chars
      case '+':
        (is_name ? name : value) += ' ';
        break;
      // convert all %xy hex codes into ASCII chars
      case '%':
        {
          if (ii+1 == encodedEnd || ii+2 == encodedEnd)
            throw FormDecodeException(); // fail as we've reached the end of the string before decoding this hex

          // copy the two bytes following the %
          hexstr[0] = *(ii+1);
          hexstr[1] = *(ii+2);

          // skip over the hex (next iteration will advance enc one more position)
          ii += 2;

          bool ok;
          int uni = hexstr.toInt(&ok, 16);

          if (!ok)
            throw FormDecodeException();

          // convert the hex to ASCII
          (is_name ? name : value).append(QChar(uni));
        }
        break;
      case '=':
        // begin parsing the value
         is_name = false;
         break;
      case '&':
        // begin parsing a new name
        is_name = true;
        // insert stored name and value into map
        dm_params[name] = value;
        // clear name/value strings for next pair
        name.clear();
        value.clear();
        break;
      // make an exact copy of anything else
      default:
        (is_name ? name : value) += *ii;
        break;
    }
  }

  if (!is_name)
    dm_params[name] = value; // value parsed, insert name and value into map
}

