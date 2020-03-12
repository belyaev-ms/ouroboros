/**
 * @file   indexedtable.h
 * The table that has indexed data records
 */

#ifndef OUROBOROS_INDEXEDTABLE_H
#define	OUROBOROS_INDEXEDTABLE_H

#include <string>
#include <vector>

#include "ouroboros/global.h"
#include "ouroboros/table.h"
#include "ouroboros/record.h"
#include "ouroboros/index.h"
#include "ouroboros/container.h"
#include "ouroboros/scoped_buffer.h"
#include "ouroboros/datatable.h"

namespace ouroboros
{

/**
 * The interface class adapter for the table that has the indexed records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
class indexed_table : public data_table<Table, Record, Key, Interface>
{
    typedef data_table<Table, Record, Key, Interface> base_class;
public:
    enum { TABLE_TYPE = TABLE_INDEX };
    typedef typename base_class::unsafe_table unsafe_table; ///< the table that doesn't have locking
    typedef typename base_class::interface_type interface_type;
    typedef typename base_class::record_type record_type;
    typedef typename base_class::record_list record_list;
    typedef typename base_class::skey_type skey_type;
    typedef record_type raw_record_type;
    typedef Index<record_type> index_type;
    typedef typename index_type::field_type field_type;
    typedef typename base_class::source_type source_type;
    typedef typename base_class::guard_type guard_type;
    typedef std::vector<pos_type> pos_list;

    indexed_table(source_type& source, skey_type& skey);
    indexed_table(source_type& source, skey_type& skey, const guard_type& guard);

    inline pos_type read(record_type& record, const pos_type pos) const; ///< read a record
    inline pos_type read(record_list& records, const pos_type pos) const; ///< read records [pos, pos + count)
    inline pos_type rread(record_type& record, const pos_type pos) const; ///< reverse read a record
    pos_type write(const record_type& record, const pos_type pos); ///< write a record
    pos_type write(const record_list& records, const pos_type pos); ///< write records [pos, pos + count)
    pos_type rwrite(const record_type& record, const pos_type pos); ///< reverse write a record
    pos_type add(const record_type& record); ///< add a record
    pos_type add(const record_list& records); ///< add records

    count_type remove_by_index(const field_type& beg, const field_type& end); ///< delete records by index [beg, end)

    pos_type read_front_by_index(record_type& record, const field_type& beg, const field_type& end) const; ///< read the first record by index
    pos_type read_back_by_index(record_type& record, const field_type& beg, const field_type& end) const; ///< read the last record by index

    count_type read_index(pos_list& dest, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< read indexes of the records
    count_type rread_index(pos_list& dest, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< reverse read indexes of the records
    count_type read_by_index(record_list& records, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< read records by index [beg, end)
    count_type rread_by_index(record_list& records, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< reverse read records by index [beg, end)
    count_type read(record_list& records, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< read records that have an index in range [beg, end)
    count_type rread(record_list& records, const field_type& beg, const field_type& end, const count_type size = 0) const; ///< reverse read records that have an index in range [beg, end)

    pos_type get(const field_type& field, record_type& record) const; ///< get a record that has an index

    template <typename Finder>
    pos_type find_by_index(Finder& finder, const field_type& beg, const field_type& end) const; ///< find a record by index [beg, end)
    template <typename Finder>
    pos_type rfind_by_index(Finder& finder, const field_type& beg, const field_type& end) const; ///< reverse find a record by index [beg, end)
    template <typename Finder>
    pos_type find(Finder& finder, const field_type& beg, const field_type& end) const; ///< find a record that has index in range [beg, end)
    template <typename Finder>
    pos_type rfind(Finder& finder, const field_type& beg, const field_type& end) const; ///< reverse find a record that has index in range [beg, end)

    inline void build_indexes(); ///< build the indexes of the records
protected:
    inline void add_index(const record_type& record, const pos_type pos); ///< add the index of the record
    inline void remove_index(const record_type& record, const pos_type pos); ///< delete the index of the record
    virtual void do_before_remove(const pos_type pos); ///< perform an action before deleting record
    virtual void do_before_move(const pos_type source, const pos_type dest); ///< perform an action before moving record
    virtual void do_clear(); ///< clear the table
    inline void do_build_indexes(const pos_type beg, const pos_type end); ///< build the indexes of the records
    void do_get_pos_list(pos_list& dest, const field_type& beg, const field_type& end) const; ///< get positions of the records that have an index in range [beg, end)
    /* the methods don't use any locking */
    inline pos_type unsafe_write(const record_type& record, const pos_type pos); ///< write a record
    inline pos_type unsafe_add(const record_type& record); ///< add a record
private:
    typedef std::multimap<field_type, pos_type> index_list;
    index_list m_indexes;
};

/**
 * The interface class adapter for the table that doesn't have any indexed records
 */
template <template <typename, typename, typename> class Table, typename Record, typename Key, typename Interface>
class indexed_table<Table, Record, index_null, Key, Interface> : public data_table<Table, Record, Key, Interface>
{
    typedef data_table<Table, Record, Key, Interface> base_class;
public:
    typedef typename base_class::unsafe_table unsafe_table;
    typedef Record record_type;
    typedef Key skey_type;
    typedef typename base_class::source_type source_type;
    typedef typename base_class::guard_type guard_type;
    indexed_table(source_type& source, skey_type& key) : base_class(source, key) {}
    indexed_table(source_type& source, skey_type& key, const guard_type& guard) : base_class(source, key, guard) {}
    inline void build_indexes() {}
};

//==============================================================================
//  indexed_table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
indexed_table<Table, Record, Index, Key, Interface>::indexed_table(source_type& source,
        skey_type& skey) :
    base_class(source, skey)
{
    if (0 == skey.count)
    {
        base_class::clear();
    }
    else
    {
        build_indexes();
    }
}

/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 * @param guard the guard of the table
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
indexed_table<Table, Record, Index, Key, Interface>::indexed_table(source_type& source,
        skey_type& skey, const guard_type& guard) :
    base_class(source, skey, guard)
{
    if (0 == skey.count)
    {
        base_class::clear();
    }
    else
    {
        build_indexes();
    }
}

/**
 * Read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::read(record_type& record, const pos_type pos) const
{
    return base_class::read(record, pos);
}

/**
 * Read records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::read(record_list& records, const pos_type pos) const
{
    return base_class::read(records, pos);
}

/**
 * Reverse read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::rread(record_type& record, const pos_type pos) const
{
    return base_class::rread(record, pos);
}

/**
 * Write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::unsafe_write(const record_type& record, const pos_type pos)
{
    record_type replaced_record;
    base_class::unsafe_read(replaced_record, pos);
    remove_index(replaced_record, pos);
    const pos_type num = base_class::unsafe_write(record, pos);
    add_index(record, pos);
    return num;
}

/**
 * Write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::write(const record_type& record, const pos_type pos)
{
    typename base_class::lock_write lock(*this);
    return unsafe_write(record, pos);
}

/**
 * Write records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::write(const record_list& records, const pos_type pos)
{
    typename base_class::lock_write lock(*this);
    pos_type num = pos;
    const typename record_list::const_iterator itend = records.end();
    for (typename record_list::const_iterator it = records.begin(); it != itend; ++it)
    {
        num = unsafe_write(*it, num);
    }
    return num;
}

/**
 * Add a record
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::unsafe_add(const record_type& record)
{
    const pos_type beg = unsafe_table::beg_pos();
    const pos_type end = unsafe_table::end_pos();
    ///@todo repace count == MaxCount
    if (end == beg && !unsafe_table::empty())
    {
        record_type replaced_record;
        base_class::unsafe_read(replaced_record, beg);
        remove_index(replaced_record, beg);
    }
    const pos_type pos = base_class::unsafe_add(record);
    add_index(record, end);
    return pos;
}

/**
 * Add a record
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::add(const record_type& record)
{
    typename base_class::lock_write lock(*this);
    return unsafe_add(record);
}

/**
 * Add records
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::add(const record_list& records)
{
    typename base_class::lock_write lock(*this);
    pos_type pos = unsafe_table::end_pos();
    const typename record_list::const_iterator end = records.end();
    for (typename record_list::const_iterator it = records.begin(); it != end; ++it)
    {
        pos = unsafe_add(*it);
    }
    return pos;
}

/**
 * Perform an action before removing record
 * @param pos the position of record to be deleted
 */
//virtual
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
void indexed_table<Table, Record, Index, Key, Interface>::do_before_remove(const pos_type pos)
{
    record_type record;
    base_class::unsafe_read(record, pos);
    remove_index(record, pos);
}

/**
 * Perform an action before moving record
 * @param source the position of record to be moved
 * @param dest the position of record to be deleted
 */
//virtual
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
void indexed_table<Table, Record, Index, Key, Interface>::do_before_move(const pos_type source, const pos_type dest)
{
    record_type record;
    base_class::unsafe_read(record, source);
    remove_index(record, source);
    add_index(record, dest);
}

/**
 * Delete records by index [beg, end)
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the count of the deleted records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::remove_by_index(const field_type& beg, const field_type& end)
{
    typename base_class::lock_write lock(*this);
    const count_type count = unsafe_table::count();
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    // delete records in parts from the end
    const pos_list::reverse_iterator itend = list.rend();
    for (pos_list::reverse_iterator it = list.rbegin(); it != itend; ++it)
    {
        pos_type pbeg = *it % count;
        count_type pcount = 1;
        for (pos_list::reverse_iterator ait = it + 1 ; ait != itend; ++ait)
        {
            const pos_type pos = *ait % count;
            if (unsafe_table::distance(pos, pbeg) > 1)
            {
                break;
            }
            pbeg = pos;
            it = ait;
            ++pcount;
        }
        unsafe_table::remove(pbeg, pcount);
    }
    return count;
}

/**
 * Add the index of the record
 * @param record data of the record
 * @param pos the position of the record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
inline void indexed_table<Table, Record, Index, Key, Interface>::add_index(const record_type& record, const pos_type pos)
{
    m_indexes.insert(std::make_pair(index_type::value(record), pos));
}

/**
 * Delete the index of the record
 * @param record data of the record
 * @param pos the position of the record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
inline void indexed_table<Table, Record, Index, Key, Interface>::remove_index(const record_type& record, const pos_type pos)
{
    const std::pair<typename index_list::iterator, typename index_list::iterator> range =
        m_indexes.equal_range(index_type::value(record));
    for (typename index_list::iterator it = range.first; it != range.second; ++it)
    {
        if (it->second == pos)
        {
            m_indexes.erase(it);
            return;
        }
    }
    OUROBOROS_THROW_BUG("the index not found " << PR(pos) << PE(record));
}

/**
 * Read the first record by index
 * @param record data of the record
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::read_front_by_index(record_type& record, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    if (list.empty())
    {
        return NIL;
    }
    else
    {
        pos_list::const_iterator it = std::min_element(list.begin(), list.end());
        const pos_type pos = *it % unsafe_table::count();
        base_class::unsafe_read(record, pos);
        return pos;
    }
}

/**
 * Read the last record by index
 * @param record data of the record
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::read_back_by_index(record_type& record, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    if (list.empty())
    {
        return NIL;
    }
    else
    {
        pos_list::const_iterator it = std::max_element(list.begin(), list.end());
        const pos_type pos = *it % unsafe_table::count();
        base_class::unsafe_read(record, pos);
        return pos;
    }
}

/**
 * Get positions of the records that have an index in range [beg, end)
 * @param dest the destination of positions of records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
void indexed_table<Table, Record, Index, Key, Interface>::do_get_pos_list(pos_list& dest, const field_type& beg, const field_type& end) const
{
    typename index_list::const_iterator itbeg = m_indexes.lower_bound(beg);
    typename index_list::const_iterator itend = m_indexes.upper_bound(end);

    const pos_type beg_pos = unsafe_table::beg_pos();
    const pos_type end_pos = unsafe_table::end_pos();
    const count_type count = unsafe_table::count();

    ///@todo doesn't work for unordered table
    if (beg_pos < end_pos)
    {
        for (typename index_list::const_iterator it = itbeg; it != itend; ++it)
        {
            const pos_type pos = it->second;
            dest.push_back(pos);
        }
    }
    else
    {
        for (typename index_list::const_iterator it = itbeg; it != itend; ++it)
        {
            const pos_type pos = it->second >= beg_pos ? it->second : it->second + count;
            dest.push_back(pos);
        }
    }
}

/**
 * Read records that have an index in range [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::read(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    records.resize(read_index(list, beg, end, size));
    pos_list::const_iterator pos = list.begin();
    const typename record_list::iterator itend = records.end();
    for (typename record_list::iterator it = records.begin(); it != itend; ++it)
    {
        base_class::unsafe_read(*it, *pos++);
    }
    return records.size();
}

/**
 * Reverse read records that have an index in range [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::rread(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    ///@todo why is size here???
    typename base_class::lock_read lock(*this);
    pos_list list;
    records.resize(rread_index(list, beg, end, size));
    pos_list::const_reverse_iterator pos = list.rbegin();
    const typename record_list::iterator itend = records.end();
    for (typename record_list::iterator it = records.begin(); it != itend; ++it)
    {
        base_class::unsafe_read(*it, *pos++);
    }
    return records.size();
}

/**
 * Get a record that has an index
 * @param field the value of index
 * @param record the record that has an index
 * @return the position of the record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
pos_type indexed_table<Table, Record, Index, Key, Interface>::get(const field_type& field,
    record_type& record) const
{
    typename base_class::lock_read lock(*this);
    const typename index_list::const_iterator it = m_indexes.find(field);
    if (it != m_indexes.end())
    {
        base_class::unsafe_read(record, it->second);
        return it->second;
    }
    return NIL;
}

/**
 * Read indexes of the records
 * @param dest the destination of positions of records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of indexes
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::read_index(pos_list& dest,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    do_get_pos_list(dest, beg, end);
    std::sort(dest.begin(), dest.end());
    if (size != 0 && size < dest.size())
    {
        dest.resize(size);
    }
    const count_type count = unsafe_table::rec_count();
    std::transform(dest.begin(), dest.end(), dest.begin(), std::bind2nd(std::modulus<pos_type>(), count));
    return dest.size();
}

/**
 * Reverse read indexes of the records
 * @param dest the destination of positions of records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of indexes
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::rread_index(pos_list& dest,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    do_get_pos_list(dest, beg, end);
    std::sort(dest.begin(), dest.end(), std::greater<pos_type>());
    if (size != 0 && size < dest.size())
    {
        dest.resize(size);
    }
    const count_type count = unsafe_table::rec_count();
    std::transform(dest.begin(), dest.end(), dest.begin(), std::bind2nd(std::modulus<pos_type>(), count));
    return dest.size();
}

/**
 * Read records by index [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::read_by_index(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    typename index_list::const_iterator itbeg = m_indexes.lower_bound(beg);
    typename index_list::const_iterator itend = m_indexes.upper_bound(end);
    count_type count = 0;
    for (typename index_list::const_iterator it = itbeg; it != itend; ++it)
    {
        record_type record;
        base_class::unsafe_read(record, it->second);
        records.push_back(record);
        if (++count == size)
        {
            break;
        }
    }
    return count;
}

/**
 * Reverse read records by index [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
count_type indexed_table<Table, Record, Index, Key, Interface>::rread_by_index(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    typename index_list::const_iterator itbeg = m_indexes.lower_bound(beg);
    typename index_list::const_iterator itend = m_indexes.upper_bound(end);
    typename index_list::const_reverse_iterator ritbeg(itend);
    typename index_list::const_reverse_iterator ritend(itbeg);
    count_type count = 0;
    for (typename index_list::const_reverse_iterator it = ritbeg; it != ritend; ++it)
    {
        record_type record;
        base_class::unsafe_read(record, it->second);
        records.push_back(record);
        if (++count == size)
        {
            break;
        }
    }
    return count;
}

/**
 * Find a record by index [beg, end)
 * @param finder the finder
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
template <typename Finder>
pos_type indexed_table<Table, Record, Index, Key, Interface>::
    find_by_index(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    typename index_list::const_iterator itbeg = m_indexes.lower_bound(beg);
    typename index_list::const_iterator itend = m_indexes.upper_bound(end);
    for (typename index_list::const_iterator it = itbeg; it != itend; ++it)
    {
        const pos_type pos = it->second;
        base_class::unsafe_read(finder.record(pos), pos);
        if (!finder())
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Reverse find a record by index [beg, end)
 * @param finder the finder
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
template <typename Finder>
pos_type indexed_table<Table, Record, Index, Key, Interface>::
    rfind_by_index(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    typename index_list::const_iterator itbeg = m_indexes.lower_bound(beg);
    typename index_list::const_iterator itend = m_indexes.upper_bound(end);
    typename index_list::const_reverse_iterator ritbeg(itend);
    typename index_list::const_reverse_iterator ritend(itbeg);
    for (typename index_list::const_reverse_iterator it = ritbeg; it != ritend; ++it)
    {
        const pos_type pos = it->second;
        base_class::unsafe_read(finder.record(pos), pos);
        if (!finder())
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Find a record that has index in range [beg, end)
 * @param finder the finder
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
template <typename Finder>
pos_type indexed_table<Table, Record, Index, Key, Interface>::
    find(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    const count_type count = unsafe_table::count();
    const pos_list::const_iterator itend = list.end();
    for (pos_list::const_iterator it = list.begin(); it != itend; ++it)
    {
        const pos_type pos = *it % count;
        base_class::unsafe_read(finder.record(pos), pos);
        if (!finder())
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Reverse find a record that has index in range [beg, end)
 * @param finder the finder
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
template <typename Finder>
pos_type indexed_table<Table, Record, Index, Key, Interface>::
    rfind(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    const count_type count = unsafe_table::count();
    const pos_list::reverse_iterator itend = list.rend();
    for (pos_list::reverse_iterator it = list.rbegin(); it != itend; ++it)
    {
        const pos_type pos = *it % count;
        base_class::unsafe_read(finder.record(pos), pos);
        if (!finder())
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Clear the table
 */
//virtual
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
void indexed_table<Table, Record, Index, Key, Interface>::do_clear()
{
    base_class::do_clear();
    m_indexes.clear();
}

/**
 * Build the indexes of the records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
inline void indexed_table<Table, Record, Index, Key, Interface>::build_indexes()
{
    typename base_class::lock_write lock(*this);
    m_indexes.clear();
    if (unsafe_table::empty())
    {
        return;
    }
    ///@todo doesn't work for unordered table
    const count_type count = unsafe_table::count();
    const pos_type beg = unsafe_table::beg_pos();
    const pos_type end = unsafe_table::end_pos();
    if (end > beg)
    {
        do_build_indexes(beg, end);
    }
    else
    {
        do_build_indexes(beg, count);
        do_build_indexes(0, end);
    }
}

/**
 * Build the indexes of the records
 * @param beg the begin position of the records
 * @param end the end position of the records
 */
template <template <typename, typename, typename> class Table, typename Record,
        template <typename> class Index, typename Key, typename Interface>
inline void indexed_table<Table, Record, Index, Key, Interface>::do_build_indexes(const pos_type beg, const pos_type end)
{
    for (pos_type pos = beg; pos < end; ++pos)
    {
        record_type record;
        base_class::unsafe_read(record, pos);
        add_index(record, pos);
    }
}

}   //namespace ouroboros

#endif	/* OUROBOROS_INDEXEDTABLE_H */

