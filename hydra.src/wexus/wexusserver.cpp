
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
#include <wexus/FileApp.h>

using namespace wexus;

static void settingsToMap(const QSettings &settings, QVariantMap &out)
{
  QStringList allkeys = settings.allKeys();

  for (QStringList::const_iterator ii=allkeys.begin(); ii != allkeys.end(); ++ii)
    out[*ii] = settings.value(*ii);
}

void showHelp(void)
{
  qDebug() << 
    "how to run:\n\n"
    "wexusserver base_site_directory\n\n"
    "  base_site_directory  is the base directory for the web site\n\n"
    "  site.ini     the base_site_directory can contain a site.ini file,\n"
    "               which can have the following options:\n"
    "                httpport = 8080\n"
    "                httpthreads = 4\n\n"
    "  app.ini      any subdirectory can have an app.ini, which launches\n"
    "               an application in that subdirectory\n"
    "                app = application type (see list below) (required field)\n"
    "\n"
    "               computed values:\n"
    "                mountpoint the web address (e.g. /blog)\n"
    "                sitedir    the directory of the base site\n"
    "                appdir     the directory of the app within the site\n"
    "                headerdir  the directory of header.html and footer.html\n"
    "                           can be overidden, but will default to headers/\n"
    "                           if they exist in appdir or siteapp\n"
    "                           can contain: header.html footer.html\n"
    "\n";

  qDebug() << "Currently available app types:";
  for (Registry::AppNameMap::const_iterator ii=Registry::appsByName().begin();
      ii != Registry::appsByName().end(); ++ii) {
    qDebug() << "  " << ii.key();
  }
}

static void addAppFromSettings(Site &site, const QFileInfo &appini, const QString &appname,
    const QString &sitepath, QVariantMap &vmap)
{
  std::shared_ptr<Registry::AppInfo> appinfo = Registry::appsByName()[appname];
  assert(appinfo.get());

  QString mountpoint(appini.path().mid(sitepath.size()) + "/");

  qDebug() << "  Launching (" << appinfo->appname << ") at " << mountpoint;

  std::shared_ptr<Application> app(appinfo->loader());
  assert(app.get());

  // the computed versions always override what's in the file
  vmap["mountpoint"] = mountpoint;
  vmap["sitedir"] = sitepath;
  vmap["appdir"] = appini.path();

  // calculate headerdir, if unset
  if (!vmap.contains("headerdir")) {
    if (QFileInfo(appini.path() + "/headers").isDir())
      vmap["headerdir"] = appini.path() + "/headers";
    if (QFileInfo(sitepath + "/headers").isDir())
      vmap["headerdir"] = sitepath + "/headers";
  }

  //qDebug() << vmap;
  // config the app
  app->init(vmap);
  // add it to the site
  site.addApplication(mountpoint, app);
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
      qDebug() << "You must provide a site directory as a parameter.\n";

      showHelp();
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
      qDebug() << "INI File: " << sitefilename;
      QSettings settings(sitefilename, QSettings::IniFormat);

      settingsToMap(settings, siteoptions);
    }
  }

  HTTPParams httpparams;

  // convert some options into httpparams for Site
  if (siteoptions.contains("httpport"))
    httpparams.setPort(siteoptions["httpport"].toInt());
  if (siteoptions.contains("httpthreads"))
    httpparams.setNumThreads(siteoptions["httpthreads"].toInt());

  // in the future, pass siteoptions directly to Site too?
  wexus::Site s(sitepath, httpparams);

  // load all the app files
  int appsloaded = 0;
  {
    QDirIterator dd(sitepath, QDirIterator::Subdirectories);

    while (dd.hasNext()) {
      QString fullfilename(dd.next());
      //QString fullfilename(QFileInfo(sitepath + "/" + relfilename).canonicalFilePath());
      QFileInfo info(fullfilename);

//qDebug() << fullfilename;
      if (info.isFile() && info.fileName() == "app.ini") {
        qDebug() << "INI File: " << fullfilename;

        // load the ini file
        QSettings settings(fullfilename, QSettings::IniFormat);

        // convert the settings object to a variant map
        QVariantMap vmap = siteoptions;

        settingsToMap(settings, vmap);

        if (vmap.contains("app")) {
          QString appname = vmap["app"].toString();

          if (!Registry::appsByName().contains(appname)) {
            qDebug() << fullfilename << ": app not found:" << appname;
            return -1;
          }

          addAppFromSettings(s, info, appname, sitepath, vmap);
        } else {
          QStringList groupnames = settings.childGroups();

          if (groupnames.isEmpty()) {
            qDebug() << "no app= line or groups in the .ini file";
            return -1;
          }

          for (QStringList::const_iterator group=groupnames.begin(); group != groupnames.end(); ++group) {
            QVariantMap groupmap;

            settings.beginGroup(*group);
            settingsToMap(settings, groupmap);
            settings.endGroup();

            if (!groupmap.contains("app")) {
              qDebug() << "app= not found in group :" << *group;
              return -1;
            }

            QString appname = groupmap["app"].toString();

            if (!Registry::appsByName().contains(appname)) {
              qDebug() << fullfilename << ": app not found (in group \"" << *group << "\" :" << appname;
              return -1;
            }

            addAppFromSettings(s, info, appname, sitepath, groupmap);
            appsloaded++;
          }//for
        }
      }//if app.init
    }//whild dd
  }

  // any apps loaded?
  if (appsloaded == 0) {
    std::shared_ptr<Application> fileapp(new FileApp(sitepath));
    s.addApplication("/", fileapp);
  }

  s.start();
  s.wait();
}

