
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Registry.h>

#include <QDebug>

using namespace wexus;

Registry * Registry::dm_instance;

Registry * Registry::instance(void)
{
  if (!dm_instance)
    dm_instance = new Registry;

  return dm_instance;
}

std::shared_ptr<Registry::AppInfo> Registry::newAppInfo(const char *_appname, const char *_typename, ApplicationLoader loader)
{
  std::shared_ptr<AppInfo> & ai = Registry::appsByType()[_typename];

  QString appname(_appname);

  if (!ai.get())
    ai.reset(new AppInfo);

  // sometimes, it can exist in the type map but not in the name map
  if (!Registry::appsByName().contains(appname))
    Registry::appsByName()[appname] = ai;

  ai->classtype = _typename;
  ai->appname = appname;
  ai->loader = loader;

  return ai;
}

std::shared_ptr<Registry::AppInfo> Registry::linkAppInfo(const char *_apptype)
{
  std::shared_ptr<AppInfo> &r = Registry::appsByType()[_apptype];

  if (!r.get())
    r.reset(new AppInfo);

  return r;
}

std::shared_ptr<Registry::ControllerInfo> Registry::newControllerInfo(const char *_cname, const char *_typename, const char *_apptype, ControllerLoader loader)
{
  std::shared_ptr<ControllerInfo> & ci = Registry::controllersByType()[_typename];

  if (!ci)
    ci.reset(new ControllerInfo);

  ci->classtype = _typename;
  ci->name = _cname;
  ci->loader = loader;

  // add this controller to the AppInfo
  ci->application = Registry::linkAppInfo(_apptype);
  ci->application->controllers[_cname] = ci;

  // dont touch actions, as it may be set already

  return ci;
}

std::shared_ptr<Registry::ControllerInfo> Registry::linkControllerInfo(const char *_ctype)
{
  std::shared_ptr<ControllerInfo> &c = Registry::controllersByType()[_ctype];

  if (!c.get())
    c.reset(new ControllerInfo);

  return c;
}

std::shared_ptr<Registry::ActionInfo> Registry::newActionInfo(wexus::MemberFunction mfn, const char *_actionname, const char *_ctype, ActionFunc func)
{
  std::shared_ptr<ActionInfo> info(new ActionInfo);

  info->mfn = mfn;
  info->actionname = _actionname;
  info->func = func;

  info->controller = linkControllerInfo(_ctype);
  info->controller->actions[_actionname] = info;

  Registry::actionList().push_back(info);

  return info;
}

