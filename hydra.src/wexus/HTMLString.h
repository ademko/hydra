
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_HTMLSTRING_H__
#define __INCLUDED_WEXUS_HTMLSTRING_H__

#include <QString>
#include <QIODevice>
#include <QTextStream>

namespace wexus
{
  class HTMLString;
  class HTMLEncoderDevice;
}

/**
 * This is a QString that is safe for emiting directly to the
 * user (that is, it has been properly escaped)
 *
 * @author Aleksander Demko
 */ 
class wexus::HTMLString : public QString
{
  public:
    // empty constructor
    HTMLString(void);
    /**
     * Creates an HTMLString from the given s.
     * This basically marks s as already encoded.
     *
     * @author Aleksander Demko
     */ 
    explicit HTMLString(const QString &s);

    /**
     * HTML encode the given string and return
     * the encoded string.
     *
     * @author Aleksander Demko
     */ 
    static HTMLString encode(const QString &s);

    /// assignment operator
    HTMLString & operator = (const HTMLString &rhs) { *this = rhs; return *this; }
    /// assignment operator
    HTMLString & operator = (const QString &rhs) { QString::operator = (rhs); return *this; }
};

/**
 * Encoded any bytes to html before sending them to the
 * linked device.
 *
 * This is an output (write) device only.
 *
 * @author Aleksander Demko
 */ 
class wexus::HTMLEncoderDevice : public QIODevice
{
  public:
    /// constructor
    HTMLEncoderDevice(QIODevice *outputdev);

    void setEnabled(bool e) { dm_enabled = e; } // this is used in the operator<< HTMLString

  protected:
    // just fails
    virtual qint64 readData(char * data, qint64 maxSize);
    /// implemented here
    virtual qint64 writeData(const char * data, qint64 maxSize);

  private:
    bool dm_enabled;

    QIODevice *dm_dev;
    // a temporary buffer used to prep data before sending it
    // to the linked QIODevice
    QByteArray dm_buf;
};

QTextStream & operator << (QTextStream &o, const wexus::HTMLString &htmlString);

#endif

