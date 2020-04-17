/**
 * @file   treedatatable.h
 * The table that has indexed records by red-black tree
 */

#ifndef OUROBOROS_TREEDATATABLE_H
#define	OUROBOROS_TREEDATATABLE_H

#include "ouroboros/rbtree.h"
#include "ouroboros/indexedrecord.h"
#include "ouroboros/datatable.h"

namespace ouroboros
{

/**
 * The interface class adapter for the table that has indexed records by
 * red-black tree
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
class tree_data_table : public data_table<Table, IndexedRecord, Key, Interface>
{
    typedef data_table<Table, IndexedRecord, Key, Interface> base_class;
    typedef IndexedRecord indexed_record_type;
    typedef typename indexed_record_type::node_type node_type;
    struct extractor
    {
        inline node_type node(const indexed_record_type& record) const
        {
            return record();
        }
    };
public:
    enum
    {
        TABLE_TYPE = TABLE_TREE,
        REC_SPACE = base_class::REC_SPACE
    };
    typedef typename base_class::unsafe_table unsafe_table; ///< the table that doesn't have locking
    typedef Interface interface_type;
    typedef typename indexed_record_type::record_type record_type;
    typedef std::vector<record_type> record_list;
    typedef typename base_class::skey_type skey_type;
    typedef indexed_record_type raw_record_type;
    typedef std::vector<indexed_record_type> indexed_record_list;
    typedef typename indexed_record_type::index_type index_type;
    typedef typename index_type::field_type field_type;
    typedef typename base_class::source_type source_type;
    typedef typename base_class::guard_type guard_type;
    typedef std::vector<pos_type> pos_list;

    tree_data_table(source_type& source, skey_type& skey);
    tree_data_table(source_type& source, skey_type& skey, const guard_type& guard);

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
    pos_type find_in_range(Finder& finder, const field_type& beg, const field_type& end) const; ///< find a record that has index in range [beg, end)
    template <typename Finder>
    pos_type rfind_in_range(Finder& finder, const field_type& beg, const field_type& end) const; ///< reverse find a record that has index in range [beg, end)
    template <typename Finder>
    pos_type find(Finder& finder, const pos_type beg, const count_type count) const; ///< find a record [beg, end)
    template <typename Finder>
    pos_type rfind(Finder& finder, const pos_type end, const count_type count) const; ///< reverse find a record [beg, end)
    count_type get_range_size(const field_type& beg, const field_type& end) const; ///< get a count of records that have index in range [beg, end)

    inline bool refresh(); ///< refresh the metadata of the table by the key
    inline void update(); ///< update the key by the metadata of the table
    inline void recovery(); ///< recovery the metadata of the table by the key

    void clear(); ///< clear the table

#ifdef OUROBOROS_TEST_TOOLS_ENABLED
    void test() const; ///< test the table
    pos_type get_root() const; ///< get the position of the root
#endif
protected:
    virtual void do_before_remove(const pos_type pos); ///< perform an action before deleting record
    virtual void do_before_move(const pos_type source, const pos_type dest); ///< perform an action before moving record
    void do_get_pos_list(pos_list& dest, const field_type& beg, const field_type& end) const; ///< get positions of the records that have an index in range [beg, end)
    virtual void do_clear(); ///< clear the table
    /* the methods don't use any locking */
    inline pos_type unsafe_read(record_type& record, const pos_type pos) const; ///< read a record
    inline pos_type unsafe_read(record_list& records, const pos_type pos) const; ///< read records [pos, pos + count)
    inline pos_type unsafe_rread(record_type& record, const pos_type pos) const; ///< reverse read a record
    inline pos_type unsafe_write(const record_type& record, const pos_type pos); ///< write a record
    inline pos_type unsafe_write(const record_list& records, const pos_type pos); ///< write records [pos, pos + count)
    inline pos_type unsafe_rwrite(const record_type& record, const pos_type pos); ///< reverse write a record
    inline pos_type do_add(const record_type& record); ///< add a record
    inline pos_type unsafe_add(const record_type& record); ///< add a record
    inline pos_type unsafe_add(const record_list& records); ///< add records
    inline pos_type unsafe_read_front(record_type& record) const; ///< read the first record
    inline pos_type unsafe_read_front(record_list& records) const; ///< read the first records
    inline pos_type unsafe_read_back(record_type& record) const; ///< read the last record
    inline pos_type unsafe_read_back(record_list& records) const; ///< read the last records
protected:
    /** hide the parents methods */
    pos_type read(indexed_record_type& record, const pos_type pos) const;
    pos_type read(indexed_record_list& records, const pos_type pos) const;
    pos_type rread(indexed_record_type& record, const pos_type pos) const;
    pos_type write(const indexed_record_type& record, const pos_type pos);
    pos_type write(const indexed_record_list& records, const pos_type pos);
    pos_type rwrite(const indexed_record_type& record, const pos_type pos);
    pos_type add(const indexed_record_type& record);
    pos_type add(const indexed_record_list& records);
    pos_type read_front(indexed_record_type& record) const;
    pos_type read_front(indexed_record_list& records) const;
    pos_type read_back(indexed_record_type& record) const;
    pos_type read_back(indexed_record_list& records) const;
    pos_type read_front_by_index(indexed_record_type& record, const field_type& beg, const field_type& end) const;
    pos_type read_back_by_index(indexed_record_type& record, const field_type& beg, const field_type& end) const;
private:
#ifdef OUROBOROS_FASTRBTREE_ENABLED
    typedef node_cache<node_type, base_class> cache_type;
    typedef table_smart_pnode<node_type, base_class, extractor> pnode_type;
    typedef fast_rbtree<pnode_type> tree_type;
#else
    typedef table_pnode<node_type, base_class, extractor> pnode_type;
    typedef rbtree<pnode_type> tree_type;
#endif
    tree_type m_tree; ///< the tree of the indexed records
};

/**
 * The interface class adapter for tree_data_table, which will be used as
 * indexed_table
 */
template <template <typename, typename, typename> class Table, typename Record,
    template <typename> class Index, typename Key, typename Interface>
class tree_data_table_adapter : public tree_data_table<Table, indexed_record<Record, Index>, Key, Interface>
{
public:
    typedef Record record_type;
    typedef indexed_record<Record, Index> indexed_record_type;
private:
    typedef tree_data_table<Table, indexed_record_type, Key, Interface> base_class;
public:
    typedef typename base_class::unsafe_table unsafe_table;
    typedef typename base_class::skey_type skey_type;
    typedef typename base_class::source_type source_type;
    typedef typename base_class::guard_type guard_type;
    tree_data_table_adapter(source_type& source, skey_type& key) : base_class(source, key) {}
    tree_data_table_adapter(source_type& source, skey_type& key, const guard_type& guard) : base_class(source, key, guard) {}
    inline void build_indexes() {}
};

//==============================================================================
//  tree_data_table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
tree_data_table<Table, IndexedRecord, Key, Interface>::tree_data_table(source_type& source, skey_type& skey) :
    base_class(source, skey),
    m_tree(*this, NIL)
{
}

/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 * @param guard the guard of the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
tree_data_table<Table, IndexedRecord, Key, Interface>::tree_data_table(source_type& source, skey_type& skey, const guard_type& guard) :
    base_class(source, skey, guard),
    m_tree(*this, NIL)
{
}

/**
 * Read a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read(record_type& record, const pos_type pos) const
{
    indexed_record_type indexed_record;
    const pos_type result = base_class::unsafe_read(indexed_record, pos);
    record = static_cast<record_type>(indexed_record);
    return result;
}

/**
 * Reverser read a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_rread(record_type& record, const pos_type pos) const
{
    indexed_record_type indexed_record;
    const pos_type result = base_class::unsafe_rread(indexed_record, pos);
    record = static_cast<record_type>(indexed_record);
    return result;
}

/**
 * Read records (without any locking)
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read(record_list& records, const pos_type pos) const
{
    pos_type result = pos;
    const typename record_list::iterator end = records.end();
    for (typename record_list::iterator it = records.begin(); it != end; ++it)
    {
        result = unsafe_read(*it, result);
    }
    return result;
}

/**
 * Write a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_write(const record_type& record, const pos_type pos)
{
    pnode_type pnode(*this, pos);
    typename tree_type::iterator it(pnode);
    m_tree.replace(it, record);
    return unsafe_table::inc_pos(pos);
}

/**
 * Reverse write a record (without any locking)
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_rwrite(const record_type& record, const pos_type pos)
{
    unsafe_write(record, pos);
    return unsafe_table::dec_pos(pos);
}

/**
 * Write records (without any locking)
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_write(const record_list& records, const pos_type pos)
{
    pos_type result = pos;
    const typename record_list::iterator end = records.end();
    for (typename record_list::const_iterator it = records.begin(); it != end; ++it)
    {
        result = unsafe_write(*it, result);
    }
    return result;
}

/**
 * Add a record (without any locking, without a cache)
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::do_add(const record_type& record)
{
    if (unsafe_table::count() < unsafe_table::limit())
    {
        m_tree.insert(record);
        return unsafe_table::end_pos();
    }
    else
    {
        const pos_type result = unsafe_write(record, unsafe_table::end_pos());
        unsafe_table::set_beg_pos(result);
        unsafe_table::set_end_pos(result);
        return result;
    }
}

/**
 * Add a record (without any locking)
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_add(const record_type& record)
{
    return do_add(record);
}

/**
 * Add records (without any locking)
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_add(const record_list& records)
{
    pos_type result = unsafe_table::end_pos();
    const typename record_list::const_iterator end = records.end();
    for (typename record_list::const_iterator it = records.begin(); it != end; ++it)
    {
        result = do_add(*it);
    }
    return result;
}

/**
 * Read the first record (without any locking)
 * @param record data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read_front(record_type& record) const
{
    indexed_record_type indexed_record;
    const pos_type pos = base_class::unsafe_read_front(indexed_record);
    if (pos != NIL)
    {
        record = static_cast<record_type>(indexed_record);
    }
    return pos;
}

/**
 * Read the first records (without any locking)
 * @param records data of the first records
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read_front(record_list& records) const
{
    indexed_record_list indexed_records(records.size());
    const pos_type pos = base_class::unsafe_read_front(indexed_records);
    if (pos != NIL)
    {
        const count_type count = records.size();
        for (count_type i = 0; i < count; ++i)
        {
            records[i] = static_cast<record_type>(indexed_records[i]);
        }
    }
    return pos;
}

/**
 * Read the last record (without any locking)
 * @param record data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read_back(record_type& record) const
{
    indexed_record_type indexed_record;
    const pos_type pos = base_class::unsafe_read_back(indexed_record);
    if (pos != NIL)
    {
        record = static_cast<record_type>(indexed_record);
    }
    return pos;
}

/**
 * Read the last records (without any locking)
 * @param records data of the first records
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::unsafe_read_back(record_list& records) const
{
    indexed_record_list indexed_records(records.size());
    const pos_type pos = base_class::unsafe_read_back(indexed_records);
    if (pos != NIL)
    {
        const count_type count = records.size();
        for (count_type i = 0; i < count; ++i)
        {
            records[i] = static_cast<record_type>(indexed_records[i]);
        }
    }
    return pos;
}

/**
 * Read a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read(record_type& record, const pos_type pos) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read(record, pos);
}

/**
 * Read records
 * @param records data of the records
 * @param pos the begin position of the records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read(record_list& records, const pos_type pos) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read(records, pos);
}

/**
 * Write a record
 * @param record data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::write(const record_type& record, const pos_type pos)
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::write(const record_list& records, const pos_type pos)
{
    typename base_class::lock_write lock(*this);
    return unsafe_write(records, pos);
}

/**
 * Add a record
 * @param record data of the record
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::add(const record_type& record)
{
    typename base_class::lock_write lock(*this);
    return unsafe_add(record);
}

/**
 * Add records
 * @param records data of the records
 * @return the end position of the records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::add(const record_list& records)
{
    typename base_class::lock_write lock(*this);
    return unsafe_add(records);
}

/**
 * Perform an action before removing record
 * @param pos the position of record to be deleted
 */
//virtual
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::do_before_remove(const pos_type pos)
{
    pnode_type pnode(*this, pos);
    typename tree_type::iterator it(pnode);
    m_tree.remove(it);
}

/**
 * Perform an action before moving record
 * @param source the position of record to be moved
 * @param dest the position of record to be deleted
 */
//virtual
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::do_before_move(const pos_type source, const pos_type dest)
{
    pnode_type pnode(*this, source);
    m_tree.move(pnode, dest);
}

/**
 * Delete records by index [beg, end)
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the count of the deleted records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::remove_by_index(const field_type& beg, const field_type& end)
{
    typename base_class::lock_write lock(*this);
    const count_type limit = unsafe_table::limit();
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    // delete records in parts from the end
    count_type count = 0;
    const pos_list::reverse_iterator itend = list.rend();
    for (pos_list::reverse_iterator it = list.rbegin(); it != itend; ++it)
    {
        pos_type pbeg = *it % limit;
        count_type pcount = 1;
        for (pos_list::reverse_iterator ait = it + 1 ; ait != itend; ++ait)
        {
            const pos_type pos = *ait % limit;
            if (unsafe_table::distance(pos, pbeg) > 1)
            {
                break;
            }
            pbeg = pos;
            it = ait;
            ++pcount;
        }
        unsafe_table::remove(pbeg, pcount);
        count += pcount;
    }
    return count;
}

/**
 * Read the first record
 * @param record data of the first record
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_front(record_type& record) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read_front(record);
}

/**
 * Read the first records
 * @param records data of the first records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_front(record_list& records) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read_front(records);
}

/**
 * Read the last record
 * @param record data of the last record
 * @return the position of the last record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_back(record_type& record) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read_back(record);
}

/**
 * Read the last records
 * @param record data of the last records
 * @return the position of the next record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_back(record_list& records) const
{
    typename base_class::lock_read lock(*this);
    return unsafe_read_back(records);
}

/**
 * Read the first record by index
 * @param record data of the record
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the position of the first record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    read_front_by_index(record_type& record, const field_type& beg, const field_type& end) const
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
        const pos_type pos = *it % unsafe_table::limit();
        unsafe_read(record, pos);
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    read_back_by_index(record_type& record, const field_type& beg, const field_type& end) const
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
        const pos_type pos = *it % unsafe_table::limit();
        unsafe_read(record, pos);
        return pos;
    }
}

/**
 * Get positions of the records that have an index in range [beg, end)
 * @param dest the destination of positions of records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::do_get_pos_list(pos_list& dest, const field_type& beg, const field_type& end) const
{
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    const pos_type beg_pos = unsafe_table::beg_pos();
    const pos_type end_pos = unsafe_table::end_pos();
    ///@todo doesn't work for unordered table
    if (beg_pos < end_pos)
    {
        for (typename tree_type::const_iterator it = itbeg; it != itend; ++it)
        {
            const pos_type pos = it->pos();
            dest.push_back(pos);
        }
    }
    else
    {
        const count_type limit = unsafe_table::limit();
        for (typename tree_type::const_iterator it = itbeg; it != itend; ++it)
        {
            const pos_type pos = it->pos();
            dest.push_back(pos >= beg_pos ? pos : pos + limit);
        }
    }
}

/**
 * Read indexes of the records
 * @param dest the destination of positions of records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of indexes
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_index(pos_list& dest,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    do_get_pos_list(dest, beg, end);
    std::sort(dest.begin(), dest.end());
    if (size != 0 && size < dest.size())
    {
        dest.resize(size);
    }
    const count_type limit = unsafe_table::limit();
    std::transform(dest.begin(), dest.end(), dest.begin(), std::bind2nd(std::modulus<pos_type>(), limit));
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::rread_index(pos_list& dest,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    do_get_pos_list(dest, beg, end);
    std::sort(dest.begin(), dest.end(), std::greater<pos_type>());
    if (size != 0 && size < dest.size())
    {
        dest.resize(size);
    }
    const count_type limit = unsafe_table::limit();
    std::transform(dest.begin(), dest.end(), dest.begin(), std::bind2nd(std::modulus<pos_type>(), limit));
    return dest.size();
}

/**
 * Read records that have an index in range [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::read(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    records.resize(read_index(list, beg, end, size));
    pos_list::const_iterator pos = list.begin();
    const typename record_list::iterator itend = records.end();
    for (typename record_list::iterator it = records.begin(); it != itend; ++it)
    {
        unsafe_read(*it, *pos++);
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::rread(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    records.resize(rread_index(list, beg, end, size));
    pos_list::const_reverse_iterator pos = list.rbegin();
    const typename record_list::iterator itend = records.end();
    for (typename record_list::iterator it = records.begin(); it != itend; ++it)
    {
        unsafe_read(*it, *pos++);
    }
    return records.size();
}

/**
 * Get a record that has an index
 * @param field the value of index
 * @param record the record that has an index
 * @return the position of the record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::get(const field_type& field,
    record_type& record) const
{
    typename base_class::lock_read lock(*this);
    const typename tree_type::iterator it = m_tree.find(field);
    if (it != m_tree.end())
    {
        record = it->get().body();
        return it->pos();
    }
    return NIL;
}

/**
 * Read records by index [beg, end)
 * @param records data of the records
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @param size the maximum count of records (if size = 0 then the count of records is unlimited)
 * @return the count of records
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::read_by_index(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    count_type count = 0;
    for (typename tree_type::const_iterator it = itbeg; it != itend; ++it)
    {
        records.push_back(it->get().body());
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::rread_by_index(record_list& records,
    const field_type& beg, const field_type& end, const count_type size) const
{
    typename base_class::lock_read lock(*this);
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    typename tree_type::const_reverse_iterator ritbeg(itend);
    typename tree_type::const_reverse_iterator ritend(itbeg);
    count_type count = 0;
    for (typename tree_type::const_reverse_iterator it = ritbeg; it != ritend; ++it)
    {
        records.push_back(it->get().body());
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    find_by_index(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    for (typename tree_type::const_iterator it = itbeg; it != itend; ++it)
    {
        const pos_type pos = it->pos();
        finder.record(pos) = it->get().body();
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    rfind_by_index(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    typename tree_type::const_reverse_iterator ritbeg(itend);
    typename tree_type::const_reverse_iterator ritend(itbeg);
    for (typename tree_type::const_reverse_iterator it = ritbeg; it != ritend; ++it)
    {
        const pos_type pos = it->pos();
        finder.record(pos) = it->get().body();
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    find_in_range(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    const count_type limit = unsafe_table::limit();
    const pos_list::const_iterator itend = list.end();
    for (pos_list::const_iterator it = list.begin(); it != itend; ++it)
    {
        const pos_type pos = *it % limit;
        unsafe_read(finder.record(pos), pos);
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    rfind_in_range(Finder& finder, const field_type& beg, const field_type& end) const
{
    typename base_class::lock_read lock(*this);
    pos_list list;
    do_get_pos_list(list, beg, end);
    std::sort(list.begin(), list.end());
    const count_type limit = unsafe_table::limit();
    const pos_list::reverse_iterator itend = list.rend();
    for (pos_list::reverse_iterator it = list.rbegin(); it != itend; ++it)
    {
        const pos_type pos = *it % limit;
        unsafe_read(finder.record(pos), pos);
        if (!finder())
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Find a record in the range [beg, beg + count)
 * @param finder the finder
 * @param beg the begin position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    find(Finder& finder, const pos_type beg, const count_type count) const
{
    typename base_class::lock_read lock(*this);
    if (!unsafe_table::empty())
    {
        pos_type pos = beg;
        for (count_type i = 0; i < count; ++i)
        {
            const pos_type result = pos;
            pos = unsafe_read(finder.record(result), result);
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
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
template <typename Finder>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    rfind(Finder& finder, const pos_type end, const count_type count) const
{
    typename base_class::lock_read lock(*this);
    pos_type pos = unsafe_table::dec_pos(end);
    for (count_type i = 0; i < count; ++i)
    {
        const pos_type result = pos;
        pos = unsafe_rread(finder.record(result), result);
        if (!finder())
        {
            return result;
        }
    }
    return NIL;
}

/**
 * Get a count of records that have index in range [beg, end)
 * @param beg the begin value of the index field
 * @param end the end value of the index field
 * @return the count of records that have index in range [beg, end)
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
count_type tree_data_table<Table, IndexedRecord, Key, Interface>::
    get_range_size(const field_type& beg, const field_type& end) const
{
    typename tree_type::const_iterator itbeg = m_tree.lower_bound(beg);
    typename tree_type::const_iterator itend = m_tree.upper_bound(end);
    count_type count = 0;
    for (typename tree_type::const_iterator it = itbeg; it != itend; ++it)
    {
        ++count;
    }
    return count;
}

/**
 * Clear the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::clear()
{
    typename base_class::lock_write lock(*this);
    tree_data_table<Table, IndexedRecord, Key, Interface>::do_clear();
}

/**
 * Clear the table
 */
//virtual
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::do_clear()
{
    m_tree.clear();
}

/**
 * Refresh the metadata of the table by the key
 * @return the result of the checking
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline bool tree_data_table<Table, IndexedRecord, Key, Interface>::refresh()
{
    typename base_class::lock_read lock(*this);
    bool result = unsafe_table::refresh();
    if (result)
    {
        m_tree.set_root(base_class::cast_skey().root);
    }
#ifdef OUROBOROS_NODECACHE_ENABLED
    cache_type::static_begin(*this);
#endif
    return result;
}

/**
 * Update the key by the metadata of the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline void tree_data_table<Table, IndexedRecord, Key, Interface>::update()
{
    typename base_class::lock_write lock(*this);
#ifdef OUROBOROS_NODECACHE_ENABLED
    cache_type::static_end();
#endif
    skey_type& skey = base_class::cast_skey();
    skey.root = m_tree.get_root();
    unsafe_table::update();
}

/**
 * Recovery the metadata of the table by the key
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
inline void tree_data_table<Table, IndexedRecord, Key, Interface>::recovery()
{
    typename base_class::lock_read lock(*this);
#ifdef OUROBOROS_NODECACHE_ENABLED
    cache_type::static_cancel();
#endif
    m_tree.set_root(unsafe_table::skey().root);
    unsafe_table::recovery();
}

#ifdef OUROBOROS_TEST_TOOLS_ENABLED
/**
 * Test the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
void tree_data_table<Table, IndexedRecord, Key, Interface>::test() const
{
    m_tree.test();
}

/**
 * Get the position of the table
 * @return the position of the table
 */
template <template <typename, typename, typename> class Table, typename IndexedRecord, typename Key, typename Interface>
pos_type tree_data_table<Table, IndexedRecord, Key, Interface>::get_root() const
{
    return m_tree.get_root();
}
#endif

} //namespace ouroboros

#endif	/* OUROBOROS_TREEDATATABLE_H */

