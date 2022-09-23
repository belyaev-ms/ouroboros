/**
 * @file    sharedlock.h
 */

#ifndef OUROBOROS_SHARED_LOCK_H
#define OUROBOROS_SHARED_LOCK_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use shared_lock (OUROBOROS_BOOST_ENABLED)
#endif

#include <assert.h>
#include <limits.h>
#include <boost/thread/thread_time.hpp>
#ifdef OUROBOROS_SPINLOCK_ENABLED
#include <boost/smart_ptr/detail/spinlock.hpp>
#else
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#endif

namespace ouroboros
{

/**
 * Simple class for RW lock which is placed in a shared memory
 */
class shared_lock
{
public:
    inline shared_lock();
    inline void lock();
    inline bool timed_lock(const boost::posix_time::ptime& abs_time);
    inline bool try_lock();
    inline void unlock();
    inline void lock_sharable();
    inline bool timed_lock_sharable(const boost::posix_time::ptime& abs_time);
    inline bool try_lock_sharable();
    inline void unlock_sharable();
protected:
    enum lock_state
    {
        LS_SCOPED,
        LS_SHARABLE,
        LS_NONE
    };
    inline lock_state try_book_lock();
    inline unsigned int count_sharable();
    inline void delay() const;
private:
    shared_lock(const shared_lock& );
    shared_lock& operator=(const shared_lock& );
private:
#ifdef OUROBOROS_SPINLOCK_ENABLED
    typedef boost::detail::spinlock lock_type;
    typedef boost::detail::spinlock::scoped_lock guard_type;
#else
    typedef boost::interprocess::interprocess_mutex lock_type;
    typedef boost::interprocess::scoped_lock<lock_type> guard_type;
#endif
    lock_type m_lock;
    unsigned int m_scoped;
    unsigned int m_sharable;
};

/**
 * Constructor
 */
inline shared_lock::shared_lock() :
    m_scoped(0),
    m_sharable(0)
{
#ifdef OUROBOROS_SPINLOCK_ENABLED
    m_lock = BOOST_DETAIL_SPINLOCK_INIT;
#endif
}

/**
 * Delay
 */
inline void shared_lock::delay() const
{
    usleep(10);
}

/**
 * Try to book the exclusive lock
 * @return the type of the current lock
 */
inline shared_lock::lock_state shared_lock::try_book_lock()
{
    guard_type guard(m_lock);
    if (0 == m_scoped)
    {
        m_scoped = 1;
        return 0 == m_sharable ? LS_NONE : LS_SHARABLE;
    }
    return LS_SCOPED;
}

/**
 * Get the count of the sharable lock
 * @return the count of the sharable lock
 */
inline unsigned int shared_lock::count_sharable()
{
    guard_type guard(m_lock);
    return m_sharable;
}

/**
 * Try to set the exclusive lock
 * @return the result of the setting
 */
inline bool shared_lock::try_lock()
{
    guard_type guard(m_lock);
    if (0 == m_scoped && 0 == m_sharable)
    {
        m_scoped = 1;
        return true;
    }
    return false;
}

/**
 * Set the exclusive lock
 */
inline void shared_lock::lock()
{
    while (1)
    {
        switch (try_book_lock())
        {
            case LS_NONE:
                return;
            case LS_SCOPED:
                break;
            case LS_SHARABLE:
                while (count_sharable() > 0)
                {
                    delay();
                }
                return;
        }
        delay();
    }
}

/**
 * Try to set the exclusive lock until the time comes
 * @param abs_time the time
 * @return the result of the setting
 */
inline bool shared_lock::timed_lock(const boost::posix_time::ptime& abs_time)
{
    while (boost::get_system_time() < abs_time)
    {
        switch (try_book_lock())
        {
            case LS_NONE:
                return true;
            case LS_SCOPED:
                break;
            case LS_SHARABLE:
                while (boost::get_system_time() < abs_time)
                {
                    if (0 == count_sharable())
                    {
                        return true;
                    }
                    delay();
                }
                unlock();
                return false;
        }
        delay();
    }
    return false;
}

/**
 * Remove the exclusive lock
 */
inline void shared_lock::unlock()
{
    guard_type guard(m_lock);
    assert(1 == m_scoped);
    m_scoped = 0;
}

/**
 * Try to set the sharable lock
 * @return the result of the setting
 */
inline bool shared_lock::try_lock_sharable()
{
    guard_type guard(m_lock);
    if (0 == m_scoped && m_sharable < UINT_MAX)
    {
        ++m_sharable;
        return true;
    }
    return false;
}

/**
 * Set the sharable lock
 */
inline void shared_lock::lock_sharable()
{
    while (1)
    {
        if (try_lock_sharable())
        {
            return;
        }
        delay();
    }
}

/**
 * Try to set the sharable lock until the time comes
 * @param abs_time the time
 * @return the result of the setting
 */
inline bool shared_lock::timed_lock_sharable(const boost::posix_time::ptime& abs_time)
{
    while (boost::get_system_time() < abs_time)
    {
        if (try_lock_sharable())
        {
            return true;
        }
        delay();
    }
    return false;
}

/**
 * Remove the sharable lock
 */
inline void shared_lock::unlock_sharable()
{
    guard_type guard(m_lock);
    assert(m_sharable > 0);
    --m_sharable;
}

} // namespace ouroboros

#endif /* OUROBOROS_SHARED_LOCK_H */

