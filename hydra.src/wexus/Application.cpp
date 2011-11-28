
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Application.h>

#include <QDebug>

#include <assert.h>

#include <wexus/VarPath.h>
#include <wexus/AssertException.h>

using namespace wexus;

class Application::Route
{
  public:
    Route(Application *parent);

    virtual ~Route() { }

    /// returns true if handled
    virtual bool handleApplicationRequest(QStringList &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply) = 0;

  protected:
    Application *dm_parent;
};

class Application::MatchingRoute : public Application::Route
{
  public:
    MatchingRoute(Application *parent, QStringList &slist, const QVariantMap &defaults);

    virtual bool handleApplicationRequest(QStringList &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply);

  protected:
    struct Part {
      QString name;
      bool isVar;
      bool isOptional;
      Part(void) : isVar(false), isOptional(false) { }
    };
    typedef QVector<Part> PartList;

    PartList dm_parts;
    int dm_minparts, dm_maxparts;
    QVariantMap dm_defaults;
};

//
// Application::Route
//

Application::Route::Route(Application *parent)
  : dm_parent(parent)
{
  assert(dm_parent);
}

//
// Application::MatchingRoute
//

Application::MatchingRoute::MatchingRoute(Application *parent, QStringList &slist, const QVariantMap &defaults)
  : Route(parent), dm_defaults(defaults)
{
  dm_parts.resize(slist.size());

  dm_minparts = dm_maxparts = dm_parts.size();

  for (int i=0; i<dm_parts.size(); ++i) {
    const QString &instring = slist[i];
    int startindex = 0;

    if (instring.size() > startindex && instring[startindex] == '?') {
      if (i<dm_minparts)
        dm_minparts = i;
      dm_parts[i].isOptional = true;
      startindex++;
    }
    if (instring.size() > startindex && instring[startindex] == ':') {
      dm_parts[i].isVar = true;
      startindex++;
    }
    dm_parts[i].name = instring.mid(startindex);
//qDebug() << "part=" << dm_parts[i].name;
  }
}

bool Application::MatchingRoute::handleApplicationRequest(QStringList &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  // try to match dm_parts against filteredRequest
  int i;
  int sz = filteredRequest.size();

  if (sz<dm_minparts || sz>dm_maxparts)
    return false;

  if (dm_parts.size() < sz)
    sz = dm_parts.size();

  for (i=0; i<sz; ++i) {
    const QString &testString = filteredRequest[i];
    bool matched = true;
    if (!dm_parts[i].isVar)
      matched = dm_parts[i].name == testString;
    if (!matched) {
      // didnt match
      if (dm_parts[i].isOptional)
        break;    // its ok, it was optional, so just keep what we have
      else
        return false; // it was REQUIRED, return failture.
    }
  }

  int nummatched = i;   // i will point to the last unmatched entry

  // fill the map up with the variable matches
  QVariantMap callparams = dm_defaults;
  for (i=0; i<nummatched; ++i)
    if (dm_parts[i].isVar)
      callparams[dm_parts[i].name] = filteredRequest[i];

  // find the controller
  QString controllername = callparams["controller"].toString();
  QString actionname = callparams["action"].toString();

qDebug() << "MatchingRoute trying controllername=" << controllername << "actionname=" << actionname;

  if (!dm_parent->appInfo()->controllers.contains(controllername))
    return false;

  std::shared_ptr<Registry::ControllerInfo> controllerinfo = dm_parent->appInfo()->controllers[controllername];
  std::shared_ptr<Registry::ActionInfo> actioninfo;

  assert(controllerinfo.get());

  if (!controllerinfo->actions.contains(actionname))
    return false;

  actioninfo = controllerinfo->actions[actionname];
  assert(actioninfo.get());

  // finally, call the found controller and action
  {
    Context ctx(dm_parent, req, reply);

    {
      // merge my params over the Context params
      for (QVariantMap::const_iterator ii=callparams.begin(); ii!=callparams.end(); ++ii)
        ctx.params[ii.key()] = *ii;
    }

    // instantiate the controller
    std::shared_ptr<Controller> C( controllerinfo->loader() );

    assert(C.get());

    // call the action
    actioninfo->func(C.get());

    if (Context::reply().status() == 0)
      throw HTTPHandler::Exception("Controller called, but it didn't set any status (or send output)");
  }

  return true;
}

//
// Application::RouteBuilder
//

Application::RouteBuilder::RouteBuilder(Application &app)
  : dm_app(app)
{
  dm_app.dm_routes.clear();
}

Application::RouteBuilder::~RouteBuilder()
{
  assertThrow(!dm_app.dm_routes.isEmpty());
}

void Application::RouteBuilder::addMatch(const QString &matchString, const QVariantMap &defaults)
{
  QStringList parts = matchString.split('/', QString::SkipEmptyParts);

  dm_app.dm_routes.push_back(std::shared_ptr<Route>(new MatchingRoute(&dm_app, parts, defaults)));
}

void Application::RouteBuilder::addStandardRoutes(void)
{
  addMatch("/?:controller/?:action/", key("controller","home") + key("action","index"));
  addMatch("/:controller/:id/?:action/", key("action","show"));
  addMatch("/:pid/:controller/?:action/", key("action","index"));
  addMatch("/:pid/:controller/:id/?:action/", key("action","show"));
}

//
//
// Application
//
//

Application::Application(void)
{
  // make the default routing table
  RouteBuilder b(*this);
  b.addStandardRoutes();
}

Application::~Application()
{
}

void Application::setMountPoint(const QString &mountPoint)
{
  dm_mountpoint = mountPoint;
}

void Application::init(const QVariantMap &settings)
{
  initBasic(settings);

  openDB();
}

void Application::initBasic(const QVariantMap &settings)
{
  dm_settings = settings;
}

void Application::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  QStringList splitreq = filteredRequest.split('/', QString::SkipEmptyParts);

  for (RouteList::iterator ii=dm_routes.begin(); ii != dm_routes.end(); ++ii)
    if ((*ii)->handleApplicationRequest(splitreq, req, reply))
      return;
  //commented out to allow fall through to the next application
  //throw HTTPHandler::Exception("wexus::Application: not route found for " + filteredRequest);
}

std::shared_ptr<Registry::AppInfo> Application::appInfo(void)
{
  if (!dm_appinfo.get()) {
    dm_appinfo = Registry::appsByType()[typeid(*this).name()];

    assert(dm_appinfo.get());
  }

  return dm_appinfo;
}

void Application::openDB(void)
{
  assert(settings().contains("appdir"));
  QString dbpath = settings()["appdir"].toString() + "/database.sqlite";

  // setup DB
  dm_db = OpenDatabases::Handle(dbpath);
}

