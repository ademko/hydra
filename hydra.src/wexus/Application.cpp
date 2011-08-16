
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Application.h>

#include <QDebug>

#include <assert.h>

#include <wexus/VarPath.h>

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
    int dm_reqparts;
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

  dm_reqparts = dm_parts.size();

  for (int i=0; i<dm_parts.size(); ++i) {
    const QString &instring = slist[i];
    int startindex = 0;

    if (instring.size() > startindex && instring[startindex] == '?') {
      if (i<dm_reqparts)
        dm_reqparts = i;
      dm_parts[i].isOptional = true;
      startindex++;
    }
    if (instring.size() > startindex && instring[startindex] == ':') {
      dm_parts[i].isVar = true;
      startindex++;
    }
    dm_parts[i].name = instring.mid(startindex);
qDebug() << "part=" << dm_parts[i].name;
  }
}

bool Application::MatchingRoute::handleApplicationRequest(QStringList &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  // try to match dm_parts against filteredRequest
  int i;
  int sz = filteredRequest.size();

  if (sz<dm_reqparts)
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
    Context ctx(dm_parent, actionname, req, reply);

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
//
// Application
//
//

Application::Application(void)
{
  // setup DB
  dm_db = QSqlDatabase::addDatabase("QSQLITE", "C1");
  assert(dm_db.isValid());

  dm_db.setDatabaseName("/tmp/one.sqlite");
  bool good = dm_db.open();
  assert(good);

  // make the default routing table
  QStringList l;
  l << "?:controller" << "?:action";
  dm_routes.push_back(std::shared_ptr<Route>(new MatchingRoute(this, l, key("controller","home") + key("action","index"))));
}

Application::~Application()
{
  // tear down DB
  if (dm_db.isOpen()) {
    dm_db.close();

    dm_db = QSqlDatabase(); // we need to "null" dm_db otherwise removeDatabasew ill think its open

    QSqlDatabase::removeDatabase("C1");
  }
}

void Application::setMountPoint(const QString &mountPoint)
{
  dm_mountpoint = mountPoint;
}

void Application::handleApplicationRequest(QString &filteredRequest, wexus::HTTPRequest &req, wexus::HTTPReply &reply)
{
  QStringList splitreq = filteredRequest.split('/', QString::SkipEmptyParts);

  for (RouteList::iterator ii=dm_routes.begin(); ii != dm_routes.end(); ++ii)
    if ((*ii)->handleApplicationRequest(splitreq, req, reply))
      return;
  throw ControllerNotFoundException("wexus::Application: not route found for " + filteredRequest);

return;
  QString controllername(filteredRequest.mid(1));
  QString action("index");

  if (controllername.isEmpty()) {
    //controllername = "home";
    // if no controllername is supplied, redir the user to an explicit default one
    QString redir_url(req.request());

    if (redir_url[redir_url.size()-1] != '/')
      redir_url += "/";
    redir_url += "home/";

    reply.redirectTo(redir_url);
    return;
  } else {
    int index = controllername.indexOf("/");

    // must trail in a /
    // (this is so simple linkTo()s work nicely
    if (index == -1) {
      QString redir_url(req.request());

      redir_url += "/";
      reply.redirectTo(redir_url);
      return;
    }

    assert(index != -1);

    if (index+1 < controllername.size())
      action = controllername.mid(index+1);
    controllername = controllername.left(index);
  }

  // compute the action
  // find where the action starts
qDebug() << "Application::handleApplicationRequest" << filteredRequest << "controllername" << controllername << "action" << action;

  // get my own info object
  // this cant be done in the ctor, as the decendant object isnt built yet
  // see if we a have controller for this request
  if (appInfo()->controllers.contains(controllername)) {
    Context ctx(this, action, req, reply);

    std::shared_ptr<Controller> C( appInfo()->controllers[controllername]->loader() );

    assert(C.get());

    C->handleControllerRequest(action);
  } else
    throw ControllerNotFoundException("wexus::Application: Controller not found: " + controllername);
}

std::shared_ptr<Registry::AppInfo> Application::appInfo(void)
{
  if (!dm_appinfo.get()) {
    dm_appinfo = Registry::appsByType()[typeid(*this).name()];

    assert(dm_appinfo.get());
  }

  return dm_appinfo;
}

