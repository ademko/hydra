
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <pingapp/Hosts.h>

#include <QDebug>

#include <wexus/TemplateInclude.h>    // lazy :)
#include <wexus/Registry.h>

#include <pingapp/App.h>

using namespace wexus;
using namespace pingapp;

Hosts::Hosts(void)
{
  registerAction<Hosts, &Hosts::index>("index");
}

void Hosts::index(void)
{
  indexHtml();
}

static wexus::RegisterController<App, Hosts> r1("hosts");
static wexus::RegisterAction<Hosts, &Hosts::index> r2("index");

