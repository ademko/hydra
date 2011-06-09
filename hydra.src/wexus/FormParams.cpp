
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/FormParams.h>

#include <assert.h>

#include <QDebug>

using namespace wexus;

/**
 * Utility class that allows input iteration
 * over a QIODevice.
 * This is a candinate for promotion for more general/wide-spread use.
 *
 * @author Aleksander Demko
 */ 
/*class IODeviceIterator
{
  public:
    IODeviceIterator(QIODevice *dev = 0);

    QChar operator *(void) const { return dm_c; }

    char operator ++(void) { assert(dm_dev); dm_dev->getChar(&dm_c); return dm_c; }

    bool operator ==(const IODeviceIterator &rhs) const {
      if (dm_dev && rhs.dm_dev)
        return dm_dev == rhs.dm_dev;
      if (!dm_dev && !rhs.dm_dev)
        return false;
      // otherwise, one of the devs is empty, so do an end check
      QIODevice *tocheck = dm_dev ? dm_dev : rhs.dm_dev;

      return !tocheck->atEnd();
    }

    inline bool operator !=(const IODeviceIterator &rhs) const {
      return !(*this == rhs);
    }

  private:
    QIODevice *dm_dev;
    char dm_c;
};

IODeviceIterator::IODeviceIterator(QIODevice *dev)
  : dm_dev(dev), dm_c(0)
{
}*/

FormParams::ParamNotFoundException::ParamNotFoundException(const QString &fieldname)
  : wexus::HTTPHandler::Exception("field not found: " + fieldname)
{
}

FormParams::FormDecodeException::FormDecodeException(void)
  : wexus::HTTPHandler::Exception("FormDecodeException")
{
}

FormParams::FormParams(wexus::HTTPRequest *req)
{
  dm_req = req;
  assert(req);

  dm_processedreq = false;
}

bool FormParams::contains(const QString &paramName)
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

  if (!dm_req->query().isEmpty())
    decodeAndParse(dm_req->query().begin(), dm_req->query().end());

  // parse the post data
  if (dm_req->contentLength() > 0) {
    // read the content length into a string and then process that
    // TODO optimizes this in the future to iterate over the Device
    // santify check this resize operation???
    QByteArray ary = dm_req->input()->read(dm_req->contentLength());

    decodeAndParse(ary.begin(), ary.end());
  }

  /*IODeviceIterator ii(dm_req->input());
  IODeviceIterator endii;

  decodeAndParse(ii, endii);*/
}

template <class C>
  inline char toChar(C c) { return c; }
template <>
  inline char toChar<QChar>(QChar c) { return c.toAscii(); }

template <class ITER>
  void FormParams::decodeAndParse(ITER encodedBegin, ITER encodedEnd)
{
  // currently iterating over the name?
  bool is_name = true;

  QString name, value, hexstr;

  name.reserve(64);
  value.reserve(64);
  hexstr.resize(2);

  ITER ii;
  for (ii = encodedBegin; ii != encodedEnd; ++ii) {
    switch (toChar(*ii)) {
      // convert all + chars to space chars
      case '+':
        (is_name ? name : value) += ' ';
        break;
      // convert all %xy hex codes into ASCII chars
      case '%':
        {
          ++ii;
          if (ii == encodedEnd)
            throw FormDecodeException(); // fail as we've reached the end of the string before decoding this hex
          hexstr[0] = *ii;

          ++ii;
          if (ii == encodedEnd)
            throw FormDecodeException(); // fail as we've reached the end of the string before decoding this hex
          hexstr[1] = *ii;

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

