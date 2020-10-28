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
 * The locker based a lock which is placed in a shared memory
 */
template <typename Lock>
class base_shared_locker
{
public:
    typedef Lock lock_type;
    explicit inline base_shared_locker(const std::string& name);
    explicit inline base_shared_locker(lock_type& lock);

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
    object<lock_type, shared_object> m_lock;
    lock_state m_locked;
};

/**
 * The locker based the interprocess_upgradable_mutex
 */
class mutex_locker : public base_shared_locker<boost::interprocess::interprocess_upgradable_mutex>
{
    typedef base_shared_locker<boost::interprocess::interprocess_upgradable_mutex> base_class;
public:
    typedef base_class::lock_type lock_type;
    explicit inline mutex_locker(const std::string& name) :
        base_class(name)
    {}
    explicit inline mutex_locker(lock_type& lock) :
        base_class(lock)
    {}
};

/**
 * The lock based the shared_lock
 */
class simple_mutex_locker : public base_shared_locker<shared_lock>
{
    typedef base_shared_locker<shared_lock> base_class;
public:
    typedef base_class::lock_type lock_type;
    explicit inline simple_mutex_locker(const std::string& name) :
        base_shared_locker<shared_lock>(name)
    {}
    explicit inline simple_mutex_locker(lock_type& lock) :
        base_shared_locker<shared_lock>(lock)
    {}
};

//==============================================================================
//  base_shared_locker
//==============================================================================
/**
 * Constructor
 * @param name the name of the locker
 */
template <typename Lock>
inline base_shared_locker<Lock>::base_shared_locker(const std::string& name) :
    m_lock(make_object_name(name, "lock")),
    m_locked(LS_NONE)
{
}

/**
 * Constructor
 * @param lock the lock of the locker
 */
template <typename Lock>
inline base_shared_locker<Lock>::base_shared_locker(lock_type& lock) :
    m_lock(object_adoption_tag(), lock),
    m_locked(LS_NONE)
{
}

/**
 * Set the exclusive lock
 * @return the result of the setting
 */
template <typename Lock>
inline bool base_shared_locker<Lock>::lock()
{
    return lock(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Lock>
inline bool base_shared_locker<Lock>::lock(const size_t timeout)
{
    assert(LS_NONE == m_locked);
    m_locked = m_lock->timed_lock(boost::get_system_time() + boost::posix_time::millisec(timeout)) ? LS_SCOPED : LS_NONE;
    return m_locked != LS_NONE;
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
template <typename Lock>
inline bool base_shared_locker<Lock>::unlock()
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
template <typename Lock>
inline bool base_shared_locker<Lock>::lock_sharable()
{
    return lock_sharable(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result
 */
template <typename Lock>
inline bool base_shared_locker<Lock>::lock_sharable(const size_t timeout)
{
    assert(LS_NONE == m_locked);
    m_locked = m_lock->timed_lock_sharable(boost::get_system_time() + boost::posix_time::millisec(timeout)) ? LS_SHARABLE : LS_NONE;
    return m_locked != LS_NONE;
}

/**
 * Remove the shared lock
 * @return the result
 */
template <typename Lock>
inline bool base_shared_locker<Lock>::unlock_sharable()
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
template <typename Lock>
inline const char* base_shared_locker<Lock>::name() const
{
    return m_lock.name();
}

}   // namespace ouroboros

#endif	/* OUROBOROS_SHAREDLOCKER_H */

