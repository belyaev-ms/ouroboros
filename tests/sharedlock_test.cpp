#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sharedlock_test
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include "ouroboros/sharedlock.h"
#include "ouroboros/sharedorderedlock.h"

typedef boost::mpl::list<ouroboros::shared_lock, ouroboros::shared_ordered_lock> lock_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_lock_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE_THROW(locker.unlock(), ouroboros::assert_error);
    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE(locker.try_lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE(locker.timed_lock(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE_THROW(locker.unlock(), ouroboros::assert_error);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_lock_sharable_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE_THROW(locker.unlock_sharable(), ouroboros::assert_error);
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE(locker.try_lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE(locker.timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE_THROW(locker.unlock_sharable(), ouroboros::assert_error);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking_lock_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());

    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE(!locker.try_lock());
    BOOST_REQUIRE(!locker.timed_lock(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE(!locker.try_lock_sharable());
    BOOST_REQUIRE(!locker.timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE_NO_THROW(locker.unlock());

    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking_lock_sharable_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());

    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE(!locker.try_lock());
    BOOST_REQUIRE(!locker.timed_lock(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE(locker.try_lock_sharable());
    BOOST_REQUIRE(locker.timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(1)));
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());

    BOOST_REQUIRE_NO_THROW(locker.lock());
    BOOST_REQUIRE_NO_THROW(locker.unlock());
    BOOST_REQUIRE_NO_THROW(locker.lock_sharable());
    BOOST_REQUIRE_NO_THROW(locker.unlock_sharable());
}

size_t time_us()
{
#if __APPLE__
    return mach_absolute_time();
#else
    struct timespec res = {0};
	clock_gettime(CLOCK_MONOTONIC, &res);
    return res.tv_sec * 1000000 + res.tv_nsec / 1000;
#endif
}

void delay_ms(size_t to)
{
    size_t t = time_us();
    while (time_us() - t < to * 1000)
    {
        usleep(100);
    }
}

template <typename Lock>
class base_thread_test
{
public:
    typedef Lock lock_type;
    typedef std::vector<size_t> event_list;
    typedef std::function<void()> notification_type;
    explicit base_thread_test(lock_type& locker) :
        m_locker(locker)
    {
    }
    void run()
    {
        if (!m_pthread)
        {
            m_pthread = std::unique_ptr<std::thread>(new std::thread(&base_thread_test::execute, std::ref(*this)));
        }
    }
    void join()
    {
        if (m_pthread)
        {
            m_pthread->join();
            m_pthread.reset();
        }
    }
    const event_list& get_events() const
    {
        return m_events;
    }
    void set_notification(notification_type notification)
    {
        m_notification = notification;
    }
protected:
    void push_event()
    {
        m_events.push_back(time_us());
    }
    void notify()
    {
        if (m_notification)
        {
            m_notification();
        }
    }
private:
    virtual void do_execute() = 0;
    void execute()
    {
        do_execute();
    }
protected:
    lock_type& m_locker;
private:
    std::unique_ptr<std::thread> m_pthread;
    notification_type m_notification;
    event_list m_events;
};

template <typename Lock>
class thread_lock_test : public base_thread_test<Lock>
{
    typedef base_thread_test<Lock> base_class;
public:
    typedef Lock lock_type;
    explicit thread_lock_test(lock_type& locker) :
        base_thread_test<Lock>(locker)
    {}
private:
    void do_execute()
    {
        base_class::m_locker.lock();
        base_class::push_event();
        base_class::notify();
        delay_ms(10);
        base_class::push_event();
        base_class::m_locker.unlock();
    }
};

template <typename Lock>
class thread_lock_sharable_test : public base_thread_test<Lock>
{
    typedef base_thread_test<Lock> base_class;
public:
    typedef Lock lock_type;
    explicit thread_lock_sharable_test(lock_type& locker) :
        base_thread_test<Lock>(locker)
    {}
private:
    void do_execute()
    {
        base_class::m_locker.lock_sharable();
        base_class::push_event();
        base_class::notify();
        delay_ms(10);
        base_class::push_event();
        base_class::m_locker.unlock_sharable();
    }
};

typedef std::multimap<size_t, size_t> event_map;
template <typename Lock>
void put_events(event_map& events, size_t id, const base_thread_test<Lock>& th)
{
    for (auto tv : th.get_events())
    {
        events.insert({ tv, id });
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(threads_lock_test, lock_type, lock_types)
{
    std::atomic<bool> flag(true);
    lock_type locker;
    thread_lock_test<lock_type> th_lock1(locker);
    thread_lock_test<lock_type> th_lock2(locker);
    thread_lock_test<lock_type> th_lock3(locker);
    thread_lock_sharable_test<lock_type> th_lock_sharable1(locker);
    thread_lock_sharable_test<lock_type> th_lock_sharable2(locker);
    thread_lock_sharable_test<lock_type> th_lock_sharable3(locker);
    thread_lock_sharable_test<lock_type> th_lock_sharable4(locker);
    th_lock1.set_notification([&]{
        th_lock2.run();
    });
    th_lock2.set_notification([&]{
        th_lock_sharable1.run();
    });
    th_lock_sharable1.set_notification([&]{
        th_lock_sharable2.run();
    });
    th_lock_sharable2.set_notification([&]{
        th_lock3.run();
        delay_ms(5);
        th_lock_sharable3.run();
        th_lock_sharable4.run();
    });
    th_lock_sharable4.set_notification([&]{
        flag = false;
    });
    th_lock1.run();
    while (flag)
    {
        delay_ms(1);
    }
    th_lock1.join();
    th_lock2.join();
    th_lock3.join();
    th_lock_sharable1.join();
    th_lock_sharable2.join();
    th_lock_sharable3.join();
    th_lock_sharable4.join();
    event_map events;
    put_events(events, 1, th_lock1);
    put_events(events, 2, th_lock2);
    put_events(events, 3, th_lock3);
    put_events(events, 4, th_lock_sharable1);
    put_events(events, 5, th_lock_sharable2);
    put_events(events, 6, th_lock_sharable3);
    put_events(events, 7, th_lock_sharable4);
    std::vector<std::vector<size_t>> result =
    {
        { 1 }, { 1 }, { 2 }, { 2 }, { 4, 5 }, { 4, 5 }, { 4, 5 }, { 4, 5},
        { 3 }, { 3 }, { 6, 7 }, { 6, 7 }, { 6, 7 }, { 6, 7 }
    };
    size_t t0 = events.begin() != events.end() ? events.begin()->first : 0;
    for (auto ev : events)
    {
        size_t tv = ev.first - t0;
        BOOST_TEST_MESSAGE(ev.second << "\t: " << tv);
    }
    BOOST_REQUIRE_EQUAL(result.size(), events.size());
    auto rit = result.begin();
    auto eit = events.begin();
    for (; rit != result.end() && eit != events.end(); ++rit, ++eit)
    {
        BOOST_REQUIRE(std::find(rit->begin(), rit->end(), eit->second) != rit->end());
    }
}