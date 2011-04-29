
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <wexus/Context.h>

#include <assert.h>

#include <QThreadStorage>

using namespace wexus;

typedef Context* ContextPtr;

static QThreadStorage<ContextPtr *> Storage;

QTextStream & wexus::output(void)
{
  Context *c = Context::instance();

  assert(c);

  return c->reply().output();
}

Context::Context(wexus::HTTPRequest &req, wexus::HTTPReply &reply)
  : dm_req(req), dm_reply(reply)
{
  // we need to store a dynamically allocated ptr-to-ptr
  // becase QThreadStorage insists on being able to call delete
  Storage.setLocalData(new ContextPtr(this));
}

Context * Context::instance(void)
{
  return *(Storage.localData());
}

Context::~Context()
{
  Storage.setLocalData(0);
}

