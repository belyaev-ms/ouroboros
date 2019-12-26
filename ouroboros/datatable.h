/**
 * @file   datatable.h
 * The table that has records of data
 */

#ifndef OUROBOROS_DATATABLE_H
#define	OUROBOROS_DATATABLE_H

#include <string>
#include <vector>

#include "ouroboros/global.h"
#include "ouroboros/lockedtable.h"
#include "ouroboros/record.h"
#include "ouroboros/scoped_buffer.h"

namespace ouroboros
{

/**
 * The reading cursor of a table
 */
template <typename Table>
class const_cursor_type : public base_cursor<Table>
{
    typedef base_cursor<Table> base_class;
public:
    typedef typename base_class::table_type table_type;
    typedef typename table_type::record_type record_type;

    const_cursor_type& operator++ ();
    const_cursor_type operator++ (int );
    const_cursor_type& operator-- ();
    const_cursor_type operator-- (int );
    const_cursor_type& operator>> (record_type& record);
};

/**
 * The writing cursor of a table
 */
template <typename Table>
class cursor_type : public const_cursor_type<Table>
{
    typedef base_cursor<Table> base_class;
public:
    typedef typename base_class::table_type table_type;
    typedef typename table_type::record_type record_type;

    cursor_type& operator<< (const record_type& record);
};

/**
 * The interface class adapter for the table that has the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Interface> class data_source;
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
class data_table : public interface_locked_table<Table, data_source<Table, Record, Interface>, Key, Interface>
{
    typedef interface_locked_table<Table, data_source<Table, Record, Interface>, Key, Interface> base_class;
    typedef data_table<Table, Record, Key, Interface> self_type;
    template <typename, typename, typename> friend class table_pnode;
    template <typename, typename> friend class node_cache;
    template <typename> friend class rbtree;
public:
    typedef typename base_class::unsafe_table unsafe_table; ///< the table that doesn't have locking
    typedef Interface interface_type;
    typedef Record record_type;
    typedef std::vector<record_type> record_list;
    typedef Key skey_type;
    typedef record_type raw_record_type;
    typedef data_source<Table, record_type, interface_type> source_type;
    typedef typename base_class::guard_type guard_type;
    typedef const_cursor_type<self_type> const_cursor;
    typedef cursor_type<self_type> cursor;

    data_table(source_type& source, skey_type& skey);
    data_table(source_type& source, skey_type& skey, const guard_type& guard);

    const_cursor begin() const; ///< get the begin reading cursor
    const_cursor end() const; ///< get the end reading cursor
    cursor begin(); ///< get the begin writing cursor
    cursor end(); ///< get the end writing cursor

    pos_type read(record_type& record, const pos_type pos) const; ///< read a record
    pos_type read(record_list& records, const pos_type pos) const; ///< read records [pos, pos + count)
    pos_type rread(record_type& record, const pos_type pos) const; ///< reverse read a record
    pos_type write(const record_type& record, const pos_type pos); ///< write a record
    pos_type write(const record_list& records, const pos_type pos); ///< write records [pos, pos + count)
    pos_type rwrite(const record_type& record, const pos_type pos); ///< reverse write a record
    pos_type add(const record_type& record); ///< add a record
    pos_type add(const record_list& records); ///< add records
    pos_type read_front(record_type& record) const; ///< read the first record
    pos_type read_front(record_list& records) const; ///< read the first records
    pos_type read_back(record_type& record) const; ///< read the last record
    pos_type read_back(record_list& records) const; ///< read the last records
    pos_type find(const record_type& record, const pos_type beg, const count_type count) const; ///< find a record [beg, beg + count)
    inline pos_type rfind(const record_type& record, const pos_type end, const count_type count) const; ///< reverse find a record [end - count, end)
    template <typename Finder>
    pos_type find(Finder& finder, const pos_type beg, const count_type count) const; ///< find a record [beg, end)
    template <typename Finder>
    pos_type rfind(Finder& finder, const pos_type end, const count_type count) const; ///< reverse find a record [beg, end)
protected:
    template <typename T>
    inline pos_type do_read(record_type& record, const pos_type pos) const; ///< read a record
    template <typename T>
    inline pos_type do_read(record_list& records, const pos_type pos) const; ///< read records [pos, pos + count)
    template <typename T>
    inline pos_type do_rread(record_type& record, const pos_type pos) const; ///< reverse read a record
    template <typename T>
    inline pos_type do_write(const record_type& record, const pos_type pos); ///< write a record
    template <typename T>
    inline pos_type do_write(const record_list& records, const pos_type pos); ///< write records [pos, pos + count)
    template <typename T>
    inline pos_type do_rwrite(const record_type& record, const pos_type pos); ///< reverse write a record
    template <typename T>
    inline pos_type do_add(const record_type& record); ///< add a record
    template <typename T>
    inline pos_type do_add(const record_list& records); ///< add records
    template <typename T>
    inline pos_type do_read_front(record_type& record) const; ///< read the first record
    template <typename T>
    inline pos_type do_read_front(record_list& records) const; ///< read the first records
    template <typename T>
    inline pos_type do_read_back(record_type& record) const; ///< read the last record
    template <typename T>
    inline pos_type do_read_back(record_list& records) const; ///< read the last records
    /* the methods don't use any locking */
    pos_type unsafe_read(record_type& record, const pos_type pos) const; ///< read a record
    pos_type unsafe_read(record_list& records, const pos_type pos) const; ///< read records [pos, pos + count)
    pos_type unsafe_rread(record_type& record, const pos_type pos) const; ///< reverse read a record
    pos_type unsafe_write(const record_type& record, const pos_type pos); ///< write a record
    pos_type unsafe_write(const record_list& records, const pos_type pos); ///< write records [pos, pos + count)
    pos_type unsafe_rwrite(const record_type& record, const pos_type pos); ///< reverse write a record
    pos_type unsafe_add(const record_type& record); ///< add a record
    pos_type unsafe_add(const record_list& records); ///< add records
    pos_type unsafe_read_front(record_type& record) const; ///< read the first record
    pos_type unsafe_read_front(record_list& records) const; ///< read the first records
    pos_type unsafe_read_back(record_type& record) const; ///< read the last record
    pos_type unsafe_read_back(record_list& records) const; ///< read the last records
protected:
    /** hide the parents methods */
    pos_type read(void *data, const pos_type pos) const; ///< read a record
    pos_type read(void *data, const pos_type beg, const count_type count) const; ///< read records [beg, beg + count)
    pos_type rread(void *data, const pos_type pos) const; ///< reverse read a record
    pos_type write(const void *data, const pos_type pos); ///< write a record
    pos_type write(const void *data, const pos_type beg, const count_type count); ///< write records [beg, beg + count)
    pos_type rwrite(const void *data, const pos_type pos); ///< reverse write a record
    pos_type add(const void *data); ///< add a record
    pos_type add(const void *data, const count_type count); ///< add records
    pos_type read_front(void *data) const; ///< read the first record
    pos_type read_back(void *data) const; ///< read the last record
    pos_type find(const void *data, const pos_type beg, const count_type count) const; ///< find a record [beg, beg + count)
    pos_type rfind(const void *data, const pos_type end, const count_type count) const; ///< reverse find a record [end - count, end)
private:
    scoped_buffer<void> m_buffer; ///< the buffer for a record
};

/**
 * The interface class for table source whose table has the data record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Interface>
class data_source : public locked_source<Interface>
{
    typedef locked_source<Interface> base_class;
public:
    typedef Interface interface_type;
    typedef typename interface_type::file_type file_type;
    typedef Record record_type;
    explicit data_source(file_type& file, const options_type& options = options_type()) :
        base_class(file, record_type().size(), options)
    {}
    data_source(file_type& file, const count_type tbl_count, const options_type& options = options_type()) :
        base_class(file, tbl_count, record_type().size(), options)
    {}
    data_source(file_type& file, const count_type tbl_count, const count_type rec_count, const options_type& options = options_type()) :
        base_class(file, tbl_count, rec_count, record_type().size(), options)
    {}
    explicit data_source(const std::string& name, const options_type& options = options_type()) :
        base_class(name, record_type().size(), options)
    {}
    data_source(const std::string& name, const count_type tbl_count, const options_type& options = options_type()) :
        base_class(name, tbl_count, record_type().size(), options)
    {}
    data_source(const std::string& name, const count_type tbl_count, const count_type rec_count, const options_type& options = options_type()) :
        base_class(name, tbl_count, rec_count, record_type().size(), options)
    {}
};

//==============================================================================
//  const_cursor_type
//==============================================================================
/**
 * Operator ++
 * @return the cursor to next position
 */
template <typename Table>
const_cursor_type<Table>&  const_cursor_type<Table>::operator++ ()
{
    base_class::pos(base_class::table().inc_pos(base_class::pos()));
    return *this;
}

/**
 * Operator ++
 * @return the cursor to current position
 */
template <typename Table>
const_cursor_type<Table> const_cursor_type<Table>::operator++ (int )
{
    const pos_type pos = base_class::pos();
    operator++();
    return const_cursor_type<Table>(base_class::table(), pos);
}

/**
 * Operator --
 * @return the cursor to previous position
 */
template <typename Table>
const_cursor_type<Table>& const_cursor_type<Table>::operator-- ()
{
    base_class::pos(base_class::table().dec_pos(base_class::pos()));
    return *this;
}

/**
 * Operator --
 * @return the cursor to current position
 */
template <typename Table>
const_cursor_type<Table> const_cursor_type<Table>::operator-- (int )
{
    const pos_type pos = base_class::pos();
    operator--();
    return const_cursor_type<Table>(base_class::table(), pos);
}

template <typename Table>
const_cursor_type<Table>& const_cursor_type<Table>::operator>> (record_type& record)
{
    base_class::table().read(record, base_class::pos());
}

//==============================================================================
//  cursor_type
//==============================================================================
template <typename Table>
cursor_type<Table>& cursor_type<Table>::operator<< (const record_type& record)
{
    base_class::table().write(record, base_class::pos());
}

//==============================================================================
//  data_table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
data_table<Table, Record, Key, Interface>::data_table(source_type& source, skey_type& skey) :
    base_class(source, skey),
    m_buffer(base_class::rec_size())
{
}

/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 * @param guard the guard of the table
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
data_table<Table, Record, Key, Interface>::data_table(source_type& source, skey_type& skey, const guard_type& guard) :
    base_class(source, skey, guard),
    m_buffer(base_class::rec_size())
{
}

/**
 * Get the begin reading cursor
 * @return the begin reading cursor
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
typename data_table<Table, Record, Key, Interface>::const_cursor
    data_table<Table, Record, Key, Interface>::begin() const
{
    return const_cursor(*this, base_class::beg_pos());
}

/**
 * Get the end reading cursor
 * @return the end reading cursor
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
typename data_table<Table, Record, Key, Interface>::const_cursor
    data_table<Table, Record, Key, Interface>::end() const
{
    return const_cursor(*this, base_class::end_pos());
}

/**
 * Get the begin writing cursor
 * @return the begin writing cursor
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
typename data_table<Table, Record, Key, Interface>::cursor
    data_table<Table, Record, Key, Interface>::begin()
{
    return cursor(*this, base_class::beg_pos());
}

/**
 * Get the end writing cursor
 * @return the end writing cursor
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
typename data_table<Table, Record, Key, Interface>::cursor
    data_table<Table, Record, Key, Interface>::end()
{
    return cursor(*this, base_class::end_pos());
}

/**
 * Read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read(record_type& record, const pos_type pos) const
{
    return do_read<base_class>(record, pos);
}

/**
 * Read records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read(record_list& records, const pos_type pos) const
{
    return do_read<base_class>(records, pos);
}

/**
 * Reverse read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::rread(record_type& record, const pos_type pos) const
{
    return do_rread<base_class>(record, pos);
}

/**
 * Write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::write(const record_type& record, const pos_type pos)
{
    return do_write<base_class>(record, pos);
}

/**
 * Write records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::write(const record_list& records, const pos_type pos)
{
    return do_write<base_class>(records, pos);
}

/**
 * Reverse write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::rwrite(const record_type& record, const pos_type pos)
{
    return do_rwrite<base_class>(record, pos);
}

/**
 * Add a record
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::add(const record_type& record)
{
    return do_add<base_class>(record);
}

/**
 * Add records
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::add(const record_list& records)
{
    return do_add<base_class>(records);
}

/**
 * Read the first record
 * @param record data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read_front(record_type& record) const
{
    return do_read_front<base_class>(record);
}

/**
 * Read the first records
 * @param records data of the first records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read_front(record_list& records) const
{
    return do_read_front<base_class>(records);
}

/**
 * Read the last record
 * @param record data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read_back(record_type& record) const
{
    return do_read_back<base_class>(record);
}

/**
 * Read the last records
 * @param records data of the last records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::read_back(record_list& records) const
{
    return do_read_back<base_class>(records);
}

/**
 * Find a record in the range [beg, beg + count)
 * @param record data of the record
 * @param beg the begin position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::find(const record_type& record, const pos_type beg, const count_type count) const
{
    record.pack(m_buffer.get());
    return base_class::find(m_buffer.get(), beg, count);
}

/**
 * Reverse find a record in the range [end - count, end)
 * @param record data of the record
 * @param end the end position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::rfind(const record_type& record, const pos_type end, const count_type count) const
{
    record.pack(m_buffer.get());
    return base_class::rfind(m_buffer.get(), end, count);
}

/**
 * Find a record in the range [beg, beg + count)
 * @param finder the finder
 * @param beg the begin position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
// cppcheck-suppress syntaxError
template <typename Finder>
pos_type data_table<Table, Record, Key, Interface>::find(Finder& finder, const pos_type beg, const count_type count) const
{
    typename base_class::lock_read lock(*this);
    if (!unsafe_table::empty())
    {
        pos_type pos = beg;
        for (count_type i = 0; i < count; ++i)
        {
            const pos_type result = pos;
            pos = unsafe_table::read(m_buffer.get(), result);
            finder.record(pos).unpack(m_buffer.get());
            if (!finder())
            {
                return result;
            }
        }
    }
    return NIL;
}

/**
 * Reverse find a record in the range [end - count, end)
 * @param finder the finder of records
 * @param end the end position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename Finder>
pos_type data_table<Table, Record, Key, Interface>::rfind(Finder& finder, const pos_type end, const count_type count) const
{
    typename base_class::lock_read lock(*this);
    if (!unsafe_table::empty())
    {
        pos_type pos = unsafe_table::dec_pos(end);
        for (count_type i = 0; i < count; ++i)
        {
            const pos_type result = pos;
            pos = unsafe_table::rread(m_buffer.get(), result);
            finder.record(pos).unpack(m_buffer.get());
            if (!finder())
            {
                return result;
            }
        }
    }
    return NIL;
}

/**
 * Read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_read(record_type& record, const pos_type pos) const
{
    const pos_type result = T::read(m_buffer.get(), pos);
    record.unpack(m_buffer.get());
    return result;
}

/**
 * Reverse read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_rread(record_type& record, const pos_type pos) const
{
    const pos_type result = T::rread(m_buffer.get(), pos);
    record.unpack(m_buffer.get());
    return result;
}

/**
 * Read records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_read(record_list& records, const pos_type pos) const
{
    const count_type count = records.size();
    scoped_buffer<void> buffer(unsafe_table::rec_size() * count);
    const pos_type result = T::read(buffer.get(), pos, count);
    const void *it = buffer.get();
    ///@todo replace to iterator
    for (count_type i = 0; i < count; ++i)
    {
        it = records[i].unpack(it);
    }
    return result;
}

/**
 * Write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_write(const record_type& record, const pos_type pos)
{
    record.pack(m_buffer.get());
    return T::write(m_buffer.get(), pos);
}

/**
 * Reverse write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_rwrite(const record_type& record, const pos_type pos)
{
    record.pack(m_buffer.get());
    return T::rwrite(m_buffer.get(), pos);
}

/**
 * Write records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_write(const record_list& records, const pos_type pos)
{
    const count_type count = records.size();
    scoped_buffer<void> buffer(unsafe_table::rec_size() * count);
    void *it = buffer.get();
    for (count_type i = 0; i < count; ++i)
    {
        it = records[i].pack(it);
    }
    return T::write(buffer.get(), pos, count);
}

/**
 * Add a record
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_add(const record_type& record)
{
    record.pack(m_buffer.get());
    return T::add(m_buffer.get());
}

/**
 * Add records
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_add(const record_list& records)
{
    const count_type count = records.size();
    scoped_buffer<void> buffer(base_class::rec_size() * count);
    void *it = buffer.get();
    for (pos_type i = 0; i < count; ++i)
    {
        it = records[i].pack(it);
    }
    return T::add(buffer.get(), count);
}

/**
 * Read the first record
 * @param record data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
pos_type data_table<Table, Record, Key, Interface>::do_read_front(record_type& record) const
{
    const pos_type pos = T::read_front(m_buffer.get());
    if (pos != NIL)
    {
        record.unpack(m_buffer.get());
    }
    return pos;
}

/**
 * Read the first records
 * @param records data of the first records
 * @return the position of the next records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_read_front(record_list& records) const
{
   const count_type count = records.size();
    scoped_buffer<void> buffer(unsafe_table::rec_size() * count);
    const pos_type result = T::read_front(buffer.get(), count);
    if (result != NIL)
    {
        const void *it = buffer.get();
        for (count_type i = 0; i < count; ++i)
        {
            it = records[i].unpack(it);
        }
    }
    return result;
}

/**
 * Read the last record
 * @param record data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
pos_type data_table<Table, Record, Key, Interface>::do_read_back(record_type& record) const
{
    const pos_type pos = T::read_back(m_buffer.get());
    if (pos != NIL)
    {
        record.unpack(m_buffer.get());
    }
    return pos;
}

/**
 * Read the last records
 * @param records data of the last records
 * @return the position of the next records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
template <typename T>
inline pos_type data_table<Table, Record, Key, Interface>::do_read_back(record_list& records) const
{
   const count_type count = records.size();
    scoped_buffer<void> buffer(unsafe_table::rec_size() * count);
    const pos_type result = T::read_back(buffer.get(), count);
    if (result != NIL)
    {
        const void *it = buffer.get();
        for (count_type i = 0; i < count; ++i)
        {
            it = records[i].unpack(it);
        }
    }
    return result;
}

/**
 * Read a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read(record_type& record, const pos_type pos) const
{
    return do_read<unsafe_table>(record, pos);
}

/**
 * Reverse read a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_rread(record_type& record, const pos_type pos) const
{
    return do_rread<unsafe_table>(record, pos);
}

/**
 * Read records (without any locking)
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read(record_list& records, const pos_type pos) const
{
    return do_read<unsafe_table>(records, pos);
}

/**
 * Write a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_write(const record_type& record, const pos_type pos)
{
    return do_write<unsafe_table>(record, pos);
}

/**
 * Reverse write a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_rwrite(const record_type& record, const pos_type pos)
{
    return do_rwrite<unsafe_table>(record, pos);
}

/**
 * Write records (without any locking)
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_write(const record_list& records, const pos_type pos)
{
    return do_write<unsafe_table>(records, pos);
}

/**
 * Add a record (without any locking)
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_add(const record_type& record)
{
    return do_add<unsafe_table>(record);
}

/**
 * Add records (without any locking)
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_add(const record_list& records)
{
    return do_add<unsafe_table>(records);
}

/**
 * Read the first record (without any locking)
 * @param record data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read_front(record_type& record) const
{
    return do_read_front<unsafe_table>(record);
}

/**
 * Read the first records (without any locking)
 * @param record data of the first records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read_front(record_list& records) const
{
    return do_read_front<unsafe_table>(records);
}

/**
 * Read the last record (without any locking)
 * @param record data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read_back(record_type& record) const
{
    return do_read_back<unsafe_table>(record);
}

/**
 * Read the last records (without any locking)
 * @param records data of the last records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
pos_type data_table<Table, Record, Key, Interface>::unsafe_read_back(record_list& records) const
{
    return do_read_back<unsafe_table>(records);
}

}   //namespace ouroboros

#endif	/* OUROBOROS_DATATABLE_H */

