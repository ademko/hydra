
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Registry.h>

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
  std::shared_ptr<AppInfo> ai;
  QString appname(_appname), tname(_typename);

  if (Registry::appsByType().contains(tname))
    ai = Registry::appsByType()[tname];
  else
    ai.reset(new AppInfo);

  // sometimes, it can exist in the type map but not in the name map
  if (!Registry::appsByName().contains(appname))
    Registry::appsByName()[appname] = ai;

  ai->type = tname;
  ai->name = appname;
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
  std::shared_ptr<ControllerInfo> ci;
  QString cname(_cname), tname(_typename);

  if (Registry::controllersByType().contains(tname))
    ci = Registry::controllersByType()[tname];
  else
    ci.reset(new ControllerInfo);

  // sometimes, it can exist in the type map but not in the name map
  if (!Registry::controllersByName().contains(cname))
    Registry::controllersByName()[cname] = ci;

  ci->type = tname;
  ci->name = cname;
  ci->loader = loader;
  ci->app = Registry::linkAppInfo(_apptype);

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
  info->controller = linkControllerInfo(_ctype);
  info->func = func;

  Registry::actionList().push_back(info);

  return info;
}

