/**
 * @file   transaction.h
 * The transaction for read/write operations
 */

#ifndef OUROBOROS_TRANSACTION_H
#define	OUROBOROS_TRANSACTION_H

#include <assert.h>
#include <list>
#include "ouroboros/global.h"
#include "ouroboros/locker.h"

#define OUROBOROS_SAFE_BEGIN try {
#define OUROBOROS_SAFE_END   } catch (const std::exception& e) { OUROBOROS_ERROR(PE(e.what())); assert(0); }

namespace ouroboros
{

/**
 * The base global locker
 */
class base_global_locker
{
public:
    static count_type sharable_count(); ///< get the count of the locks for read data
    static count_type scoped_count(); ///< get the count of the locks for write data
protected:
    inline base_global_locker() {}
private:
    base_global_locker(const base_global_locker&);
    base_global_locker& operator=(const base_global_locker&);
protected:
    static count_type s_scoped_count;
    static count_type s_sharable_count;
};

/**
 * The interface class adapter for a global locker
 */
template <typename Interface>
class global_locker : public base_global_locker, public Interface::locker_type
{
    typedef typename Interface::locker_type base_class;
public:
    static global_locker<Interface>& instance();
protected:
    global_locker();
private:
    global_locker(const global_locker&);
    global_locker& operator=(const global_locker&);
};

/**
 * The global lock
 */
template <typename Interface>
class global_lock
{
    typedef global_locker<Interface> locker_type;
public:
    global_lock();
    explicit global_lock(const size_t timeout);
    ~global_lock();

    static bool lock(); ///< set the global lock
    static bool lock(const size_t timeout); ///< set the global lock with a timeout
    static bool unlock(); ///< remove the global lock
};

/**
 * The global lazy lock
 * @attention use to wait until the global lock is removed
 */
template <typename Interface>
class global_lazy_lock
{
    typedef global_locker<Interface> locker_type;
public:
    global_lazy_lock();
    explicit global_lazy_lock(const size_t timeout);
    ~global_lazy_lock();

    static bool lock(); ///< set the global lazy lock
    static bool lock(const size_t timeout); ///< set the global lazy lock with a timeout
    static bool unlock(); ///< remove the global lazy lock
};

/**
 * The base transaction
 */
class base_transaction
{
public:
    base_transaction();
    virtual ~base_transaction();

    virtual void attach(base_transaction *transact); ///< attach a transaction
    virtual void start(); ///< start the transaction
    virtual void stop(); ///< stop the transaction
    virtual void cancel(); ///< cancel the transaction
    bool enabled() const; ///< check the transaction is enabled
private:
    base_transaction(const base_transaction& );
    base_transaction& operator= (const base_transaction& );
private:
    bool m_enabled;
};

/**
 * The transaction with the lock
 */
template <typename Lock>
class locked_transaction : public base_transaction
{
public:
    typedef Lock lock_type;
    locked_transaction();
    virtual ~locked_transaction();
protected:
    inline void lock(); ///< lock the transaction
    inline void unlock(); ///< unlock the transaction
    inline bool locked() const; ///< check the the transaction is locked
private:
    bool m_lock;
};

/**
 * The transaction for the dataset
 */
template <typename Dataset>
class dataset_transaction : public locked_transaction<typename Dataset::lock_type>
{
    typedef locked_transaction<typename Dataset::lock_type> base_class;
public:
    typedef Dataset dataset_type; ///< the dataset
    explicit dataset_transaction(dataset_type& dataset);
    virtual ~dataset_transaction();

    virtual void start(); ///< start the transaction
    virtual void stop(); ///< stop the transaction
    virtual void cancel(); ///< cancel the transaction
private:
    dataset_type& m_dataset; ///< the supported dataset
};

/**
 * The lazy transaction for the dataset
 * @attention when a lazy transaction is started access from another processes
 * to the dataset is not disabled, but the all sessions for write data to tables
 * of the dataset will be closed when the lazy transaction will be stopped and
 * all changes will be commit together (either all changes will be commit, or none)
 */
template <typename Dataset>
class lazy_transaction : public locked_transaction<typename Dataset::lazy_lock_type>
{
    typedef locked_transaction<typename Dataset::lazy_lock_type> base_class;
public:
    typedef Dataset dataset_type; ///< the dataset
    typedef typename dataset_type::session_write session_write; ///< the session for write to a table

    explicit lazy_transaction(dataset_type& dataset);
    virtual ~lazy_transaction();

    void push(session_write& session); ///< push a session to context of the transaction
    virtual void start(); ///< start the transaction
    virtual void stop(); ///< stop the transaction
    virtual void cancel(); ///< cancel the transaction
protected:
    void SessionsStop(); ///< stop all session of the transaction
    void SessionsCancel(); ///< cancel all session of the transaction
private:
    typedef typename dataset_type::table_type table_type;
    typedef std::list<session_write> session_list;
    dataset_type& m_dataset; ///< the supported dataset
    session_list m_sessions; ///< the list of sessions that are open
};

/**
 * The base global transaction
 */
template <typename Interface, template <typename> class Lock, typename Helper>
class base_global_transaction : public locked_transaction<Lock<Interface> >
{
    typedef locked_transaction<Lock<Interface> > base_class;
public:
    typedef base_transaction transaction_type;
    virtual ~base_global_transaction();
    void attach(transaction_type *transact); ///< attach a transaction
    virtual void start(); ///< start the transaction
    virtual void stop(); ///< stop the transaction
    virtual void cancel(); ///< cancel the transaction
protected:
    base_global_transaction();
private:
    typedef std::list<transaction_type* > ptransaction_list;
    ptransaction_list m_ptransactions;
};

/**
 * The global transaction
 */
template <typename Interface, typename Helper>
class global_transaction : public base_global_transaction<Interface, global_lock, Helper>
{
    typedef base_global_transaction<Interface, global_lock, Helper> base_class;
public:
    global_transaction();
};

/**
 * The global lazy transaction
 */
template <typename Interface, typename Helper>
class global_lazy_transaction : public base_global_transaction<Interface, global_lazy_lock, Helper>
{
    typedef base_global_transaction<Interface, global_lazy_lock, Helper> base_class;
public:
    global_lazy_transaction();
};

/**
 * Assistant for creating a transaction
 */
template <typename Transaction>
struct transaction_maker
{
    template <typename Dataset>
    static Transaction *make(Dataset& dataset)
    {
        return new Transaction(dataset);
    }
};

//==============================================================================
//  global_locker
//==============================================================================
/**
 * Constructor
 */
template <typename Interface>
global_locker<Interface>::global_locker() :
    base_class("global_lock", base_global_locker::s_scoped_count,
        base_global_locker::s_sharable_count)
{
}

/**
 * Get the instance of the class
 * @return the instance of the class
 */
//static
template <typename Interface>
global_locker<Interface>& global_locker<Interface>::instance()
{
    static global_locker<Interface> self;
    return self;
}

//==============================================================================
//  global_lock
//==============================================================================
/**
 * Constructor
 */
template <typename Interface>
global_lock<Interface>::global_lock()
{
    lock();
}

/**
 * Constructor
 * @param timeout the timeout
 */
template <typename Interface>
global_lock<Interface>::global_lock(const size_t timeout)
{
    lock(timeout);
}

/**
 * Destructor
 */
template <typename Interface>
global_lock<Interface>::~global_lock()
{
    unlock();
}

/**
 * Set the global lock
 * @return the result of setting
 */
//static
template <typename Interface>
bool global_lock<Interface>::lock()
{
    return locker_type::instance().lock();
}

/**
 * Set the global lock
 * @param timeout the timeout
 * @return the result of setting
 */
//static
template <typename Interface>
bool global_lock<Interface>::lock(const size_t timeout)
{
    return locker_type::instance().lock(timeout);
}

/**
 * Remove the global lock
 */
//static
template <typename Interface>
bool global_lock<Interface>::unlock()
{
    return locker_type::instance().unlock();
}

//==============================================================================
//  global_lazy_lock
//==============================================================================
/**
 * Constructor
 */
template <typename Interface>
global_lazy_lock<Interface>::global_lazy_lock()
{
    lock();
}

/**
 * Constructor
 * @param timeout the timeout
 */
template <typename Interface>
global_lazy_lock<Interface>::global_lazy_lock(const size_t timeout)
{
    lock(timeout);
}

/**
 * Destructor
 */
template <typename Interface>
global_lazy_lock<Interface>::~global_lazy_lock()
{
    unlock();
}

/**
 * Set the global lazy lock
 * @return the result of the setting
 */
//static
template <typename Interface>
bool global_lazy_lock<Interface>::lock()
{
    return locker_type::instance().lock_sharable();
}

/**
 * Set the global lazy lock with a timeout
 * @param timeout the timeout
 * @return the result of the setting
 */
//static
template <typename Interface>
bool global_lazy_lock<Interface>::lock(const size_t timeout)
{
    return locker_type::instance().lock_sharable(timeout);
}

/**
 * Remove the global lazy lock
 * @return the result of the removing
 */
//static
template <typename Interface>
bool global_lazy_lock<Interface>::unlock()
{
    return locker_type::instance().unlock_sharable();
}

//==============================================================================
//  locked_transaction
//==============================================================================
/**
 * Constructor
 */
template <typename Lock>
locked_transaction<Lock>::locked_transaction() :
    m_lock(false)
{

}

/**
 * Destructor
 */
//virtual
template <typename Lock>
locked_transaction<Lock>::~locked_transaction()
{
    unlock();
}

/**
 * Lock the transaction
 */
template <typename Lock>
inline void locked_transaction<Lock>::lock()
{
    if (!m_lock)
    {
        lock_type::lock();
        m_lock = true;
    }
}

/**
 * Unlock the transaction
 */
template <typename Lock>
inline void locked_transaction<Lock>::unlock()
{
    if (m_lock)
    {
        lock_type::unlock();
        m_lock = false;
    }
}

/**
 * Check the the transaction is locked
 * @return the result of the checking
 */
template <typename Lock>
inline bool locked_transaction<Lock>::locked() const
{
    return m_lock;
}

//==============================================================================
//  dataset_transaction
//==============================================================================
/**
 * Constructor
 * @param dataset the dataset
 */
template <typename Dataset>
dataset_transaction<Dataset>::dataset_transaction(dataset_type& dataset) :
    m_dataset(dataset)
{
    start();
}

/**
 * Destructor
 */
//virtual
template <typename Dataset>
dataset_transaction<Dataset>::~dataset_transaction()
{
    if (std::uncaught_exception())
    {
        cancel();
    }
    else
    {
        stop();
    }
}

/**
 * Start the transaction
 */
//virtual
template <typename Dataset>
void dataset_transaction<Dataset>::start()
{
    base_class::lock();
    if (!base_class::enabled())
    {
        m_dataset.start();
        base_class::start();
    }
}

/**
 * Stop the transaction
 */
//virtual
template <typename Dataset>
void dataset_transaction<Dataset>::stop()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        m_dataset.stop();
        base_class::stop();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

/**
 * Cancel the transaction
 */
//virtual
template <typename Dataset>
void dataset_transaction<Dataset>::cancel()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        m_dataset.cancel();
        base_class::cancel();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

//==============================================================================
//  lazy_transaction
//==============================================================================
/**
 * Constructor
 * @param dataset the dataset
 */
template <typename Dataset>
lazy_transaction<Dataset>::lazy_transaction(dataset_type& dataset) :
    m_dataset(dataset)
{
    start();
}

/**
 * Destructor
 */
//virtual
template <typename Dataset>
lazy_transaction<Dataset>::~lazy_transaction()
{
    if (std::uncaught_exception())
    {
        cancel();
    }
    else
    {
        stop();
    }
}

/**
 * Push a session to context of the transaction
 * @param session the session for write a table
 */
template <typename Dataset>
void lazy_transaction<Dataset>::push(session_write& session)
{
    OUROBOROS_ASSERT(base_class::enabled() && m_dataset.lazy_transaction_exists());
    if (1 == static_cast<const table_type&>(session.table()).scoped_count())
    {
        m_sessions.push_front(session);
    }
}

/**
 * Start the transaction
 */
//virtual
template <typename Dataset>
void lazy_transaction<Dataset>::start()
{
    base_class::lock();
    if (!base_class::enabled())
    {
        OUROBOROS_ASSERT(!m_dataset.lazy_transaction_exists());
        m_dataset.lazy_start();
        m_dataset.lazy_transaction(this);
        base_class::start();
    }
}

/**
 * Stop the transaction
 */
//virtual
template <typename Dataset>
void lazy_transaction<Dataset>::stop()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        OUROBOROS_ASSERT(m_dataset.lazy_transaction_exists());
        SessionsStop();
        m_dataset.lazy_stop();
        m_dataset.lazy_transaction(NULL);
        base_class::stop();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

/**
 * Stop all session of the transaction
 */
template <typename Dataset>
void lazy_transaction<Dataset>::SessionsStop()
{
    const typename session_list::iterator end = m_sessions.end();
    for (typename session_list::iterator it = m_sessions.begin(); it != end; ++it)
    {
        it->stop();
        const table_type& table = static_cast<const table_type&>(it->table());
        assert(table.sharable_count() == 0);
        assert(table.scoped_count() == 0);
    }
    m_sessions.clear();
}

/**
 * Cancel the transaction
 */
//virtual
template <typename Dataset>
void lazy_transaction<Dataset>::cancel()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        OUROBOROS_ASSERT(m_dataset.lazy_transaction_exists());
        SessionsCancel();
        m_dataset.lazy_cancel();
        m_dataset.lazy_transaction(NULL);
        base_class::cancel();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

/**
 * Cancel all session of the transaction
 */
template <typename Dataset>
void lazy_transaction<Dataset>::SessionsCancel()
{
    const typename session_list::iterator end = m_sessions.end();
    for (typename session_list::iterator it = m_sessions.begin(); it != end; ++it)
    {
        it->cancel();
    }
    m_sessions.clear();
}

//==============================================================================
//  base_global_transaction
//==============================================================================
/**
 * Constructor
 */
template <typename Interface, template <typename> class Lock, typename Helper>
base_global_transaction<Interface, Lock, Helper>::base_global_transaction()
{
}

/**
 * Destructor
 */
//virtual
template <typename Interface, template <typename> class Lock, typename Helper>
base_global_transaction<Interface, Lock, Helper>::~base_global_transaction()
{
    if (std::uncaught_exception())
    {
        cancel();
    }
    else
    {
        stop();
    }
}

/**
 * Attach a transaction
 * @param transact the transaction
 */
template <typename Interface, template <typename> class Lock, typename Helper>
void base_global_transaction<Interface, Lock, Helper>::attach(transaction_type *transact)
{
    m_ptransactions.push_front(transact);
}

/**
 * Start the transaction
 */
//virtual
template <typename Interface, template <typename> class Lock, typename Helper>
void base_global_transaction<Interface, Lock, Helper>::start()
{
    base_class::lock();
    if (!base_class::enabled())
    {
        base_class::start();
        Helper()(*this);
    }
}

/**
 * Stop the transaction
 */
//virtual
template <typename Interface, template <typename> class Lock, typename Helper>
void base_global_transaction<Interface, Lock, Helper>::stop()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        const ptransaction_list::iterator end = m_ptransactions.end();
        for (ptransaction_list::iterator it = m_ptransactions.begin(); it != end; ++it)
        {
            (*it)->stop();
            delete *it;
        }
        m_ptransactions.clear();
        base_class::stop();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

/**
 * Cancel the transaction
 */
//virtual
template <typename Interface, template <typename> class Lock, typename Helper>
void base_global_transaction<Interface, Lock, Helper>::cancel()
{
    OUROBOROS_SAFE_BEGIN;
    if (base_class::enabled())
    {
        const ptransaction_list::iterator end = m_ptransactions.end();
        for (ptransaction_list::iterator it = m_ptransactions.begin(); it != end; ++it)
        {
            (*it)->cancel();
            delete *it;
        }
        m_ptransactions.clear();
        base_class::cancel();
    }
    base_class::unlock();
    OUROBOROS_SAFE_END;
}

//==============================================================================
//  global_transaction
//==============================================================================
/**
 * Constructor
 */
template <typename Interface, typename Helper>
global_transaction<Interface, Helper>::global_transaction()
{
    base_class::start();
}

//==============================================================================
//  global_lazy_transaction
//==============================================================================
/**
 * Constructor
 */
template <typename Interface, typename Helper>
global_lazy_transaction<Interface, Helper>::global_lazy_transaction()
{
    base_class::start();
}

}   // namespace ouroboros

#endif	/* OUROBOROS_TRANSACTION_H */

