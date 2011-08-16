
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Controller.h>

#include <assert.h>

#include <QDebug>

#include <wexus/Registry.h>

using namespace wexus;

//
//
// ControllerContext
//
//


ControllerContext::ControllerContext(void)
  : dm_context(*Context::threadInstance()),
  params(dm_context.params),
  cookies(dm_context.cookies),
  session(dm_context.session),
  flash(dm_context.flash),
  setFlash(dm_context.setFlash),
  errors(dm_context.errors)
{
}

//
//
// Controller
//
//

Controller::Controller(void)
{
}

Controller::~Controller()
{
}

