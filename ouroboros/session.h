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
template <typename Table, typename GlobalLock>
class base_session
{
public:
    typedef Table table_type;
    typedef typename table_type::skey_type::key_type key_type;
    typedef GlobalLock global_lock_type;

    base_session();
    base_session(const base_session& session);
    explicit base_session(table_type *table);
    virtual ~base_session();

    inline table_type& table(); ///< get the table
    inline const table_type& table() const; ///< get the table
    inline key_type key() const; ///< get the key of the table
    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    inline transaction_state state() const; ///< get the state of the transaction

    inline table_type& operator() (); ///< get the table
    inline const table_type& operator() () const; ///< get the table
    inline table_type* operator-> (); ///< get the pointer to the table
    inline const table_type* operator-> () const; ///< get the pointer to the table

    inline bool valid() const; ///< check the session is valid
    base_session& operator= (const base_session& session);
protected:
    typedef typename table_type::unsafe_table unsafe_table; ///< the table without lock
    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
    virtual bool do_start();  ///< start the transaction
    virtual bool do_stop();   ///< stop the transaction
    virtual bool do_cancel(); ///< cancel the transaction
protected:
    mutable bool m_glock; ///< the sign that the global lock is set
    mutable bool m_primary; ///< the sign that the session is primary
    table_type *m_table; ///< the supported table
};

/**
 * The session for a table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
class table_session : public base_session<Table, GlobalLock>
{
    typedef base_session<Table, GlobalLock> base_class;
public:
    typedef Table base_table_type;
    typedef typename base_class::global_lock_type global_lock_type;
    typedef TableInterface<typename base_class::table_type> table_type;

    table_session();
    table_session(const table_session& session);
    explicit table_session(base_table_type *table);

    inline table_type& table(); ///< get the table
    inline const table_type& table() const; ///< get the table

    inline table_type& operator() (); ///< get the table
    inline const table_type& operator() () const; ///< get the table
    inline table_type* operator-> (); ///< get the pointer to the table
    inline const table_type* operator-> () const; ///< get the pointer to the table

    table_session& operator= (const table_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    virtual bool do_start();  ///< start the transaction
    virtual bool do_stop();   ///< stop the transaction
    virtual bool do_cancel(); ///< cancel the transaction
protected:
    mutable bool m_lock;
};

/**
 * The base session for read from a table
 */
template <typename Table, typename GlobalLock>
class base_sharable_session : public table_session<Table, readable_table, GlobalLock>
{
    typedef base_session<Table, GlobalLock> raw_class;
    typedef table_session<Table, readable_table, GlobalLock> base_class;
public:
    typedef typename base_class::table_type table_type;
    typedef typename base_class::base_table_type base_table_type;
    typedef typename base_class::global_lock_type global_lock_type;

    base_sharable_session();
    base_sharable_session(const base_sharable_session& session);
    explicit base_sharable_session(base_table_type *table);

    base_sharable_session& operator= (const base_sharable_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
    virtual bool do_start();  ///< start the transaction
};

/**
 * The base session for write to a table
 */
template <typename Table, typename GlobalLock>
class base_scoped_session : public table_session<Table, writable_table, GlobalLock>
{
    typedef base_session<Table, GlobalLock> raw_class;
    typedef table_session<Table, writable_table, GlobalLock> base_class;
public:
    typedef typename base_class::table_type table_type;
    typedef typename base_class::base_table_type base_table_type;
    typedef typename base_class::global_lock_type global_lock_type;

    base_scoped_session();
    base_scoped_session(const base_scoped_session& session);
    explicit base_scoped_session(base_table_type *table);

    base_scoped_session& operator= (const base_scoped_session& session);
protected:
    typedef typename base_class::unsafe_table unsafe_table;
    virtual void lock(); ///< lock the table
    virtual void unlock(); ///< unlock the table
};

/**
 * The session for read from a table
 */
template <typename Dataset>
class sharable_session : public base_sharable_session<typename Dataset::table_type,
        global_lazy_lock<typename Dataset::interface_type> >
{
    typedef base_sharable_session<typename Dataset::table_type,
        global_lazy_lock<typename Dataset::interface_type> > base_class;
public:
    typedef Dataset dataset_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::table_type table_type;
    typedef typename base_class::global_lock_type global_lock_type;

    sharable_session();
    sharable_session(const sharable_session& session);
    sharable_session(dataset_type& dataset, const key_type& key);
    virtual ~sharable_session();

    sharable_session& operator= (const sharable_session& session);
};

/**
 * The session for write to a table
 */
template <typename Dataset>
class scoped_session : public base_scoped_session<typename Dataset::table_type,
        global_lazy_lock<typename Dataset::interface_type> >
{
    friend class lazy_transaction<Dataset>;
    typedef base_session<typename Dataset::table_type,
        global_lazy_lock<typename Dataset::interface_type> > raw_class;
    typedef base_scoped_session<typename Dataset::table_type,
        global_lazy_lock<typename Dataset::interface_type> > base_class;
public:
    typedef Dataset dataset_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::table_type table_type;
    typedef typename base_class::global_lock_type global_lock_type;

    scoped_session();
    scoped_session(const scoped_session& session);
    scoped_session(dataset_type& dataset, const key_type& key);
    virtual ~scoped_session();

    scoped_session& operator= (const scoped_session& session);
private:
    virtual bool do_start();  ///< start the transation
    virtual bool do_stop();   ///< stop the transation
    virtual bool do_cancel(); ///< cancel the transation
private:
    mutable dataset_type *m_dataset; ///< the supported dataset
};

/**
 * The session for write to a keys table
 */
template <typename Dataset>
class scoped_key_session : public base_scoped_session<typename Dataset::key_table_type,
        global_lock<typename Dataset::interface_type> >
{
    typedef base_session<typename Dataset::key_table_type,
        global_lock<typename Dataset::interface_type> > raw_class;
    typedef base_scoped_session<typename Dataset::key_table_type,
        global_lock<typename Dataset::interface_type> > base_class;
public:
    typedef Dataset dataset_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::table_type table_type;
    typedef typename base_class::global_lock_type global_lock_type;

    scoped_key_session();
    scoped_key_session(const scoped_key_session& session);
    explicit scoped_key_session(dataset_type& dataset);
    virtual ~scoped_key_session();

    scoped_key_session& operator= (const scoped_key_session& session);
private:
    virtual bool do_start();  ///< start the transation
    virtual bool do_stop();   ///< stop the transation
    virtual bool do_cancel(); ///< cancel the transation
private:
    mutable dataset_type *m_dataset; ///< the supported dataset
};

//==============================================================================
//  base_session
//==============================================================================
/**
 * Constructor
 */
template <typename Table, typename GlobalLock>
base_session<Table, GlobalLock>::base_session() :
    m_glock(false),
    m_primary(false),
    m_table(NULL)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Table, typename GlobalLock>
base_session<Table, GlobalLock>::base_session(const base_session& session) :
    m_glock(session.m_glock),
    m_primary(session.m_primary),
    m_table(session.m_table)
{
    session.m_glock = false;
    session.m_primary = false;
}

/**
 * Constructor
 * @param table the supported table
 */
template <typename Table, typename GlobalLock>
base_session<Table, GlobalLock>::base_session(table_type *table) :
    m_glock(false),
    m_primary(false),
    m_table(table)
{
}

/**
 * Destructor
 */
//virtual
template <typename Table, typename GlobalLock>
base_session<Table, GlobalLock>::~base_session()
{
    base_session::unlock();
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, typename GlobalLock>
inline typename base_session<Table, GlobalLock>::table_type&
    base_session<Table, GlobalLock>::table()
{
    return *m_table;
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, typename GlobalLock>
inline const typename base_session<Table, GlobalLock>::table_type&
    base_session<Table, GlobalLock>::table() const
{
    return *m_table;
}

/**
 * Get the key of the table
 * @return the key of the table
 */
template <typename Table, typename GlobalLock>
inline typename base_session<Table, GlobalLock>::key_type
    base_session<Table, GlobalLock>::key() const
{
    return m_table->skey().key;
}

/**
 * Start the transaction
 */
template <typename Table, typename GlobalLock>
void base_session<Table, GlobalLock>::start()
{
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (TR_STARTED != state())
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
#endif
    lock();
    do_start();
}

/**
 * Stop the transaction
 */
template <typename Table, typename GlobalLock>
void base_session<Table, GlobalLock>::stop()
{
    OUROBOROS_SAFE_BEGIN;
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    const bool result =
#endif
    do_stop();
    unlock();
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (result)
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
#endif
    OUROBOROS_SAFE_END;
}

/**
 * Cancel the transaction
 */
template <typename Table, typename GlobalLock>
void base_session<Table, GlobalLock>::cancel()
{
    OUROBOROS_SAFE_BEGIN;
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    const bool result =
#endif
    do_cancel();
    unlock();
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (result)
    {
        assert(m_table->sharable_count() == 0);
        assert(m_table->scoped_count() == 0);
    }
#endif
    OUROBOROS_SAFE_END;
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Table, typename GlobalLock>
bool base_session<Table, GlobalLock>::do_start()
{
    m_primary = TR_STARTED != state();
    return m_primary;
}

/**
 * Stop the transaction
 * @return the result of the stopping
 */
//virtual
template <typename Table, typename GlobalLock>
bool base_session<Table, GlobalLock>::do_stop()
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
template <typename Table, typename GlobalLock>
bool base_session<Table, GlobalLock>::do_cancel()
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
template <typename Table, typename GlobalLock>
inline transaction_state base_session<Table, GlobalLock>::state() const
{
    return table().state();
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, typename GlobalLock>
inline typename base_session<Table, GlobalLock>::table_type&
    base_session<Table, GlobalLock>::operator() ()
{
    return table();
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, typename GlobalLock>
inline const typename base_session<Table, GlobalLock>::table_type&
    base_session<Table, GlobalLock>::operator() () const
{
    return table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Table, typename GlobalLock>
inline typename base_session<Table, GlobalLock>::table_type*
    base_session<Table, GlobalLock>::operator-> ()
{
    return &table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Table, typename GlobalLock>
inline const typename base_session<Table, GlobalLock>::table_type*
    base_session<Table, GlobalLock>::operator-> () const
{
    return &table();
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Table, typename GlobalLock>
base_session<Table, GlobalLock>& base_session<Table, GlobalLock>::
    operator= (const base_session& session)
{
    m_glock = session.m_glock;
    m_primary = session.m_primary;
    m_table = session.m_table;
    session.m_glock = false;
    session.m_primary = false;
    return *this;
}

/**
 * Check the session is valid
 * @return the result of the checking
 */
template <typename Table, typename GlobalLock>
inline bool base_session<Table, GlobalLock>::valid() const
{
    return m_table != NULL;
}

/**
 * Lock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_session<Table, GlobalLock>::lock()
{
    assert(!m_glock);
    global_lock_type::lock();
    m_glock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_session<Table, GlobalLock>::unlock()
{
    if (m_glock)
    {
        global_lock_type::unlock();
        m_glock = false;
    }
}

//==============================================================================
//  table_session
//==============================================================================
/**
 * Constructor
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
table_session<Table, TableInterface, GlobalLock>::table_session() :
    base_class(),
    m_lock(false)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
table_session<Table, TableInterface, GlobalLock>::table_session(const table_session& session) :
    base_class(session),
    m_lock(session.m_lock)
{
    session.m_lock = false;
}

/**
 * Constructor
 * @param table the supported table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
table_session<Table, TableInterface, GlobalLock>::table_session(base_table_type *table) :
    base_class(table),
    m_lock(false)
{
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline typename table_session<Table, TableInterface, GlobalLock>::table_type&
    table_session<Table, TableInterface, GlobalLock>::table()
{
    return static_cast<table_type&>(base_class::table());
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline const typename table_session<Table, TableInterface, GlobalLock>::table_type&
    table_session<Table, TableInterface, GlobalLock>::table() const
{
    return static_cast<const table_type&>(base_class::table());
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline typename table_session<Table, TableInterface, GlobalLock>::table_type&
    table_session<Table, TableInterface, GlobalLock>::operator() ()
{
    return table();
}

/**
 * Get the table
 * @return the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline const typename table_session<Table, TableInterface, GlobalLock>::table_type&
    table_session<Table, TableInterface, GlobalLock>::operator() () const
{
    return table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline typename table_session<Table, TableInterface, GlobalLock>::table_type*
    table_session<Table, TableInterface, GlobalLock>::operator-> ()
{
    return &table();
}

/**
 * Get the pointer to the table
 * @return the pointer to the table
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
inline const typename table_session<Table, TableInterface, GlobalLock>::table_type*
    table_session<Table, TableInterface, GlobalLock>::operator-> () const
{
    return &table();
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
table_session<Table, TableInterface, GlobalLock>& table_session<Table,
    TableInterface, GlobalLock>::operator= (const table_session& session)
{
    m_lock = session.m_lock;
    session.m_lock = false;
    base_class::operator =(session);
    return *this;
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
bool table_session<Table, TableInterface, GlobalLock>::do_start()
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
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
bool table_session<Table, TableInterface, GlobalLock>::do_stop()
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
template <typename Table, template <typename> class TableInterface, typename GlobalLock>
bool table_session<Table, TableInterface, GlobalLock>::do_cancel()
{
    if (base_class::do_cancel())
    {
        static_cast<unsafe_table&>(base_class::table()).cancel();
        return true;
    }
    return false;
}

//==============================================================================
//  base_sharable_session
//==============================================================================
/**
 * Constructor
 */
template <typename Table, typename GlobalLock>
base_sharable_session<Table, GlobalLock>::base_sharable_session() :
    base_class()
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Table, typename GlobalLock>
base_sharable_session<Table, GlobalLock>::base_sharable_session(const base_sharable_session& session) :
    base_class(session)
{
}

/**
 * Constructor
 * @param table the supported table
 */
template <typename Table, typename GlobalLock>
base_sharable_session<Table, GlobalLock>::base_sharable_session(base_table_type *table) :
    base_class(table)
{

}

/**
 * Lock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_sharable_session<Table, GlobalLock>::lock()
{
    base_class::lock();
    raw_class::table().lock_sharable();
    base_class::m_lock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_sharable_session<Table, GlobalLock>::unlock()
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
template <typename Table, typename GlobalLock>
bool base_sharable_session<Table, GlobalLock>::do_start()
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
template <typename Table, typename GlobalLock>
base_sharable_session<Table, GlobalLock>& base_sharable_session<Table,
    GlobalLock>::operator= (const base_sharable_session& session)
{
    base_class::operator =(session);
    return *this;
}

//==============================================================================
//  base_scoped_session
//==============================================================================
/**
 * Constructor
 */
template <typename Table, typename GlobalLock>
base_scoped_session<Table, GlobalLock>::base_scoped_session() :
    base_class()
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Table, typename GlobalLock>
base_scoped_session<Table, GlobalLock>::base_scoped_session(const base_scoped_session& session) :
    base_class(session)
{
}

/**
 * Constructor
 * @param table the supported table
 */
template <typename Table, typename GlobalLock>
base_scoped_session<Table, GlobalLock>::base_scoped_session(base_table_type *table) :
    base_class(table)
{
}

/**
 * Lock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_scoped_session<Table, GlobalLock>::lock()
{
    base_class::lock();
    raw_class::table().lock_scoped();
    base_class::m_lock = true;
}

/**
 * Unlock the table
 */
//virtual
template <typename Table, typename GlobalLock>
void base_scoped_session<Table, GlobalLock>::unlock()
{
    if (base_class::m_lock)
    {
        base_class::m_lock = false;
        raw_class::table().unlock_scoped();
        base_class::unlock();
    }
}

/**
 * The operator =
 * @param session the another session
 */
template <typename Table, typename GlobalLock>
base_scoped_session<Table, GlobalLock>& base_scoped_session<Table,
    GlobalLock>::operator= (const base_scoped_session& session)
{
    base_class::operator =(session);
    return *this;
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
    base_class(dataset.table(key))
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
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
sharable_session<Dataset>& sharable_session<Dataset>::
    operator= (const sharable_session& session)
{
    base_class::operator =(session);
    return *this;
}

//==============================================================================
//  scoped_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session() :
    base_class(),
    m_dataset(NULL)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session(const scoped_session& session) :
    base_class(session),
    m_dataset(session.m_dataset)
{
    session.m_dataset = NULL;
}

/**
 * Constructor
 * @param dataset the supported dataset
 * @param key the key of the table
 */
template <typename Dataset>
scoped_session<Dataset>::scoped_session(dataset_type& dataset, const key_type& key) :
    base_class(dataset.table(key)),
    m_dataset(&dataset)
{
    if (base_class::valid())
    {
        base_class::start();
        m_dataset->store_session(*this);
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
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
scoped_session<Dataset>& scoped_session<Dataset>::
    operator= (const scoped_session& session)
{
    base_class::operator =(session);
    m_dataset = session.m_dataset;
    session.m_dataset = NULL;
    return *this;
}

/**
 * Start the transaction
 * @return the result of the starting
 */
template <typename Dataset>
bool scoped_session<Dataset>::do_start()
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
bool scoped_session<Dataset>::do_stop()
{
    if (base_class::m_lock && 1 == raw_class::table().scoped_count())
    {
        ///@todo unsafe_table execute lock!!! why?
        raw_class::table().update();
        m_dataset->update_key(raw_class::table());
    }
    return base_class::do_stop();
}

/**
 * Cancel the transaction
 * @return the result of the canceling
 */
//virtual
template <typename Dataset>
bool scoped_session<Dataset>::do_cancel()
{
    const bool result = base_class::do_cancel();
    if (base_class::m_lock && 1 == raw_class::table().scoped_count())
    {
        raw_class::table().recovery();
    }
    return result;
}


//==============================================================================
//  scoped_key_session
//==============================================================================
/**
 * Constructor
 */
template <typename Dataset>
scoped_key_session<Dataset>::scoped_key_session() :
    base_class(),
    m_dataset(NULL)
{
}

/**
 * Constructor
 * @param session the another session
 */
template <typename Dataset>
scoped_key_session<Dataset>::scoped_key_session(const scoped_key_session& session) :
    base_class(session),
    m_dataset(session.m_dataset)
{
    session.m_dataset = NULL;
}

/**
 * Constructor
 * @param dataset the supported dataset
 */
template <typename Dataset>
scoped_key_session<Dataset>::scoped_key_session(dataset_type& dataset) :
    base_class(&dataset.m_key_table),
    m_dataset(&dataset)
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
scoped_key_session<Dataset>::~scoped_key_session()
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
 * The operator =
 * @param session the another session
 */
template <typename Dataset>
inline scoped_key_session<Dataset>& scoped_key_session<Dataset>::
    operator= (const scoped_key_session& session)
{
    base_class::operator =(session);
    m_dataset = session.m_dataset;
    session.m_dataset = NULL;
    return *this;
}

/**
 * Start the transaction
 * @return the result of the starting
 */
//virtual
template <typename Dataset>
bool scoped_key_session<Dataset>::do_start()
{
    const bool result = base_class::do_start();
    if (base_class::m_lock)
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
bool scoped_key_session<Dataset>::do_stop()
{
    if (base_class::m_lock)
    {
        ///@todo unsafe_table execute lock!!! why?
        raw_class::table().update();
        m_dataset->update_info();
    }
    return base_class::do_stop();
}

/**
 * Cancel the transaction
 * @return the result of the canceling
 */
//virtual
template <typename Dataset>
bool scoped_key_session<Dataset>::do_cancel()
{
    const bool result = base_class::do_cancel();
    if (base_class::m_lock)
    {
        raw_class::table().recovery();
    }
    return result;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_SESSION_H */

