
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_EXCEPTION_H__
#define __INCLUDED_WEXUS_EXCEPTION_H__

#include <exception>

#include <QString>

namespace wexus
{
  class Exception;
}

/**
 * A helper class for many types of exceptions.
 * This has a buffer to store the contents of a pasted
 * QString suitable for what()
 *
 * @author Aleksander Demko
 */ 
class wexus::Exception : public std::exception
{
  public:
    /// usermessage constructor
    Exception(const QString &_what) throw();

    virtual ~Exception() throw();

    virtual const char* what() const throw() { return dm_whatbuf; }

  protected:
    // cant be a QString as then what() will return a * to a temporary
    QByteArray dm_whatbuf;
};

#endif

