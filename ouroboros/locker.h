/**
 * @file   locker.h
 * The classes for locking data
 */

#ifndef OUROBOROS_LOCKER_H
#define	OUROBOROS_LOCKER_H

#include <assert.h>
#include "ouroboros/global.h"
#include "ouroboros/object.h"

namespace ouroboros
{

/**
 * The base locker
 */
class base_locker
{
public:
    inline base_locker(count_type& scoped_count, count_type& sharable_count);
    inline bool lock(); ///< set the exclusive lock
    inline bool lock(const size_t timeout); ///< set the exclusive lock with a timeout
    inline bool unlock(); ///< remove the exclusive lock
    inline bool lock_sharable(); ///< set the shared lock
    inline bool lock_sharable(const size_t timeout); ///< set the shared lock with a timeout
    inline bool unlock_sharable(); ///< remove the shared lock
    inline const char* name() const; ///< get the name of the locker
private:
    count_type& m_scoped_count;
    count_type& m_sharable_count;
};

/**
 * The stub lock (dummy)
 */
struct stub_lock {};

/**
 * The stub locker without any locking
 */
struct stub_locker : public base_locker
{
    typedef stub_lock lock_type;
    inline stub_locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
        base_locker(scoped_count, sharable_count)
    {}
    inline stub_locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
        base_locker(scoped_count, sharable_count)
    {}
};

///@todo may be must remove it
/**
 * The locker for internal locking
 */
struct internal_locker : public base_locker
{
    typedef stub_lock lock_type;
    inline internal_locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
        base_locker(scoped_count, sharable_count)
    {}
    inline internal_locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
        base_locker(scoped_count, sharable_count)
    {}
};

/**
 * The guard
 */
template <typename Locker>
class guard
{
public:
    typedef Locker locker_type;
    explicit guard(locker_type& locker) :
        m_locker(locker)
    {
        m_locker.lock();
    }
    ~guard()
    {
        m_locker.unlock();
    }
private:
    guard(guard&);
    guard& operator=(guard&);
private:
    locker_type& m_locker;
};

/**
 * The locker for recursive locking
 */
template <typename Lock, template <typename> class Interface>
class locker : public base_locker
{
public:
    typedef Lock lock_type;
    inline locker(const std::string& name, count_type& scoped_count, count_type& sharable_count);
    inline locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count);
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
    object<lock_type, Interface> m_lock;
    lock_state m_locked;
};

/**
 * The local locker
 */
template <typename Lock>
struct local_locker : public locker<Lock, local_object>
{
    typedef Lock lock_type;
    local_locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
        locker<Lock, local_object>(name, scoped_count, sharable_count)
    {}
    local_locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
        locker<Lock, local_object>(lock, scoped_count, sharable_count)
    {}
};

//==============================================================================
//  base_locker
//==============================================================================
/**
 * Constructor
 * @param scoped_count the counter for exclusive lock
 * @param sharable_count the counter for shared lock
 */
inline base_locker::base_locker(count_type& scoped_count, count_type& sharable_count) :
    m_scoped_count(scoped_count),
    m_sharable_count(sharable_count)
{
}

/**
 * Set the exclusive lock
 * @return the result of the setting
 */
inline bool base_locker::lock()
{
    return 0 == m_scoped_count++;
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
inline bool base_locker::lock(const size_t timeout)
{
    return lock();
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
inline bool base_locker::unlock()
{
    assert(m_scoped_count > 0);
    return 0 == --m_scoped_count;
}

/**
 * Set the shared lock
 * @return the result of the setting
 */
inline bool base_locker::lock_sharable()
{
    return 0 == m_scoped_count && 0 == m_sharable_count++;
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result
 */
inline bool base_locker::lock_sharable(const size_t timeout)
{
    return lock_sharable();
}

/**
 * Remove the shared lock
 * @return the result of the removing
 */
inline bool base_locker::unlock_sharable()
{
    assert(m_scoped_count > 0 || m_sharable_count > 0);
    return 0 == m_scoped_count && 0 == --m_sharable_count;
}

/**
 * Get the name of the locker
 * @return the name of the locker
 */
inline const char* base_locker::name() const
{
    return NULL;
}

//==============================================================================
//  locker
//==============================================================================
/**
 * Constructor
 * @param name the name of the locker
 * @param scoped_count the counter for exclusive lock
 * @param sharable_count the counter for shared lock
 */
template <typename Locker, template <typename> class Interface>
inline locker<Locker, Interface>::locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
    base_locker(scoped_count, sharable_count),
    m_lock(make_object_name(name, "lock")),
    m_locked(LS_NONE)
{
}

/**
 * Constructor
 * @param lock the lock
 * @param scoped_count the counter for exclusive lock
 * @param sharable_count the counter for shared lock
 */
template <typename Locker, template <typename> class Interface>
inline locker<Locker, Interface>::locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
    base_locker(scoped_count, sharable_count),
    m_lock(object_adoption_tag(), lock),
    m_locked(LS_NONE)
{
}

/**
 * Set the exclusive lock
 * @return the result of the setting
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::lock()
{
    return lock(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::lock(const size_t timeout)
{
    if (base_locker::lock())
    {
        assert(LS_NONE == m_locked);
        if (!m_lock->lock(timeout))
        {
            base_locker::unlock();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error installing the exclusive lock "
                    << (NULL == name() ? "" : name()));
        }
        m_locked = LS_SCOPED;
        return true;
    }
    return false;
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::unlock()
{
    if (base_locker::unlock())
    {
        assert(LS_SCOPED == m_locked);
        if (!m_lock->unlock())
        {
            base_locker::lock();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error removing the exclusive lock "
                    << (NULL == name() ? "" : name()));
        }
        m_locked = LS_NONE;
        return true;
    }
    return false;
}

/**
 * Set the shared lock
 * @return the result of the setting
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::lock_sharable()
{
    return lock_sharable(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::lock_sharable(const size_t timeout)
{
    if (base_locker::lock_sharable())
    {
        assert(LS_NONE == m_locked);
        if (!m_lock->lock_sharable(timeout))
        {
            base_locker::unlock_sharable();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error installing the shared lock "
                    << (NULL == name() ? "" : name()));
        }
        m_locked = LS_SHARABLE;
        return true;
    }
    return false;
}

/**
 * Remove the shared lock
 * @return the result of the removing
 */
template <typename Locker, template <typename> class Interface>
inline bool locker<Locker, Interface>::unlock_sharable()
{
    if (base_locker::unlock_sharable())
    {
        assert(LS_SHARABLE == m_locked);
        if (!m_lock->unlock_sharable())
        {
            base_locker::lock_sharable();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error removing the shared lock "
                    << (NULL == name() ? "" : name()));
        }
        m_locked = LS_NONE;
        return true;
    }
    return false;
}

/**
 * Get the name of the locker
 * @return the name of the locker
 */
template <typename Locker, template <typename> class Interface>
inline const char* locker<Locker, Interface>::name() const
{
    return m_lock.name();
}

}   // namespace ouroboros

#endif	/* OUROBOROS_LOCKER_H */

