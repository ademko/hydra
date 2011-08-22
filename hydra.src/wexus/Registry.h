
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_REGISTRY_H__
#define __INCLUDED_WEXUS_REGISTRY_H__

#include <typeinfo>

#include <wexus/TR1.h>

#include <wexus/MemberFunction.h>

#include <QString>
#include <QMap>

namespace wexus
{
  class Registry;

  template <class T> class RegisterApp;
  template <class APPTYPE, class T> class RegisterController;
  template <class CONTROLLERTYPE, void (CONTROLLERTYPE::*MFN)(void)> class RegisterAction;

  class Application;  // fwd
  class Controller; //fwd

  // convert this to return shared_ptr<>?
  typedef Application* (*ApplicationLoader)(void);
  template <class SUB> Application* ApplicationLoaderType(void) { return new SUB(); } // cant seem to nest this in ControllerInfo

  // convert this to return shared_ptr<>?
  typedef Controller* (*ControllerLoader)(void);
  template <class SUB> Controller* ControllerLoaderType(void) { return new SUB(); } // cant seem to nest this in ControllerInfo

  typedef void (*ActionFunc)(Controller *c);
};

/**
 * A type registry for applications, controllers and actions.
 * This is a singleton.
 *
 * @author Aleksander Demko
 */ 
class wexus::Registry
{
  public:
    class ControllerInfo;
    class AppInfo
    {
      public:
        typedef QMap<QString, std::shared_ptr<ControllerInfo>  > ControllerMap; // controllers by their name
      public:
        QString classtype; // the type string (from typeid)
        QString appname; // the application name

        ApplicationLoader loader;

        ControllerMap controllers;
    };
    class ActionInfo;
    class ControllerInfo
    {
      public:
        // maps actionames to actions
        typedef QMap<QString, std::shared_ptr<ActionInfo>  > ActionMap;
      public:
        QString classtype; // the type string (from typeid)
        QString name; // the controller name

        ControllerLoader loader;

        /// parent app
        std::shared_ptr<AppInfo> application;

        /// maps actionames to actions
        ActionMap actions;
    };
    class ActionInfo
    {
      public:
        MemberFunction mfn;
        QString actionname;

        /// parent controller
        std::shared_ptr<ControllerInfo> controller;

        ActionFunc func;
    };

  public:
    typedef QMap<QString, std::shared_ptr<AppInfo>  > AppTypeMap; // apps by the typeinfo.name()
    typedef QMap<QString, std::shared_ptr<AppInfo>  > AppNameMap; // apps by their name

    typedef QMap<QString, std::shared_ptr<ControllerInfo>  > ControllerTypeMap; // controllers by their typeinfo.name()

    typedef QList<std::shared_ptr<ActionInfo>  > ActionTypeList; // cant be a map cuz MemberFunction doesnt have operator<

    static AppTypeMap & appsByType(void) { return instance()->dm_apptype; }
    static AppNameMap & appsByName(void) { return instance()->dm_appname; }

    static ControllerTypeMap & controllersByType(void) { return instance()->dm_contype; }

    static ActionTypeList & actionList(void) { return instance()->dm_actionlist; }

    /// called by RegisterApp
    static std::shared_ptr<AppInfo> newAppInfo(const char *_appname, const char *_typename, ApplicationLoader loader);
    /// called by RegistrerController
    /// creates a app placeholder if needed
    static std::shared_ptr<AppInfo> linkAppInfo(const char *_apptype);

    /// called by RegistrerController
    static std::shared_ptr<ControllerInfo> newControllerInfo(const char *_cname, const char *_typename, const char *_apptype, ControllerLoader loader);
    /// called b y RegisterAction
    /// createa a controllerinfo placeholder if needed
    static std::shared_ptr<ControllerInfo> linkControllerInfo(const char *_ctype);

    /// called by RegisterAction
    static std::shared_ptr<ActionInfo> newActionInfo(wexus::MemberFunction mfn, const char *_actionname, const char *_ctype, ActionFunc func);

  protected:
    /// never returns null, makes it if need be
    static Registry * instance(void);

  protected:
    static Registry *dm_instance;

  protected:
    AppTypeMap dm_apptype;
    AppNameMap dm_appname;

    ControllerTypeMap dm_contype;

    ActionTypeList dm_actionlist;
};

template <class T> class wexus::RegisterApp
{
  public:
    RegisterApp(const char *appname);
};
template <class T> wexus::RegisterApp<T>::RegisterApp(const char *appname)
{
  /*std::shared_ptr<Registry::AppInfo> ai = */ Registry::newAppInfo(appname, typeid(T).name(), & ApplicationLoaderType<T> );
  // nothing more to fill out, .name is filled by new
}

template <class APPTYPE, class T> class wexus::RegisterController
{
  public:
    RegisterController(const char *appname);
};
template <class APPTYPE, class T> wexus::RegisterController<APPTYPE,T>::RegisterController(const char *appname)
{
  /*std::shared_ptr<Registry::ControllerInfo> ci =*/ Registry::newControllerInfo(appname, typeid(T).name(), typeid(APPTYPE).name(), & ControllerLoaderType<T> );
  // nothing more to fill out, .name is filled by new
}

template <class CONTROLLERTYPE, void (CONTROLLERTYPE::*MFN)(void)>
class wexus::RegisterAction
{
  private:
    template <class C, void (C::*LMETH)(void)>
      static void methodToFunc(Controller *c)
      {
        // bounce the function call to a method call
        C *here = dynamic_cast<C*>(c);
        assert(here);

        ((*here).*LMETH)();
      }
  public:
      RegisterAction(const char *actionname);;
};

template <class CONTROLLERTYPE, void (CONTROLLERTYPE::*MFN)(void)>
wexus::RegisterAction<CONTROLLERTYPE,MFN>::RegisterAction(const char *actionname)
{
  Registry::newActionInfo(MemberFunction(MFN), actionname,
      typeid(CONTROLLERTYPE).name(), methodToFunc<CONTROLLERTYPE,MFN>);
}

#endif

