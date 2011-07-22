
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_VARPATH_H__
#define __INCLUDED_WEXUS_VARPATH_H__

#include <QVariant>

namespace wexus
{
  class VarPath;

  /**
   * This is a powerful function that lets you
   * access the QVariantMap map within the v
   * (no copies!).
   *
   * Uses internal Qt access - might need
   * to be updated in the future.
   *
   * @author Aleksander Demko
   */ 
  QVariantMap & asVariantMap(QVariant &v);
}

/**
 * A helper to make accessing and setting QVariantMap
 * easier.
 *
 * @author Aleksander Demko
 */ 
class wexus::VarPath
{
  public:
    class Exception : public std::exception {
      virtual const char* what() const throw ()
      { return "VarPath::Exception"; }
    };
    class TypeException : public Exception {};
    class NotFoundException : public Exception {};

  public:
    /**
     * Createa a reference from a map
     * @author Aleksander Demko
     */ 
    VarPath(QVariantMap &srcmap);

    /**
     * Creates a reference from a variable.
     *
     * @author Aleksander Demko
     */ 
    VarPath(QVariant &v);

    /**
     * Returns a sub node. This reference must be set
     * to a map or a variant that is a map.
     *
     * @author Aleksander Demko
     */ 
    VarPath operator [](const QString &key);

    /**
     * Const version of []. Throws the field is not found.
     *
     * @author Aleksander Demko
     */ 
    //VarPath operator ()(const QString &key) const;

    /**
     * Assignment operator.
     *
     * This reference must be set to a variant.
     *
     * @author Aleksander Demko
     */ 
    VarPath & operator = (const QVariant &v);

    /**
     * Returns this reference as a note.
     * This reference must be set to a variant.
     *
     * @author Aleksander Demko
     */ 
    QVariant & asVariant(void);

    /**
     * Returns this reference as a map.
     * This reference must be set to a map.
     *
     * @author Aleksander Demko
     */ 
    QVariantMap & asMap(void);

  protected:
    QVariantMap *dm_map;
    QVariant *dm_node;
};

#endif


