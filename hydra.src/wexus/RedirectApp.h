
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_REDIRECTAPP_H__
#define __INCLUDED_WEXUS_REDIRECTAPP_H__

#include <wexus/Application.h>

namespace wexus
{
  class RedirectApp;
}

/**
 * An app that, based on a list, does redirections.
 *
 * INI parameters:
 *  linkX = "/src dest"
 *    X is a number, 1, 2, etc
 *    /src is url to match, leading / is required
 *    dest is the url to return, can be relative to have a / or hostname, etc
 *
 * @author Aleksander Demko
 */ 
class wexus::RedirectApp : public wexus::Application
{
  public:
    RedirectApp(void);

    virtual void init(const QVariantMap &settings);

    virtual void handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    QMap<QString, QString> dm_redirs;
};


#endif

