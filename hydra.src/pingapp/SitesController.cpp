
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/SitesController.h>

#include <QDebug>

#include <wexus/TemplateInclude.h>    // lazy :)
#include <wexus/Registry.h>

#include <pingapp/PingApp.h>

using namespace wexus;
using namespace pingapp;

static wexus::RegisterController<pingapp::PingApp, SitesController> r1("sites");

static wexus::RegisterAction<SitesController, &SitesController::index> r2("index");
static wexus::RegisterAction<SitesController, &SitesController::create> r3("create");
static wexus::RegisterAction<SitesController, &SitesController::edit> r4("edit");
static wexus::RegisterAction<SitesController, &SitesController::destroy> r5("destroy");

//
//
// SitesController
//
//

SitesController::SitesController(void)
{ 
}

void SitesController::index(void)
{
  // need to put this someplace better!
  // atleast before the migration stuff too
  Host().activeClass()->createTable();
  Site().activeClass()->createTable();

  dm_site.all();

  indexHtml();
}

void SitesController::create(void)
{
  Site S;
  if (S.fromForm()) {
    // insert it into the DB
    S.create();

    setFlash["notice"] = "Site added.";
    redirectTo(pathTo(&SitesController::index));
  }

  createHtml();
}

void SitesController::edit(void)
{
  dm_site.find(params["id"]);

  Site S;
  if (S.fromForm()) {
    setFlash["notice"] = "Site updated.";
    redirectTo(pathTo(&SitesController::index));
  }

  editHtml();
}

void SitesController::destroy(void)
{
  dm_site.find(params["id"]);

  if (params["sure"] == 1) {
    dm_site.destroy();
    setFlash["notice"] = "Site deleted.";
    redirectTo(pathTo(&SitesController::index));
  } else
    destroyHtml();
}

