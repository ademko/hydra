
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/MongooseServer.h>

#include <QCoreApplication>
#include <QDirIterator>
#include <QSettings>
#include <QDebug>

#include <wexus/Site.h>
#include <wexus/Registry.h>
#include <pingapp/PingApp.h>

using namespace wexus;

static void settingsToMap(const QSettings &settings, QVariantMap &out)
{
  QStringList allkeys = settings.allKeys();

  for (QStringList::const_iterator ii=allkeys.begin(); ii != allkeys.end(); ++ii)
    out[*ii] = settings.value(*ii);
}

int main(int argc, char **argv)
{
  QCoreApplication coreapp(argc, argv);
  QString sitepath;

  // find the site dir
  {
    QStringList args = coreapp.arguments();

    if (args.size() < 2) {
      //sitepath = ".";
      qDebug() << "You must provide a site directory as a parameter.";
      return -1;
    } else
      sitepath = QFileInfo(args[1]).canonicalFilePath();;
  }

  // test the dir
  if (!QFileInfo(sitepath).isDir()) {
    qDebug() << sitepath << "is not a directory.";
    return -1;
  }

  QVariantMap siteoptions;

  // load  the site.ini, if any
  {
    QString sitefilename(sitepath + "/site.ini");
    QFileInfo info(sitefilename);

    if (info.isFile()) {
      qDebug() << "parsing INI " << sitefilename;
      QSettings settings(sitefilename, QSettings::IniFormat);

      settingsToMap(settings, siteoptions);
    }
  }

  HTTPParams httpparams;

  // convert some options into httpparams for Site
  if (siteoptions.contains("httpport"))
    httpparams.setPort(siteoptions["httpport"].toInt());

  // in the future, pass siteoptions directly to Site too?
  wexus::Site s(sitepath, httpparams);

  // load all the app files
  {
    QDirIterator dd(sitepath, QDirIterator::Subdirectories);

    while (dd.hasNext()) {
      QString fullfilename(dd.next());
      //QString fullfilename(QFileInfo(sitepath + "/" + relfilename).canonicalFilePath());
      QFileInfo info(fullfilename);

//qDebug() << fullfilename;
      if (info.isFile() && info.fileName() == "app.ini") {
        qDebug() << "parsing INI " << fullfilename;

        // load the ini file
        QSettings settings(fullfilename, QSettings::IniFormat);

        // convert the settings object to a variant map
        QVariantMap vmap = siteoptions;

        settingsToMap(settings, vmap);

        if (!vmap.contains("app")) {
          qDebug() << "no app= line in the .ini file";
          return -1;
        }

        QString appname = vmap["app"].toString();

        // find the app
        if (!Registry::appsByName().contains(appname)) {
          qDebug() << "app not found:" << appname;
          return -1;
        }

        std::shared_ptr<Registry::AppInfo> appinfo = Registry::appsByName()[appname];
        assert(appinfo.get());

        QString mountpoint(info.path().mid(sitepath.size()) + "/");

        qDebug() << "launching" << appinfo->appname << "@" << mountpoint;

        std::shared_ptr<Application> app(appinfo->loader());
        assert(app.get());

        // the computed versions always override what's in the file
        vmap["mountpoint"] = mountpoint;
        vmap["sitedir"] = sitepath;
        vmap["appdir"] = info.path();

//qDebug() << vmap;
        // config the app
        app->setSettings(vmap);
        // add it to the site
        s.addApplication(mountpoint, app);
      }
    }
  }

  //s.addApplication("/pinger/", std::shared_ptr<pingapp::PingApp>(new pingapp::PingApp));

  s.start();
  s.wait();
}

