
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/ParamsParser.h>

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

/*ParamsParser::ParamNotFoundException::ParamNotFoundException(const QString &fieldname)
  : wexus::HTTPHandler::Exception("field not found: " + fieldname)
{
}*/

ParamsParser::ParamsDecodeException::ParamsDecodeException(void)
  : wexus::HTTPHandler::Exception("ParamsDecodeException")
{
}

QVariantMap ParamsParser::parse(wexus::HTTPRequest *req)
{
  QVariantMap ret;

  parse(req, ret);

  return ret;
}

void ParamsParser::parse(wexus::HTTPRequest *req, QVariantMap &out)
{
  assert(req);

  if (!req->query().isEmpty())
    decodeAndParse(req->query().begin(), req->query().end(), out);

  // parse the post data
  if (req->contentLength() > 0) {
    // read the content length into a string and then process that
    // TODO optimizes this in the future to iterate over the Device
    // santify check this resize operation???
    QByteArray ary = req->input()->read(req->contentLength());

    decodeAndParse(ary.begin(), ary.end(), out);
  }

  /*IODeviceIterator ii(req->input());
  IODeviceIterator endii;

  decodeAndParse(ii, endii);*/
}

template <class C>
  inline char toChar(C c) { return c; }
template <>
  inline char toChar<QChar>(QChar c) { return c.toAscii(); }

template <class ITER>
  void ParamsParser::decodeAndParse(ITER encodedBegin, ITER encodedEnd, QVariantMap &out)
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
            throw ParamsDecodeException(); // fail as we've reached the end of the string before decoding this hex
          hexstr[0] = *ii;

          ++ii;
          if (ii == encodedEnd)
            throw ParamsDecodeException(); // fail as we've reached the end of the string before decoding this hex
          hexstr[1] = *ii;

          bool ok;
          int uni = hexstr.toInt(&ok, 16);

          if (!ok)
            throw ParamsDecodeException();

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
        nestedInsert(out, name, value);
        //(*this)[name] = value;
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
    nestedInsert(out, name, value);
    //(*this)[name] = value; // value parsed, insert name and value into map
}

void ParamsParser::nestedInsert(QVariantMap &m, const QString &key, const QString &val)
{
  int idx = key.indexOf('[');

  if (idx == -1) {
    // easy case
    m[key] = val;
    return;
  }

  // recursive case
  int endidx = key.lastIndexOf(']');

  if (endidx == -1)
    throw ParamsDecodeException();

  QString subkey(key.left(idx));
  QString recurkey(key.mid(idx+1, endidx-idx-1));

  // gets the current map based on the subkey
  QVariantMap submap = m[subkey].toMap();

  // recurse
  nestedInsert(submap, recurkey, val);

  // package up the map and set it back into the varient
  // TODO this is gross... should be a way to do this IN PLACE
  m[subkey] = submap;
}

