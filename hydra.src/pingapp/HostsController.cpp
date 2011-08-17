
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/HostsController.h>

#include <QDebug>

#include <wexus/TemplateInclude.h>    // lazy :)
#include <wexus/Registry.h>

#include <pingapp/PingApp.h>

#include <pingapp/SitesController.h>

using namespace wexus;
using namespace pingapp;

HostsController::HostsController(void)
{
}

void HostsController::index(void)
{
  indexHtml();
}

void HostsController::create(void)
{
  Site S;
  S.find(params["pid"]);

  Host H = S.host();
  if (H.fromForm()) {
    H.create();

    setFlash["notice"] = "Host added.";
    redirectTo(pathTo(&SitesController::index));
  }

  createHtml();
}

static wexus::RegisterController<PingApp, HostsController> r1("hosts");

static wexus::RegisterAction<HostsController, &HostsController::index> r2("index");
static wexus::RegisterAction<HostsController, &HostsController::create> r3("create");

