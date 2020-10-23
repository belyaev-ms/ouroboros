/**
 * @file   locker.h
 * The classes for locking data
 */

#ifndef OUROBOROS_LOCKER_H
#define	OUROBOROS_LOCKER_H

#include <assert.h>
#include "ouroboros/global.h"

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
template <typename Locker>
struct locker : public Locker, private base_locker
{
    typedef typename Locker::lock_type lock_type;
    inline locker(const std::string& name, count_type& scoped_count, count_type& sharable_count);
    inline locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count);
    inline bool lock(); ///< set the exclusive lock
    inline bool lock(const size_t timeout); ///< set the exclusive lock with a timeout
    inline bool unlock(); ///< remove the exclusive lock
    inline bool lock_sharable(); ///< set the shared lock
    inline bool lock_sharable(const size_t timeout); ///< set the shared lock with a timeout
    inline bool unlock_sharable(); ///< remove the shared lock
    inline const char* name() const; ///< get the name of the locker
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
template <typename Locker>
inline locker<Locker>::locker(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
    Locker(name),
    base_locker(scoped_count, sharable_count)
{
}

/**
 * Constructor
 * @param lock the lock
 * @param scoped_count the counter for exclusive lock
 * @param sharable_count the counter for shared lock
 */
template <typename Locker>
inline locker<Locker>::locker(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
    Locker(lock),
    base_locker(scoped_count, sharable_count)
{
}

/**
 * Set the exclusive lock
 * @return the result of the setting
 */
template <typename Locker>
inline bool locker<Locker>::lock()
{
    return lock(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the exclusive lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Locker>
inline bool locker<Locker>::lock(const size_t timeout)
{
    if (base_locker::lock())
    {
        if (!Locker::lock(timeout))
        {
            base_locker::unlock();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error installing the exclusive lock "
                    << (NULL == name() ? "" : name()));
        }
        return true;
    }
    return false;
}

/**
 * Remove the exclusive lock
 * @return the result of the removing
 */
template <typename Locker>
inline bool locker<Locker>::unlock()
{
    if (base_locker::unlock())
    {
        if (!Locker::unlock())
        {
            base_locker::lock();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error removing the exclusive lock "
                    << (NULL == name() ? "" : name()));
        }
        return true;
    }
    return false;
}

/**
 * Set the shared lock
 * @return the result of the setting
 */
template <typename Locker>
inline bool locker<Locker>::lock_sharable()
{
    return lock_sharable(OUROBOROS_LOCK_TIMEOUT);
}

/**
 * Set the shared lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
template <typename Locker>
inline bool locker<Locker>::lock_sharable(const size_t timeout)
{
    if (base_locker::lock_sharable())
    {
        if (!Locker::lock_sharable(timeout))
        {
            base_locker::unlock_sharable();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error installing the shared lock "
                    << (NULL == name() ? "" : name()));
        }
        return true;
    }
    return false;
}

/**
 * Remove the shared lock
 * @return the result of the removing
 */
template <typename Locker>
inline bool locker<Locker>::unlock_sharable()
{
    if (base_locker::unlock_sharable())
    {
        if (!Locker::unlock_sharable())
        {
            base_locker::lock_sharable();
            // cppcheck-suppress knownConditionTrueFalse
            OUROBOROS_THROW_ERROR(lock_error, "error removing the shared lock "
                    << (NULL == name() ? "" : name()));
        }
        return true;
    }
    return false;
}

/**
 * Get the name of the locker
 * @return the name of the locker
 */
template <typename Locker>
inline const char* locker<Locker>::name() const
{
    return Locker::name();
}

}   // namespace ouroboros

#endif	/* OUROBOROS_LOCKER_H */

