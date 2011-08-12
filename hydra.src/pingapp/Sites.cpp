
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/Sites.h>

#include <QDebug>

#include <wexus/TemplateInclude.h>    // lazy :)
#include <wexus/Registry.h>

#include <pingapp/App.h>

using namespace wexus;
using namespace pingapp;

static wexus::RegisterController<pingapp::App, Sites> r1("sites");

static wexus::RegisterAction<Sites, &Sites::index> r2("index");
static wexus::RegisterAction<Sites, &Sites::create> r3("create");
static wexus::RegisterAction<Sites, &Sites::edit> r4("edit");
static wexus::RegisterAction<Sites, &Sites::destroy> r5("destroy");

//
//
// Sites
//
//

Sites::Sites(void)
{ 
}

void Sites::index(void)
{
  // need to put this someplace better!
  // atleast before the migration stuff too
  Host().activeClass()->createTable();
  Site().activeClass()->createTable();

  dm_site.all();

  indexHtml();
}

void Sites::create(void)
{
  Site S;
  if (S.fromForm()) {
    // insert it into the DB
    S.create();

    setFlash["notice"] = "Site added.";
    redirectTo(&Sites::index);
  }

  createHtml();
}

void Sites::edit(void)
{
  dm_site.find(params["id"]);

  Site S;
  if (S.fromForm()) {
    setFlash["notice"] = "Site updated.";
    redirectTo(&Sites::index);
  }

  editHtml();
}

void Sites::destroy(void)
{
  dm_site.find(params["id"]);

  if (params["sure"] == 1) {
    dm_site.destroy();
    setFlash["notice"] = "Site deleted.";
    redirectTo(&Sites::index);
  } else
    destroyHtml();
}

