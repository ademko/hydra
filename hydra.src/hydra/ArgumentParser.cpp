
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/ArgumentParser.h>

#include <assert.h>

using namespace hydra;

const char* ArgumentParser::Exception::what(void) const throw()
{
  return "ArgumentParser::Exception or decendant";
}

ArgumentParser::ErrorException::ErrorException(const QString &errormsg)
  : dm_msg(errormsg.toUtf8())
{
}

const char* ArgumentParser::ErrorException::what(void) const throw()
{
  return dm_msg.constData();
}

//
//
// ArgumentParser
//
//

ArgumentParser::ArgumentParser(const QStringList &args)
{
  parse(args);
}

bool ArgumentParser::hasNext(void) const
{
  return !dm_args.isEmpty();
}

bool ArgumentParser::hasNextParam(void) const
{
  for (QList<bool>::const_iterator ii=dm_isswitch.begin(); ii != dm_isswitch.end(); ++ii)
    if (! (*ii))
      return true;
  return false;
}

QString ArgumentParser::next(bool *isswitch)
{
  if (!hasNext())
    throw ErrorException("More parameters expected");

  QString ret(dm_args.front());

  if (isswitch)
    *isswitch = dm_isswitch.front();

  dm_args.pop_front();
  dm_isswitch.pop_front();

  return ret;
}

QString ArgumentParser::nextParam(const QString &switchName)
{
  if (!hasNextParam())
    throw ErrorException("Parameter expected for: " + switchName);

  // now find the param and remove it
  QStringList::iterator ii;
  QList<bool>::iterator bb;

  for (ii=dm_args.begin(), bb=dm_isswitch.begin(); *bb; ++ii, ++bb)
    assert(ii != dm_args.end()); //empty body

  QString ret(*ii);

  dm_args.erase(ii);
  dm_isswitch.erase(bb);

  return ret;
}

void ArgumentParser::parse(const QStringList &args)
{
  QStringList::const_iterator ii, endii;

  ii = args.begin();
  endii = args.end();

  for (; ii != endii; ++ii) {
    if (ii->startsWith("--")) {
      if (ii->contains('=')) {
        int index = ii->indexOf('=');
        assert(index >= 0);

        dm_args.push_back(ii->mid(0,index));
        dm_isswitch.push_back(true);
        dm_args.push_back(ii->mid(index+1, ii->size() - index - 1));
        dm_isswitch.push_back(false);
      } else {
        dm_args.push_back(*ii);
        dm_isswitch.push_back(true);
      }
    } else if (ii->startsWith("-"))
      pushSwitches(*ii);
    else {
      dm_args.push_back(*ii);
      dm_isswitch.push_back(false);
    }
  }
}

void ArgumentParser::pushSwitches(const QString &s)
{
  for (int i=1; i<s.size(); ++i) {
    dm_args.push_back(QString("-") + s[i]);
    dm_isswitch.push_back(true);
  }
}

