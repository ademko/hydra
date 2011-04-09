
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <desktop/RunnableEvent.h>

#include <assert.h>

#include <QCoreApplication>
#include <QtConcurrentRun>

using namespace desktop;

//
//
// RunnableEventProcessor
//
//

RunnableEventProcessor * RunnableEventProcessor::dm_instance;

RunnableEventProcessor::RunnableEventProcessor(void)
{
  assert(dm_instance == 0);
  dm_instance = this;
}

RunnableEventProcessor::~RunnableEventProcessor()
{
  assert(dm_instance == this);
  dm_instance = 0;
}

void RunnableEventProcessor::customEvent(QEvent *event)
{
  RunnableEvent *e = dynamic_cast<RunnableEvent*>(event);

  if (e)
    e->run();
}

//
//
// RunnableEvent
//
//

RunnableEvent::RunnableEvent(void)
  : QEvent(QEvent::User)
{
}

int RunnableEvent::nextPriority(void)
{
  static int prio = 1;

  return prio++;
}

//
//
// RunnableEventFunction
//
//

RunnableEventFunction::RunnableEventFunction(std::function<void()> f)
  : dm_f(f)
{
}

void RunnableEventFunction::run(void)
{
  dm_f();
}

void RunnableEventFunction::enqueueMain(std::function<void()> f)
{
  QCoreApplication::postEvent(RunnableEventProcessor::instance(), new RunnableEventFunction(f));
}

void RunnableEventFunction::enqueueWorker(std::function<void()> f, int priority)
{
  //QtConcurrent::run(f);
  assert(QThreadPool::globalInstance());
  QThreadPool::globalInstance()->start(new RunnableEventFunction(f), priority);
}

