/*
 * File: EventLoop.cpp
 * -------------------
 * @author: Fu Wei
 *
 */

#include "network/EventLoop.hpp"
#include "network/Poller.hpp"
#include "network/Channel.hpp"
#include "core/TimerQueue.hpp"
#include "system/unixUtility.hpp"

#include <cassert> // for assert macro

#include "spdlog/spdlog.h"

const int POLLTIME = 10000; // millisecond

thread_local EventLoop* loopInThisThread = nullptr;

EventLoop::EventLoop() :
    _looping(false),
    _quit(false),
    _processingFunctors(false),
    _threadId(gettid()),
    _poller(new Poller(this)),
    _timerQueue(new TimerQueue(this)),
    _wakenfd(Eventfd()),
    _wakenChannel(new Channel(this, _wakenfd))
{
    spdlog::info("A new eventLoop {} created in thread {}", (void*)this, _threadId);
    if (loopInThisThread)
    {
        spdlog::critical("an eventLoop {} exists in this thread {}",
                         (void*)loopInThisThread,
                         _threadId);
    }
    else
    {
        loopInThisThread = this;
    }
    // Register the channel of eventfd to this EventLoop for wakening
    _wakenChannel->setReadCallback(std::bind(&EventLoop::readFd, this));
    _wakenChannel->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!_looping);

    loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!_looping);

    assertInLoopThread();
    _looping = true;
    _quit = false;

    while (!_quit)
    {
        _activeChannels.clear();
        _pollReturnTime = _poller->poll(POLLTIME, &_activeChannels);
        for (auto ich = _activeChannels.cbegin(); ich < _activeChannels.cend(); ++ich)
        {
            (*ich)->handleEvent(_pollReturnTime);
        }
        // process the remaining functors
        processFunctors();
    }

    spdlog::info("EventLoop {} stop looping", (void*)this);
    _looping = false;
}

void EventLoop::quit()
{
    _quit = true;
    if (!isInLoopThread())
    {
        waken();
    }
}

void EventLoop::abortNotInLoopThread()
{
    spdlog::critical("EventLoop {} was created in thread {}, current thread is {}",
                     (void*)this,
                     _threadId,
                     gettid());
    _exit(-1);
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);

    assertInLoopThread();
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    _poller->removeChannel(channel);
}

EventLoop* EventLoop::getCurrentEventLoop()
{
    return loopInThisThread;
}

void EventLoop::runAt(Timestamp time, const EventLoop::TimerCallback& cb)
{
    _timerQueue->addTimer(cb, time, 0.0);
}

void EventLoop::runAfter(double delay, const EventLoop::TimerCallback& cb)
{
    runAt(Timestamp::now() + delay, cb);
}

void EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    _timerQueue->addTimer(cb, Timestamp::now() + interval, interval);
}

void EventLoop::waken() const
{
    uint64_t byte = 0x1;
    ssize_t n = write(_wakenfd, &byte, sizeof byte);
    if (n != sizeof byte)
    {
        unix_error("write eventfd error");
    }
}

void EventLoop::readFd() const
{
    uint64_t byte;
    ssize_t n = ::read(_wakenfd, &byte, sizeof byte);
    if (n != sizeof byte)
    {
        unix_error("read eventfd error");
    }
}

void EventLoop::processFunctors()
{
    _processingFunctors = true;
    std::vector<Functor> functors;

    {
        std::scoped_lock<std::mutex> lock(_mutex);
        functors.swap(_unprocessedFunctors);
    }

    for (auto& functor : functors)
    {
        functor();
    }
    _processingFunctors = false;
}
void EventLoop::runInLoopThread(const EventLoop::Functor& functor)
{
    if (isInLoopThread())
    {
        functor();
    }
    else
    {
        addToLoopThread(functor);
    }
}

void EventLoop::addToLoopThread(const EventLoop::Functor& functor)
{
    {
        std::scoped_lock<std::mutex> lock(_mutex);
        _unprocessedFunctors.push_back(functor);
    }

    if (!isInLoopThread() || _processingFunctors)
    {
        waken();
    }
}
Timestamp EventLoop::pollReturnTime() const
{
    return _pollReturnTime;
}
