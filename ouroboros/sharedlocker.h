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
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/lexical_cast.hpp>

#include "ouroboros/global.h"
#include "ouroboros/locker.h"
#include "ouroboros/object.h"
#include "ouroboros/sharedobject.h"
#include "ouroboros/filelock.h"
#include "ouroboros/sharedlock.h"

namespace ouroboros
{

/**
 * The lock based a mutex which is placed in a shared memory
 */
template <typename TMutex>
class base_mutex_lock
{
public:
    explicit inline base_mutex_lock(const std::string& name);

    inline bool lock(); ///< set the exclusive lock
    inline bool lock(const size_t timeout); ///< set the exclusive lock with a timeout
    inline bool unlock(); ///< remove the exclusive lock
    inline bool lock_sharable(); ///< set the shared lock
    inline bool lock_sharable(const size_t timeout); ///< set the shared lock with a timeout
    inline bool unlock_sharable(); ///< remove the shared lock
    inline const char* name() const; ///< get the name of the locker
private:
    enum lock_state
    {
        LS_SCOPED,
        LS_SHARABLE,
        LS_NONE
    };
private:
    typedef TMutex lock_type;
    object<lock_type, shared_object> m_lock;
    lock_state m_locked;
};

/**
 * The lock based the interprocess_upgradable_mutex
 */
struct mutex_lock : public base_mutex_lock<boost::interprocess::interprocess_upgradable_mutex>
{
    explicit inline mutex_lock(const std::string& name) :
        base_mutex_lock<boost::interprocess::interprocess_upgradable_mutex>(name)
    {}
};

/**
 * The lock based the shared_lock
 */
struct simple_mutex_lock : public base_mutex_lock<shared_lock>
{
    explicit inline simple_mutex_lock(const std::string& name) :
        base_mutex_lock<shared_lock>(name)
    {}
};

//==============================================================================
//  base_mutex_lock
//==============================================================================
/**
 * Constructor
 * @param name the name of the lock
 */
template <typename TMutex>
inline base_mutex_lock<TMutex>::base_mutex_lock(const std::string& name) :
    m_lock(make_object_name(name, "lock")),
    m_locked(LS_NONE)
{
}

/**
 * Set the exclusive lock
 * @return the result of the setting
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::lock()
{
    return lock(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::lock(const size_t timeout)
{
    assert(LS_NONE == m_locked);
    m_locked = m_lock->timed_lock(boost::get_system_time() + boost::posix_time::millisec(timeout)) ? LS_SCOPED : LS_NONE;
    return m_locked != LS_NONE;
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::unlock()
{
    assert(LS_SCOPED == m_locked);
    m_lock->unlock();
    m_locked = LS_NONE;
    return true;
}

/**
 * Set the shared lock
 * @return the result
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::lock_sharable()
{
    return lock_sharable(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::lock_sharable(const size_t timeout)
{
    assert(LS_NONE == m_locked);
    m_locked = m_lock->timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(timeout)) ? LS_SHARABLE : LS_NONE;
    return m_locked != LS_NONE;
}

/**
 * Remove the shared lock
 * @return the result
 */
template <typename TMutex>
inline bool base_mutex_lock<TMutex>::unlock_sharable()
{
    assert(LS_SHARABLE == m_locked);
    m_lock->unlock_sharable();
    m_locked = LS_NONE;
    return true;
}

/**
 * Get the name of the locker
 * @return the name of the locker
 */
template <typename TMutex>
inline const char* base_mutex_lock<TMutex>::name() const
{
    return m_lock.name();
}

}   // namespace ouroboros

#endif	/* OUROBOROS_SHAREDLOCKER_H */

