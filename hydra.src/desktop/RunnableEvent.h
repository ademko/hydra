
/*
 * Copyright (c) 2010 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_HYDRA_DESKTOP_RUNNABLEEVENT_H__
#define __INCLUDED_HYDRA_DESKTOP_RUNNABLEEVENT_H__

#include <QEvent>
#include <QObject>
#include <QRunnable>

#include <hydra/TR1.h>

namespace desktop {
class RunnableEventProcessor;
class RunnableEvent;
class RunnableEventFunction;
} // namespace desktop

// in the future, make this also a decendant of QRunnable
// (or has a QRunnable member that it calls?)

class desktop::RunnableEventProcessor : public QObject {
  public:
    RunnableEventProcessor(void);
    virtual ~RunnableEventProcessor();

    static RunnableEventProcessor *instance(void) { return dm_instance; }

  protected:
    virtual void customEvent(QEvent *event);

  private:
    static RunnableEventProcessor *dm_instance;
};

/**
 * This is the base class of events that have run methods.
 * The run method will be run when the event is processed.
 *
 * The receiver (a global instance RunnableEventProcessor)
 * of these events  has a customEvent() implementation that calls
 * RunnableEvent::customEvent.
 *
 * @author Aleksander Demko
 */
class desktop::RunnableEvent : public QEvent {
  public:
    RunnableEvent(void);

    /// the run method that will be called by the customEvent
    /// handler
    virtual void run(void) = 0;

    /**
     * Gets the next priority value in an always incrementing numeric sequence.
     * Not mutex protected, call only in the main thread.
     *
     * @author Aleksander Demko
     */
    static int nextPriority(void);
};

/**
 * An object who's run() method will call the given function.
 *
 * This class can be used where QRunnable's are needed
 * or QEvent's (via the RunnableEvent base).
 *
 * @author Aleksander Demko
 */
class desktop::RunnableEventFunction : public RunnableEvent, public QRunnable {
  public:
    RunnableEventFunction(std::function<void()> f);

    virtual void run(void);

    /**
     * Queues the function, via a new RunnableEventFunction to the main
     * gui thread via QCoreApplication::postEvent
     *
     * @author Aleksander Demko
     */
    static void enqueueMain(std::function<void()> f);

    /**
     * Queues a function to the background worker thread pool.
     *
     * @author Aleksander Demko
     */
    static void enqueueWorker(std::function<void()> f, int priority = 0);

  private:
    std::function<void()> dm_f;
};

#endif
