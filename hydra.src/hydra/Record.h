
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_RECORD_H__
#define __INCLUDED_HYDRA_RECORD_H__

#include <QDataStream>

namespace hydra {
class Record;
}

// forward, for the friendship thing
QDataStream &operator<<(QDataStream &stream, const hydra::Record &rec);
QDataStream &operator>>(QDataStream &stream, hydra::Record &rec);

/**
 * Something that is saved/load from the db.
 *
 * @author Aleksander Demko
 */
class hydra::Record {
  public:
    class Exception : public std::exception {
      public:
#ifdef HYDRA_ASSERT_RECORD_ERRORS
        Exception(void) {
            assert(false && "[Record::Exception thrown]\n");
        } // thrown during reads or writes
#endif
        virtual const char *what(void) const throw();
    };
    class IOException : public Exception {};     // raw IO error
    class FormatException : public Exception {}; // stream format error
    class VersionException : public Exception {
    }; // bad version of objects on stream

  public:
    virtual ~Record() {}

  protected:
    /// saver, throws exceptions on errors
    virtual void save(QDataStream &out) const = 0;
    /// loader, throws exceptions on errors
    virtual void load(QDataStream &in) = 0;

  private:
    friend QDataStream & ::operator<<(QDataStream &stream,
                                      const hydra::Record &rec);
    friend QDataStream & ::operator>>(QDataStream &stream, hydra::Record &rec);
};

// now declare their bodies as theyre inline
inline QDataStream &operator<<(QDataStream &stream, const hydra::Record &rec) {
    rec.save(stream);
    return stream;
}
inline QDataStream &operator>>(QDataStream &stream, hydra::Record &rec) {
    rec.load(stream);
    return stream;
}

#endif
