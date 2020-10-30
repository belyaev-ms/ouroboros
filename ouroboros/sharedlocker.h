/**
 * @file   sharedlocker.h
 * The classes for interprocess locking data
 */

#ifndef OUROBOROS_SHAREDLOCKER_H
#define	OUROBOROS_SHAREDLOCKER_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use sharedlocker (OUROBOROS_BOOST_ENABLED)
#endif

#include <string>
#include <boost/thread/thread_time.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>

#include "ouroboros/global.h"
#include "ouroboros/sharedobject.h"
#include "ouroboros/sharedlock.h"

namespace ouroboros
{

/**
 * The lock based a lock which is placed in a shared memory
 */
template <typename Lock>
class base_shared_lock
{
    typedef Lock lock_type;
public:
    inline bool lock(); ///< set the exclusive lock
    inline bool lock(const size_t timeout); ///< set the exclusive lock with a timeout
    inline bool unlock(); ///< remove the exclusive lock
    inline bool lock_sharable(); ///< set the shared lock
    inline bool lock_sharable(const size_t timeout); ///< set the shared lock with a timeout
    inline bool unlock_sharable(); ///< remove the shared lock
private:
    lock_type m_lock;
};

/**
 * The lock based the interprocess_upgradable_mutex
 */
struct mutex_lock : public
    base_shared_lock<boost::interprocess::interprocess_upgradable_mutex> {};

/**
 * The lock based the shared_lock
 */
struct simple_mutex_lock : public base_shared_lock<shared_lock> {};

/**
 * The shared locker
 */
template <typename Lock>
struct shared_locker : public locker<Lock, shared_object>
{
    typedef Lock lock_type;
    shared_locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
        locker<Lock, shared_object>(name, scoped_count, sharable_count)
    {}
    shared_locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
        locker<Lock, shared_object>(lock, scoped_count, sharable_count)
    {}
};

//==============================================================================
//  base_shared_lock
//==============================================================================
/**
 * Set the exclusive lock
 * @return the result of the setting
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::lock()
{
    return lock(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::lock(const size_t timeout)
{
    return m_lock.timed_lock(boost::get_system_time() + boost::posix_time::millisec(timeout));
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::unlock()
{
    m_lock.unlock();
    return true;
}

/**
 * Set the shared lock
 * @return the result
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::lock_sharable()
{
    return lock_sharable(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::lock_sharable(const size_t timeout)
{
    return m_lock.timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(timeout));
}

/**
 * Remove the shared lock
 * @return the result
 */
template <typename Lock>
inline bool base_shared_lock<Lock>::unlock_sharable()
{
    m_lock.unlock_sharable();
    return true;
}

}   // namespace ouroboros

#endif	/* OUROBOROS_SHAREDLOCKER_H */

