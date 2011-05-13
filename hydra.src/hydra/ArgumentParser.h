
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef INCLUDED_HYDRA_ARGUMENTPARSER_H__
#define INCLUDED_HYDRA_ARGUMENTPARSER_H__

#include <QCoreApplication>
#include <QStringList>

namespace hydra
{
  class ArgumentParser;
}

/**
 * Parses command line arguments ("parameters") in a QDirIterator-like manner.
 *
 * The first parameter is always the program name itself. You can remove
 * it by calling next() immediatly.
 *
 * Typically, you repeadedly call next() in a while(hasNext()) loop.
 * You can also has* methods
 * to forward-inspect the stream.
 *
 * @author Aleksander Demko
 */ 
class hydra::ArgumentParser
{
  public:
    class Exception : public std::exception
    {
      public:
        virtual const char* what(void) const throw();
    };
    class ErrorException : public Exception {
      public:
        ErrorException(const QString &errormsg);
        virtual ~ErrorException() throw () { }
        virtual const char* what(void) const throw();
      private:
        QByteArray dm_msg;
    };
    //class BadParamException
    class HelpException : public Exception { };

  public:
    /**
     * Initializes the parser with the given argument list.
     * It uses the QCoreApplication arguments by default.
     *
     * @author Aleksander Demko
     */ 
    ArgumentParser(const QStringList &args = QCoreApplication::arguments());

    /**
     * Is there a switch or a parameter available?
     * A switch begins with a - or --, everything else is a parameter.
     * Note the the program name is the first parameter, always.
     *
     * @author Aleksander Demko
     */ 
    bool hasNext(void) const;
    /**
     * Is there a parameter available?
     *
     * @author Aleksander Demko
     */ 
    bool hasNextParam(void) const;

    /**
     * Returns the next switch or parameter.
     * Examples "-s", "--switch", "blah". Switches are returned verbatem,
     * hyphens and all.
     *
     * Throws ArgsEmptyException on error. You can pre-test via hasNext
     *
     * @author Aleksander Demko
     */ 
    QString next(bool *isswitch = 0);

    /**
     * Gets the next param, throws if empty.
     *
     * @param switchName is only used for exception generation purposes
     * @author Aleksander Demko
     */ 
    QString nextParam(const QString &switchName = 0);

  private:
    void parse(const QStringList &args);
    void pushSwitches(const QString &s);

  private:
    QStringList dm_args;
    QList<bool> dm_isswitch;
};

#endif

