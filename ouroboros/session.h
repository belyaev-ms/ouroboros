/**
 * @file   session.h
 * The sessions for read/write with a data table
 */

#ifndef OUROBOROS_SESSION_H
#define	OUROBOROS_SESSION_H

#include "ouroboros/global.h"
#include "ouroboros/file.h"
#include "ouroboros/table.h"
#include "ouroboros/transaction.h"

namespace ouroboros
{

/**
 * The interface class adapter for a writable table
 */
template <typename Table>
class writable_table : public Table
{
    typedef Table base_table;
private:
    using base_table::revision;
    using base_table::set_revision;
    using base_table::inc_revision;
    using base_table::relevant;
    using base_table::lock_sharable;
    using base_table::unlock_sharable;
    using base_table::lock_scoped;
    using base_table::unlock_scoped;
    using base_table::sharable_count;
    using base_table::scoped_count;
    using base_table::set_beg_pos;
    using base_table::inc_beg_pos;
    using base_table::dec_beg_pos;
    using base_table::set_end_pos;
    using base_table::inc_end_pos;
    using base_table::dec_end_pos;
    using base_table::set_count;
};

/**
 * The interface class adapter for a readable table
 */
template <typename Table>
class readable_table : public writable_table<Table>
{
    typedef writable_table<Table> base_table;
private:
    using base_table::write;
    using base_table::rwrite;
    using base_table::add;
    using base_table::remove;
    using base_table::remove_back;
    using base_table::clear;
};

/**
 * The base session for a table. The session controls the state of a transaction
 * of a table
 */
template <typename Dataset>
class base_session
{
public:
    typedef Dataset dataset_type;
    typedef typename Dataset::key_type key_type;
    typedef typename Dataset::table_type table_type;

    base_session();
    base_session(const base_session& session);
    base_session(dataset_type& dataset, const key_type& key);
    virtual ~base_session();

    inline table_type& table(); ///< get the table
    inline const table_type& table() const; ///< get the table
    inline const key_type Key() const; ///< get the key of the table
    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    inline const transaction_state state() const; ///< get the state of the transaction

    inline table_type& operator() (); ///< get the table
    inline const table_type& operator() () const; ///< get the table
    inline table_type* operator-> (); ///< get the pointer to the table
    inline const table_type* operator-> () const; ///< get the pointer to the table

    inline const bool valid() const; ///< check the session is valid
    void operator= (const base_session& session);
protected:
    typedef typename table_type::unsafe_table unsafe_table; ///< the table without lock
    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
    virtual const bool do_start();  ///< start the transaction
    virtual const bool do_stop();   ///< stop the transaction
    virtual const bool do_cancel(); ///< cancel the transaction
protected:
    mutable bool m_glock; ///< the sign that the global lock is set
    mutable bool m_primary; ///< the sign that the session is primary
    dataset_type *m_dataset; ///< the supported dataset
    table_type *m_table; ///< the supported table
};

/**
 * The session for a table
 */
template <typename Dataset, template <typename> class Table>
class table_session : public base_session<Dataset>
{
    typedef base_session<Dataset> base_class;
public:
    typedef typename base_class::dataset_type dataset_type;
    typedef typename base_class::key_type key_type;
    typedef Table<typename base_class::table_type> table_type;

    table_session();
    table_session(const table_session& session);
    table_session(dataset_type& dataset, const key_type& key);

    inline table_type& table(); ///< get the table
    inline const table_type& table() const; ///< get the table

    inline table_type& operator() (); ///< get the table
    inline const table_type& operator() () const; ///< get the table
    inline table_type* operator-> (); ///< get the pointer to the table
    inline const table_type* operator-> () const; ///< get the pointer to the table

    void operator= (const table_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    virtual const bool do_start();  ///< start the transaction
    virtual const bool do_stop();   ///< stop the transaction
    virtual const bool do_cancel(); ///< cancel the transaction
protected:
    mutable bool m_lock;
};

/**
 * The session for read from a table
 */
template <typename Dataset>
class sharable_session : public table_session<Dataset, readable_table>
{
    typedef base_session<Dataset> raw_class;
    typedef table_session<Dataset, readable_table> base_class;
public:
    typedef typename base_class::dataset_type dataset_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::table_type table_type;

    sharable_session();
    sharable_session(const sharable_session& session);
    sharable_session(dataset_type& dataset, const key_type& key);
    virtual ~sharable_session();

    inline void operator= (const sharable_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
    virtual const bool do_start();  ///< start the transaction
};

/**
 * The session for write to a table
 */
template <typename Dataset>
class scoped_session : public table_session<Dataset, writable_table>
{
    typedef base_session<Dataset> raw_class;
    typedef table_session<Dataset, writable_table> base_class;
public:
    typedef typename base_class::dataset_type dataset_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::table_type table_type;

    scoped_session();
    scoped_session(const scoped_session& session);
    scoped_session(dataset_type& dataset, const key_type& key);
    virtual ~scoped_session();

    inline void operator= (const scoped_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    typedef typename dataset_type::lazy_transaction_type lazy_transaction_type;

    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
    virtual const bool do_start();  ///< start the transation
    virtual const bool do_stop();   ///< stop the transation
    virtual const bool do_cancel(); ///< cancel the transation
};

//==============================================================================
//  base_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset>
base_session<Dataset>::base_session() :
    m_glock(false),
    m_primary(false),
    m_dataset(NULL),
    m_table(NULL)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset>
base_session<Dataset>::base_session(const base_session& session) :
    m_glock(session.m_glock),
    m_primary(session.m_primary),
    m_dataset(session.m_dataset),
    m_table(session.m_table)
{
    session.m_glock = false;
    session.m_primary = false;
}

/**
 * Constructor
 * @param dataset the supported dataset
 * @param key the key of the table
 */
template <typename Dataset>
base_session<Dataset>::base_session(dataset_type& dataset, const key_type& key) :
    m_glock(false),
    m_primary(false),
    m_dataset(&dataset),
    m_table(dataset.table(key))
{
}

/**
 * Destructor
 */
//virtual
template <typename Dataset>
base_session<Dataset>::~base_session()
{
    unlock();
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset>
inline typename base_session<Dataset>::table_type& base_session<Dataset>::table()
{
    return *m_table;
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset>
inline const typename base_session<Dataset>::table_type& base_session<Dataset>::table() const
{
    return *m_table;
}

/**
 * Get the key of the table
 * @return the key of the table
 */
template <typename Dataset>
inline const typename base_session<Dataset>::key_type base_session<Dataset>::Key() const
{
    return m_table->skey().key;
}

/**
 * Start the transaction
 */
template <typename Dataset>
void base_session<Dataset>::start()
{
    if (TR_STARTED != state())
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
    lock();
    do_start();
}

/**
 * Stop the transaction
 */
template <typename Dataset>
void base_session<Dataset>::stop()
{
    OUROBOROS_SAFE_BEGIN;
    const bool result = do_stop();
    unlock();
    if (result)
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
    OUROBOROS_SAFE_END;
}

/**
 * Cancel the transaction
 */
template <typename Dataset>
void base_session<Dataset>::cancel()
{
    OUROBOROS_SAFE_BEGIN;
    const bool result = do_cancel();
    unlock();
    if (result)
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
    OUROBOROS_SAFE_END;
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Dataset>
const bool base_session<Dataset>::do_start()
{
    m_primary = TR_STARTED != state();
    return m_primary;
}

/**
 * Stop the transaction
 * @return the result of the stopping
 */
//virtual
template <typename Dataset>
const bool base_session<Dataset>::do_stop()
{
    if (m_primary)
    {
        m_primary = false;
        return true;
    }
    return false;
}

/**
 * Cancel the transaction
 * @return the result of the canceling
 */
//virtual
template <typename Dataset>
const bool base_session<Dataset>::do_cancel()
{
    if (m_primary)
    {
        m_primary = false;
        return true;
    }
    return false;
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <typename Dataset>
inline const transaction_state base_session<Dataset>::state() const
{
    return table().state();
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset>
inline typename base_session<Dataset>::table_type& base_session<Dataset>::operator() ()
{
    return table();
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset>
inline const typename base_session<Dataset>::table_type& base_session<Dataset>::operator() () const
{
    return table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Dataset>
inline typename base_session<Dataset>::table_type* base_session<Dataset>::operator-> ()
{
    return &table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Dataset>
inline const typename base_session<Dataset>::table_type* base_session<Dataset>::operator-> () const
{
    return &table();
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
void base_session<Dataset>::operator= (const base_session& session)
{
    m_glock = session.m_glock;
    m_primary = session.m_primary;
    m_dataset = session.m_dataset;
    m_table = session.m_table;
    session.m_glock = false;
    session.m_primary = false;
}

/**
 * Check the session is valid
 * @return the result of the checking
 */
template <typename Dataset>
inline const bool base_session<Dataset>::valid() const
{
    return m_table != NULL;
}

/**
 * Lock the table
 */
//virtual
template <typename Dataset>
void base_session<Dataset>::lock()
{
    assert(!m_glock);
    global_lazy_lock<typename dataset_type::interface_type>::lock();
    m_glock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Dataset>
void base_session<Dataset>::unlock()
{
    if (m_glock)
    {
        global_lazy_lock<typename dataset_type::interface_type>::unlock();
        m_glock = false;
    }
}

//==============================================================================
//  table_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset, template <typename> class Table>
table_session<Dataset, Table>::table_session() :
    base_class(),
    m_lock(false)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset, template <typename> class Table>
table_session<Dataset, Table>::table_session(const table_session& session) :
    base_class(session),
    m_lock(session.m_lock)
{
    session.m_lock = false;
}

/**
 * Constructor
 * @param dataset the supported dataset
 * @param key the key of the table
 */
template <typename Dataset, template <typename> class Table>
table_session<Dataset, Table>::table_session(dataset_type& dataset, const key_type& key) :
    base_class(dataset, key),
    m_lock(false)
{
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset, template <typename> class Table>
inline typename table_session<Dataset, Table>::table_type& table_session<Dataset, Table>::table()
{
    return static_cast<table_type&>(base_class::table());
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset, template <typename> class Table>
inline const typename table_session<Dataset, Table>::table_type& table_session<Dataset, Table>::table() const
{
    return static_cast<const table_type&>(base_class::table());
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset, template <typename> class Table>
inline typename table_session<Dataset, Table>::table_type& table_session<Dataset, Table>::operator() ()
{
    return table();
}

/**
 * Get the table
 * @return the table
 */
template <typename Dataset, template <typename> class Table>
inline const typename table_session<Dataset, Table>::table_type& table_session<Dataset, Table>::operator() () const
{
    return table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Dataset, template <typename> class Table>
inline typename table_session<Dataset, Table>::table_type* table_session<Dataset, Table>::operator-> ()
{
    return &table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Dataset, template <typename> class Table>
inline const typename table_session<Dataset, Table>::table_type* table_session<Dataset, Table>::operator-> () const
{
    return &table();
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Dataset, template <typename> class Table>
void table_session<Dataset, Table>::operator= (const table_session& session)
{
    m_lock = session.m_lock;
    session.m_lock = false;
    base_class::operator =(session);
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Dataset, template <typename> class Table>
const bool table_session<Dataset, Table>::do_start()
{
    if (base_class::do_start())
    {
        static_cast<unsafe_table&>(base_class::table()).start();
        return true;
    }
    return false;
}

/**
 * Stop the transaction
 * @return the result of the stopping
 */
//virtual
template <typename Dataset, template <typename> class Table>
const bool table_session<Dataset, Table>::do_stop()
{
    if (base_class::do_stop())
    {
        static_cast<unsafe_table&>(base_class::table()).stop();
        return true;
    }
    return false;
}

/**
 * Cancel the transaction
 * @return the result of the canceling
 */
//virtual
template <typename Dataset, template <typename> class Table>
const bool table_session<Dataset, Table>::do_cancel()
{
    if (base_class::do_cancel())
    {
        static_cast<unsafe_table&>(base_class::table()).cancel();
        return true;
    }
    return false;
}

//==============================================================================
//  sharable_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset>
sharable_session<Dataset>::sharable_session() :
    base_class()
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset>
sharable_session<Dataset>::sharable_session(const sharable_session& session) :
    base_class(session)
{
}

/**
 * Constructor
 * @param dataset the supported dataset
 * @param key the key of the table
 */
template <typename Dataset>
sharable_session<Dataset>::sharable_session(dataset_type& dataset, const key_type& key) :
    base_class(dataset, key)
{
    if (base_class::valid())
    {
        base_class::start();
    }
}

/**
 * Destructor
 */
//virtual
template <typename Dataset>
sharable_session<Dataset>::~sharable_session()
{
    if (std::uncaught_exception())
    {
        base_class::cancel();
    }
    else
    {
        base_class::stop();
    }
}

/**
 * Lock the table
 */
//virtual
template <typename Dataset>
void sharable_session<Dataset>::lock()
{
    base_class::lock();
    raw_class::table().lock_sharable();
    base_class::m_lock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Dataset>
void sharable_session<Dataset>::unlock()
{
    if (base_class::m_lock)
    {
        base_class::m_lock = false;
        raw_class::table().unlock_sharable();
        base_class::unlock();
    }
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Dataset>
const bool sharable_session<Dataset>::do_start()
{
    const bool result = base_class::do_start();
    if (base_class::m_lock && 1 == raw_class::table().sharable_count())
    {
        raw_class::table().refresh();
    }
    return result;
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
inline void sharable_session<Dataset>::operator= (const sharable_session& session)
{
    base_class::operator =(session);
}

//==============================================================================
//  scoped_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session() :
    base_class()
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session(const scoped_session& session) :
    base_class(session)
{
}

/**
 * Constructor
 * @param dataset the supported dataset
 * @param key the key of the table
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session(dataset_type& dataset, const key_type& key) :
    base_class(dataset, key)
{
    if (base_class::valid())
    {
        base_class::start();
        base_class::m_dataset->store_session(*this);
    }
}

/**
 * Destructor
 */
//virtual
template <typename Dataset>
scoped_session<Dataset>::~scoped_session()
{
    if (std::uncaught_exception())
    {
        base_class::cancel();
    }
    else
    {
        base_class::stop();
    }
}

/**
 * Lock the table
 */
//virtual
template <typename Dataset>
void scoped_session<Dataset>::lock()
{
    base_class::lock();
    raw_class::table().lock_scoped();
    base_class::m_lock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Dataset>
void scoped_session<Dataset>::unlock()
{
    if (base_class::m_lock)
    {
        base_class::m_lock = false;
        raw_class::table().unlock_scoped();
        base_class::unlock();
    }
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Dataset>
const bool scoped_session<Dataset>::do_start()
{
    const bool result = base_class::do_start();
    if (base_class::m_lock && 1 == raw_class::table().scoped_count())
    {
        raw_class::table().refresh();
    }
    return result;
}

/**
 * Stop the transaction
 * @return the result of the stopping
 */
//virtual
template <typename Dataset>
const bool scoped_session<Dataset>::do_stop()
{
    if (base_class::m_lock && 1 == raw_class::table().scoped_count())
    {
        ///@todo unsafe_table execute lock!!! why?
        raw_class::table().update();
        base_class::m_dataset->update_key(raw_class::table());
    }
    return base_class::do_stop();
}

/**
 * Cancel the transaction
 * @return the result of the canceling
 */
//virtual
template <typename Dataset>
const bool scoped_session<Dataset>::do_cancel()
{
    const bool result = base_class::do_cancel();
    if (base_class::m_lock && 1 == raw_class::table().scoped_count())
    {
        raw_class::table().recovery();
    }
    return result;
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
inline void scoped_session<Dataset>::operator= (const scoped_session& session)
{
    base_class::operator =(session);
}

}   //namespace ouroboros

#endif	/* OUROBOROS_SESSION_H */

