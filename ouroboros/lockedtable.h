/**
 * @file   lockedtable.h
 * The table with support lock for multi-access
 */

#ifndef OUROBOROS_LOCKEDTABLE_H
#define	OUROBOROS_LOCKEDTABLE_H

#include "ouroboros/global.h"
#include "ouroboros/locker.h"
#include "ouroboros/table.h"

namespace ouroboros
{

/**
 * The interface class for locking a table
 * @attention use it because there are the restriction of use template
 * specialization a method for a template class
 */
template <typename Table, typename Locker>
class table_lock
{
public:
    static inline void lock_sharable(const Table& table); ///< lock the table for reading
    static inline void lock_sharable(const Table& table, const size_t timeout); ///< lock the table for reading with a timeout
    static inline void unlock_sharable(const Table& table); ///< unlock the table for reading
    static inline void lock_scoped(const Table& table); ///< lock the table for writing
    static inline void lock_scoped(const Table& table, const size_t timeout); ///< lock the table for writing with a timeout
    static inline void unlock_scoped(const Table& table); ///< unlock the table for writing
};

template <typename Table>
class table_lock<Table, internal_locker>
{
public:
    static inline void lock_sharable(const Table& table); ///< lock the table for reading
    static inline void lock_sharable(const Table& table, const size_t timeout); ///< lock the table for reading with a timeout
    static inline void unlock_sharable(const Table& table); ///< unlock the table for reading
    static inline void lock_scoped(const Table& table); ///< lock the table for writing
    static inline void lock_scoped(const Table& table, const size_t timeout); ///< lock the table for writing with a timeout
    static inline void unlock_scoped(const Table& table); ///< unlock the table for writing
};

/**
 * The interface class for locking a table for reading
 */
template <typename Table>
class sharable_table_lock
{
public:
    typedef Table table_type;
    explicit inline sharable_table_lock(const table_type& table);
    inline sharable_table_lock(const table_type& table, const size_t timeout);
    inline ~sharable_table_lock();
protected:
    inline void lock() const;
    inline void lock(const size_t timeout) const;
    inline void unlock() const;
private:
    const table_type& m_table;
};

/**
 * The interface class for locking a table for writing
 */
template <typename Table>
class scoped_table_lock
{
public:
    typedef Table table_type;
    explicit inline scoped_table_lock(const table_type& table);
    inline scoped_table_lock(const table_type& table, const size_t timeout);
    inline ~scoped_table_lock();
protected:
    inline void lock() const;
    inline void lock(const size_t timeout) const;
    inline void unlock() const;
private:
    const table_type& m_table;
};

/**
 * The interface class for protecting a table
 */
template <typename Table>
class table_guard
{
public:
    typedef Table table_type;
    table_guard();
    explicit table_guard(const bool lock);
    table_guard(const bool lock, const size_t timeout);
    ~table_guard();
    inline void lock(table_type& table) const;
    inline void unlock() const;
public:
    const bool m_lock;
    const size_t m_timeout;
    mutable table_type *m_table;
};

/**
 * The table with support lock i/o operations
 */
///@todo add the special template for stub_locker
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
class locked_table : public Table<Source, typename Controlblock::skey_type, typename Controlblock::interface_type>
{
    typedef Table<Source, typename Controlblock::skey_type,
            typename Controlblock::interface_type> base_class;
public:
    typedef Source source_type;
    typedef base_class unsafe_table;
    typedef Controlblock controlblock_type;
    typedef typename controlblock_type::skey_type skey_type;
    typedef typename controlblock_type::interface_type interface_type;
    typedef table_guard<locked_table> guard_type;

    locked_table(source_type& source, skey_type& skey);
    locked_table(source_type& source, skey_type& skey, const guard_type& guard);
    locked_table(source_type& source, controlblock_type controlblock);
    locked_table(source_type& source, controlblock_type controlblock, const guard_type& guard);

    inline pos_type read(void *data, const pos_type pos) const; ///< read a record
    inline pos_type read(void *data, const pos_type beg, const count_type count) const; ///< read records [beg, beg + count)
    inline pos_type rread(void *data, const pos_type pos) const; ///< reverse read a record
    inline pos_type write(const void *data, const pos_type pos); ///< write a record
    inline pos_type write(const void *data, const pos_type beg, const count_type count); ///< write records [beg, beg + count)
    inline pos_type rwrite(const void *data, const pos_type pos); ///< reverse write a record
    inline pos_type add(const void *data); ///< add a record
    inline pos_type add(const void *data, const count_type count); ///< add records
    inline pos_type remove(const pos_type pos); ///< delete a record
    inline pos_type remove(const pos_type beg, const count_type count); ///< delete records [beg, beg + count)
    inline count_type remove_back(const count_type count); ///< delete records from the back
    inline pos_type read_front(void *data) const; ///< read the first record
    inline pos_type read_front(void *data, const count_type count) const; ///< read the first records
    inline pos_type read_back(void *data) const; ///< read the last record
    inline pos_type read_back(void *data, const count_type count) const; ///< read the last records
    inline pos_type find(const void *data, const pos_type beg, const count_type count) const; ///< find a record [beg, beg + count)
    inline pos_type rfind(const void *data, const pos_type end, const count_type count) const; ///< reverse find a record [end - count, end)

    inline pos_type beg_pos() const; ///< get the begin position of records
    inline void set_beg_pos(const pos_type pos); ///< set the begin position of records
    inline pos_type inc_beg_pos(const count_type count = 1); ///< increment the begin position of records
    inline pos_type dec_beg_pos(const count_type count = 1); ///< decrement the begin position of records
    inline pos_type end_pos() const; ///< get the end position of records
    inline void set_end_pos(const pos_type pos); ///< set the end position of records
    inline pos_type inc_end_pos(const count_type count = 1); ///< increment the end position of records
    inline pos_type dec_end_pos(const count_type count = 1); ///< decrement the end position of records
    inline count_type distance(const pos_type beg, const pos_type end) const; ///< calculate the count of pages in the range [beg, end)
    inline pos_type front_pos() const; ///< get the position of the first record
    inline pos_type back_pos() const; ///< get the position of the last record

    inline count_type count() const; ///< get the count of records
    inline void set_count(const count_type count); ///< set the count of records
    inline bool empty() const; ///< check the table is empty

    inline void clear(); ///< clear the table

    inline revision_type revision() const; ///< get the revision of modifying the table
    inline void set_revision(const revision_type rev); ///< set the revision of modifying the table
    inline revision_type inc_revision(); ///< increment the revision of modifying the table
    inline bool relevant() const; ///< check the table is relevant
    inline bool refresh(); ///< refresh the metadata of the table by the key
    inline void update(); ///< update the key by the metadata of the table
    inline void recovery(); ///< recovery the metadata of the table by the key

    inline void lock_sharable() const; ///< lock the table for reading
    inline void lock_sharable(const size_t timeout) const; ///< lock the table for reading with a timeout
    inline void unlock_sharable() const; ///< unlock the table for reading
    inline void lock_scoped() const; ///< lock the table for writing
    inline void lock_scoped(const size_t timeout) const; ///< lock the table for writing with a timeout
    inline void unlock_scoped() const; ///< unlock the table for writing

    inline count_type sharable_count() const; ///< get the count of the reading lock
    inline count_type scoped_count() const; ///< get the count of the writing lock

    inline void start();  ///< start the transaction
    inline void stop();   ///< stop the transaction
    inline void cancel(); ///< cancel the transaction

protected:
    typedef locked_table<Table, Source, Controlblock, Locker> table_type;
    typedef Locker locker_type;
    typedef sharable_table_lock<table_type> lock_read;
    typedef scoped_table_lock<table_type> lock_write;

    friend class table_lock<table_type, locker_type>;
    inline locker_type& locker() const; ///< get the locker of the table

private:
    mutable count_type m_scoped_count;
    mutable count_type m_sharable_count;
    mutable locker_type m_locker;
};

/**
 * The interface class adapter for locked_table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock>
class interface_locked_table : public locked_table<Table, Source, Controlblock,
        typename Controlblock::interface_type::locker_type>
{
    typedef locked_table<Table, Source, Controlblock,
            typename Controlblock::interface_type::locker_type> base_class;
public:
    typedef typename base_class::controlblock_type controlblock_type;
    typedef typename base_class::source_type source_type;
    typedef typename base_class::guard_type guard_type;
    typedef typename base_class::skey_type skey_type;
    interface_locked_table(source_type& source, skey_type& key) :
        base_class(source, key)
    {}
    interface_locked_table(source_type& source, skey_type& key, const guard_type& guard) :
        base_class(source, key, guard)
    {}
    interface_locked_table(source_type& source, controlblock_type controlblock) :
        base_class(source, controlblock)
    {}
    interface_locked_table(source_type& source, controlblock_type controlblock, const guard_type& guard) :
        base_class(source, controlblock, guard)
    {}
};

/**
 * The interface class for table source with support lock i/o operations
 */
template <typename Interface>
class locked_source : public source<typename Interface::file_type>
{
    typedef source<typename Interface::file_type> base_class;
public:
    typedef Interface interface_type;
    typedef typename interface_type::file_type file_type;
    typedef typename interface_type::file_page_type file_page_type;
    typedef typename base_class::file_region_type file_region_type;
    inline locked_source(file_type& file, const size_type rec_size, const options_type& options = options_type());
    inline locked_source(file_type& file, const count_type tbl_count, const size_type rec_size, const options_type& options = options_type());
    inline locked_source(file_type& file, const count_type tbl_count, const count_type rec_count, const size_type rec_size, const options_type& options = options_type());
    inline locked_source(const std::string& name, const size_type rec_size, const options_type& options = options_type());
    inline locked_source(const std::string& name, const count_type tbl_count, const size_type rec_size, const options_type& options = options_type());
    inline locked_source(const std::string& name, const count_type tbl_count, const count_type rec_count, const size_type rec_size, const options_type& options = options_type());
    inline void lock(const pos_type index) const;
    inline void unlock(const pos_type index) const;
    inline void lock_sharable(const pos_type index) const;
    inline void unlock_sharable(const pos_type index) const;
};

//==============================================================================
//  table_lock
//==============================================================================
/**
 * Lock the table for reading
 * @param table the table
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::lock_sharable(const Table& table)
{
    table.locker().lock_sharable();
}

/**
 * Lock the table for reading with a timeout
 * @param table the table
 * @param timeout the timeout
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::lock_sharable(const Table& table, const size_t timeout)
{
    if (0 == timeout)
    {
        table.locker().lock_sharable();
    }
    else
    {
        table.locker().lock_sharable(timeout);
    }
}

/**
 * Unlock the table for reading
 * @param table the table
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::unlock_sharable(const Table& table)
{
    table.locker().unlock_sharable();
}

/**
 * Lock the table for writing
 * @param table the table
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::lock_scoped(const Table& table)
{
    table.locker().lock();
}

/**
 * Lock the table for writing with a timeout
 * @param table the table
 * @param timeout the timeout
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::lock_scoped(const Table& table, const size_t timeout)
{
    if (0 == timeout)
    {
        table.locker().lock();
    }
    else
    {
        table.locker().lock(timeout);
    }
}

/**
 * Unlock the table for writing
 * @param table the table
 */
//static
template <typename Table, typename Locker>
inline void table_lock<Table, Locker>::unlock_scoped(const Table& table)
{
    table.locker().unlock();
}

//==============================================================================
//  table_lock<internal_locker>
//==============================================================================
/**
 * Lock the table for reading
 * @param table the table
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::lock_sharable(const Table& table)
{
    if (table.locker().lock_sharable())
    {
        table.source().lock_sharable(table.index());
    }
}

/**
 * Lock the table for reading with a timeout
 * @param table the table
 * @param timeout the timeout
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::lock_sharable(const Table& table,
    const size_t timeout)
{
    lock_sharable();
}

/**
 * Unlock the table for reading
 * @param table the table
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::unlock_sharable(const Table& table)
{
    if (table.locker().unlock_sharable())
    {
        table.source().unlock_sharable(table.index());
    }
}

/**
 * Lock the table for writing
 * @param table the table
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::lock_scoped(const Table& table)
{
    if (table.locker().lock())
    {
        table.source().lock(table.index());
    }
}

/**
 * Lock the table for writing with a timeout
 * @param table the table
 * @param timeout the timeout
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::lock_scoped(const Table& table,
    const size_t timeout)
{
    lock_scoped();
}

/**
 * Unlock the table for writing
 * @param table the table
 */
//static
template <typename Table>
inline void table_lock<Table, internal_locker>::unlock_scoped(const Table& table)
{
    if (table.locker().unlock())
    {
        table.source().unlock(table.index());
    }
}

//==============================================================================
//  sharable_table_lock
//==============================================================================
/**
 * Constructor
 * @param table the table
 */
template <typename Table>
inline sharable_table_lock<Table>::sharable_table_lock(const table_type& table) :
    m_table(table)
{
    lock();
}

/**
 * Constructor
 * @param table the table
 * @param timeout the timeout
 */
template <typename Table>
inline sharable_table_lock<Table>::sharable_table_lock(const table_type& table,
    const size_t timeout) :
    m_table(table)
{
    if (0 == timeout)
    {
        lock();
    }
    else
    {
        lock(timeout);
    }
}

/**
 * Destructor
 */
template <typename Table>
inline sharable_table_lock<Table>::~sharable_table_lock()
{
    unlock();
}

/**
 * Lock the table
 */
template <typename Table>
inline void sharable_table_lock<Table>::lock() const
{
    m_table.lock_sharable();
}

/**
 * Lock the table with a timeout
 * @param timeout the timeout
 */
template <typename Table>
inline void sharable_table_lock<Table>::lock(const size_t timeout) const
{
    m_table.lock_sharable(timeout);
}

/**
 * Unlock the table
 */
template <typename Table>
inline void sharable_table_lock<Table>::unlock() const
{
    m_table.unlock_sharable();
}

//==============================================================================
//  scoped_table_lock
//==============================================================================
/**
 * Constructor
 * @param table the table
 */
template <typename Table>
inline scoped_table_lock<Table>::scoped_table_lock(const table_type& table) :
    m_table(table)
{
    lock();
}

/**
 * Constructor
 * @param table the table
 * @param timeout the timeout
 */
template <typename Table>
inline scoped_table_lock<Table>::scoped_table_lock(const table_type& table,
    const size_t timeout) :
    m_table(table)
{
    if (0 == timeout)
    {
        lock();
    }
    else
    {
        lock(timeout);
    }
}

/**
 * Destructor
 */
template <typename Table>
inline scoped_table_lock<Table>::~scoped_table_lock()
{
    unlock();
}

/**
 * Lock the table
 */
template <typename Table>
inline void scoped_table_lock<Table>::lock() const
{
    m_table.lock_scoped();
}

/**
 * Lock the table with a timeout
 * @param timeout the timeout
 */
template <typename Table>
inline void scoped_table_lock<Table>::lock(const size_t timeout) const
{
    m_table.lock_scoped(timeout);
}

/**
 * Unlock the table
 */
template <typename Table>
inline void scoped_table_lock<Table>::unlock() const
{
    m_table.unlock_scoped();
}

//==============================================================================
//  table_guard
//==============================================================================
/**
 * Constructor
 */
template <typename Table>
table_guard<Table>::table_guard() :
    m_lock(true),
    m_timeout(0),
    m_table(NULL)
{
}

/**
 * Constructor
 * @param lock the flag for immediately lock the table
 */
template <typename Table>
table_guard<Table>::table_guard(const bool lock) :
    m_lock(lock),
    m_timeout(0),
    m_table(NULL)
{
}

/**
 * Constructor
 * @param lock the flag for immediately lock the table
 * @param timeout the timeout
 */
template <typename Table>
table_guard<Table>::table_guard(const bool lock, const size_t timeout) :
    m_lock(lock),
    m_timeout(timeout),
    m_table(NULL)
{
}

/**
 * Destructor
 */
template <typename Table>
table_guard<Table>::~table_guard()
{
    unlock();
}

/**
 * Lock the table
 * @param table the table
 */
template <typename Table>
inline void table_guard<Table>::lock(table_type& table) const
{
    if (m_lock)
    {
        if (0 == m_timeout)
        {
            table.lock_scoped();
        }
        else
        {
            table.lock_scoped(m_timeout);
        }
        m_table = &table;
    }
}

/**
 * Unlock the table
 */
template <typename Table>
inline void table_guard<Table>::unlock() const
{
    if (m_table != NULL)
    {
        m_table->unlock_scoped();
        m_table = NULL;
    }
}

//==============================================================================
//  locked_table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
locked_table<Table, Source, Controlblock, Locker>::locked_table(source_type& source, skey_type& skey) :
    base_class(source, skey),
    m_scoped_count(0),
    m_sharable_count(0),
    m_locker(make_object_name(source.name(), source.table_offset(skey.pos), "locker").c_str(), m_scoped_count, m_sharable_count)
{
}

/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 * @param guard the guard of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
locked_table<Table, Source, Controlblock, Locker>::locked_table(source_type& source, skey_type& skey, const guard_type& guard) :
    base_class(source, skey),
    m_scoped_count(0),
    m_sharable_count(0),
    m_locker(make_object_name(source.name(), source.table_offset(skey.pos), "locker").c_str(), m_scoped_count, m_sharable_count)
{
    guard.lock(*this);
}

/**
 * Constructor
 * @param source the source of data
 * @param controlblock the controlblock of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
locked_table<Table, Source, Controlblock, Locker>::locked_table(source_type& source,
        controlblock_type controlblock) :
    base_class(source, controlblock.get_skey()),
    m_scoped_count(0),
    m_sharable_count(0),
    m_locker(controlblock.get_lock(), m_scoped_count, m_sharable_count)
{
}

/**
 * Constructor
 * @param source the source of data
 * @param controlblock the controlblock of the table
 * @param guard the guard of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
locked_table<Table, Source, Controlblock, Locker>::locked_table(source_type& source,
        controlblock_type controlblock, const guard_type& guard) :
    base_class(source, controlblock.get_skey()),
    m_scoped_count(0),
    m_sharable_count(0),
    m_locker(controlblock.get_lock(), m_scoped_count, m_sharable_count)
{
    guard.lock(*this);
}

/**
 * Read a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read(void *data, const pos_type pos) const
{
    lock_read lock(*this);
    return base_class::read(data, pos);
}

/**
 * Read records in the range [beg, beg + count)
 * @param data data of the records
 * @param beg the begin position of the records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read(void *data, const pos_type beg, const count_type count) const
{
    lock_read lock(*this);
    return base_class::read(data, beg, count);
}

/**
 * Reverse read a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::rread(void *data, const pos_type pos) const
{
    lock_read lock(*this);
    return base_class::rread(data, pos);
}

/**
 * Write a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::write(const void *data, const pos_type pos)
{
    lock_write lock(*this);
    return base_class::write(data, pos);
}

/**
 * Write records in the range [beg, beg + count)
 * @param data data of the records
 * @param beg the begin position of the records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::write(const void *data, const pos_type beg, const count_type count)
{
    lock_write lock(*this);
    return base_class::write(data, beg, count);
}

/**
 * Reverse write a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::rwrite(const void *data, const pos_type pos)
{
    lock_write lock(*this);
    return base_class::rwrite(data, pos);
}

/**
 * Add a record
 * @param data data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::add(const void *data)
{
    lock_write lock(*this);
    return base_class::add(data);
}

/**
 * Add records
 * @param data data of the records
 * @param count the count of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::add(const void *data, const count_type count)
{
    lock_write lock(*this);
    return base_class::add(data, count);
}

/**
 * Delete a record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::remove(const pos_type pos)
{
    lock_write lock(*this);
    return base_class::remove(pos);
}

/**
 * Delete records in the range [beg, end + count)
 * @param beg the begin position of the record
 * @param count the count of the delete record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::remove(const pos_type beg, const count_type count)
{
    lock_write lock(*this);
    return base_class::remove(beg, count);
}

/**
 * Delete records from the back of the table
 * @param count the count of deleted records
 * @return the count of remaining records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::remove_back(const count_type count)
{
    lock_write lock(*this);
    return base_class::remove_back(count);
}

/**
 * Read the first record
 * @param data data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read_front(void* data) const
{
    lock_read lock(*this);
    return base_class::read_front(data);
}

/**
 * Read the first records
 * @param data data of the first records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read_front(void* data, const count_type count) const
{
    lock_read lock(*this);
    return base_class::read_front(data, count);
}

/**
 * Read the last record
 * @param data data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read_back(void* data) const
{
    lock_read lock(*this);
    return base_class::read_back(data);
}

/**
 * Read the last records
 * @param data data of the last records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::read_back(void* data, const count_type count) const
{
    lock_read lock(*this);
    return base_class::read_back(data, count);
}

/**
 * Find a record in the range [beg, beg + count)
 * @param data data of the record
 * @param beg the begin position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::find(const void *data, const pos_type beg, const count_type count) const
{
    lock_read lock(*this);
    return base_class::find(data, beg, count);
}

/**
 * Reverse find a record in the range [end - count, end)
 * @param data data of the record
 * @param end the end position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::rfind(const void *data, const pos_type end, const count_type count) const
{
    lock_read lock(*this);
    return base_class::rfind(data, end, count);
}

/**
 * Get the begin position of records
 * @return the begin position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::beg_pos() const
{
    lock_read lock(*this);
    return base_class::beg_pos();
}

/**
 * Set the begin position of records
 * @param pos the begin position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::set_beg_pos(const pos_type pos)
{
    lock_write lock(*this);
    base_class::set_beg_pos(pos);
}

/**
 * Increment the begin position of records
 * @param count value of the increment
 * @return new begin position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::inc_beg_pos(const count_type count)
{
    lock_write lock(*this);
    return base_class::inc_beg_pos(count);
}

/**
 * Decrement the begin position of records
 * @param count value of the decrement
 * @return new begin position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::dec_beg_pos(const count_type count)
{
    lock_write lock(*this);
    return base_class::dec_beg_pos(count);
}

/**
 * Get the end position of records
 * @return the end position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::end_pos() const
{
    lock_read lock(*this);
    return base_class::end_pos();
}

/**
 * Set the end position of records
 * @param pos the end position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::set_end_pos(const pos_type pos)
{
    lock_write lock(*this);
    base_class::set_end_pos(pos);
}

/**
 * Increment the end position of records
 * @param count value of the increment
 * @return new end position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::inc_end_pos(const count_type count)
{
    lock_write lock(*this);
    return base_class::inc_end_pos(count);
}

/**
 * Decrement the end position of records
 * @param count value of the decrement
 * @return new end position of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::dec_end_pos(const count_type count)
{
    lock_write lock(*this);
    return base_class::dec_end_pos(count);
}

/**
 * Calculate the count of pages in the range [beg, end)
 * @param beg the begin position of the records
 * @param end the end position of the records
 * @return the count of pages in the range
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::distance(const pos_type beg, const pos_type end) const
{
    lock_read lock(*this);
    return base_class::distance(beg, end);
}

/**
 * Get the count of records
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::count() const
{
    lock_read lock(*this);
    return base_class::count();
}

/**
 * Set the count of records
 * @param count the count of records
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::set_count(const count_type count)
{
    lock_write lock(*this);
    base_class::set_count(count);
}

/**
 * Check the table is empty
 * @return the result of the checking
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline bool locked_table<Table, Source, Controlblock, Locker>::empty() const
{
    lock_read lock(*this);
    return base_class::empty();
}

/**
 * Get the position of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::front_pos() const
{
    lock_read lock(*this);
    return base_class::front_pos();
}

/**
 * Get the position of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline pos_type locked_table<Table, Source, Controlblock, Locker>::back_pos() const
{
    lock_read lock(*this);
    return base_class::back_pos();
}

/**
 * Clear the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::clear()
{
    lock_write lock(*this);
    base_class::clear();
}

/**
 * Get the locker of the table
 * @return the locker of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline typename locked_table<Table, Source, Controlblock, Locker>::locker_type&
    locked_table<Table, Source, Controlblock, Locker>::locker() const
{
    return m_locker;
}

/**
 * Lock the table for reading
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::lock_sharable() const
{
    table_lock<table_type, locker_type>::lock_sharable(*this);
}

/**
 * Lock the table for reading with a timeout
 * @param timeout the timeout
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::lock_sharable(const size_t timeout) const
{
    table_lock<table_type, locker_type>::lock_sharable(*this, timeout);
}

/**
 * Unlock the table for reading
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::unlock_sharable() const
{
    table_lock<table_type, locker_type>::unlock_sharable(*this);
}

/**
 * Lock the table for writing
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::lock_scoped() const
{
    table_lock<table_type, locker_type>::lock_scoped(*this);
}

/**
 * Lock the table for writing with a timeout
 * @param timeout the timeout
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::lock_scoped(const size_t timeout) const
{
    table_lock<table_type, locker_type>::lock_scoped(*this, timeout);
}

/**
 * Unlock the table for writing
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::unlock_scoped() const
{
    table_lock<table_type, locker_type>::unlock_scoped(*this);
}

/**
 * Get the count of the reading lock
 * @return the count of the reading lock
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::sharable_count() const
{
    return m_sharable_count;
}

/**
 * Get the count of the writing lock
 * @return the count of the writing lock
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline count_type locked_table<Table, Source, Controlblock, Locker>::scoped_count() const
{
    return m_scoped_count;
}

/**
 * Get the revision of modifying the table
 * @return the revision of modifying the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline revision_type locked_table<Table, Source, Controlblock, Locker>::revision() const
{
    lock_read lock(*this);
    return base_class::revision();
}

/**
 * Set the revision of modifying the table
 * @param rev the revision of modifying the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::set_revision(const revision_type rev)
{
    lock_write lock(*this);
    base_class::set_revision(rev);
}

/**
 * Increment the revision of modifying the table
 * @return new revision of modifying the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline revision_type locked_table<Table, Source, Controlblock, Locker>::inc_revision()
{
    lock_write lock(*this);
    return base_class::inc_revision();
}

/**
 * Check the table is relevant
 * @return the result of the checking
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline bool locked_table<Table, Source, Controlblock, Locker>::relevant() const
{
    lock_read lock(*this);
    return base_class::relevant();
}

/**
 * Refresh the metadata of the table by the key
 * @return the result of the checking
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline bool locked_table<Table, Source, Controlblock, Locker>::refresh()
{
    lock_read lock(*this);
    return base_class::refresh();
}

/**
 * Update the key by the metadata of the table
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::update()
{
    lock_write lock(*this);
    base_class::update();
}

/**
 * Recovery the metadata of the table by the key
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::recovery()
{
    lock_read lock(*this);
    base_class::recovery();
}

/**
 * Start the transaction
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::start()
{
    lock_read lock(*this);
    base_class::start();
}

/**
 * Stop the transaction
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::stop()
{
    lock_read lock(*this);
    base_class::stop();
}

/**
 * Cancel the transaction
 */
template <template <typename, typename, typename> class Table, typename Source, typename Controlblock, typename Locker>
inline void locked_table<Table, Source, Controlblock, Locker>::cancel()
{
    lock_read lock(*this);
    base_class::cancel();
}

//==============================================================================
//  locked_source
//==============================================================================
/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param file the file of the data source
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(file_type& file, const size_type rec_size, const options_type& options) :
    base_class(file, rec_size, options)
{
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param file the file of the data source
 * @param tbl_count the count of the tables
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(file_type& file, const count_type tbl_count,
        const size_type rec_size, const options_type& options) :
    base_class(file, tbl_count, rec_size, options)
{
}

/**
 * Constructor that is used when parameters of the source are known
 * @param file the file of the data source
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(file_type& file, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size, const options_type& options) :
    base_class(file, tbl_count, rec_count, rec_size, options)
{
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param name the file name of the data source
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(const std::string& name,
        const size_type rec_size, const options_type& options) :
    base_class(name, rec_size, options)
{
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param name the file name of the data source
 * @param tbl_count the count of the tables
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(const std::string& name,
        const count_type tbl_count, const size_type rec_size, const options_type& options) :
    base_class(name, tbl_count, rec_size, options)
{
}

/**
 * Constructor that is used when parameters of the source are known
 * @param name the file name of the data source
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename Interface>
inline locked_source<Interface>::locked_source(const std::string& name, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size, const options_type& options) :
    base_class(name, tbl_count, rec_count, rec_size, options)
{
}

/**
 * Set the exclusive lock for a table by an index
 * @param index the index of the table
 */
template <typename Interface>
inline void locked_source<Interface>::lock(const pos_type index) const
{
    base_class::file().lock(base_class::table_offset(index), base_class::table_size());
}

/**
 * Remove the exclusive lock for a table by an index
 * @param index the index of the table
 */
template <typename Interface>
inline void locked_source<Interface>::unlock(const pos_type index) const
{
    base_class::file().unlock(base_class::table_offset(index), base_class::table_size());
}

/**
 * Set the shared lock for a table by an index
 * @param index the index of the table
 */
template <typename Interface>
inline void locked_source<Interface>::lock_sharable(const pos_type index) const
{
    base_class::file().lock_sharable(base_class::table_offset(index), base_class::table_size());
}

/**
 * Remove the shared lock for a table by an index
 * @param index the index of the table
 */
template <typename Interface>
inline void locked_source<Interface>::unlock_sharable(const pos_type index) const
{
    base_class::file().unlock_sharable(base_class::table_offset(index), base_class::table_size());
}

}   //namespace ouroboros

#endif	/* OUROBOROS_LOCKEDTABLE_H */

