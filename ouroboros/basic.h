/**
 * @file    basic.h
 * The basic interface of a table
 */

#ifndef OUROBOROS_BASIC_H
#define OUROBOROS_BASIC_H

#include "ouroboros/global.h"
#include "ouroboros/file.h"
#include "ouroboros/cache.h"
#include "ouroboros/scoped_buffer.h"

namespace ouroboros
{

/**
 * The class of options for a source of data
 */
struct options_type
{
    options_type() :
        offset(0),
        rec_space(0),
        tbl_space(0)
    {}
    options_type(const offset_type aoffset, const count_type arec_space = 0, const count_type atbl_space = 0) :
        offset(aoffset),
        rec_space(arec_space),
        tbl_space(atbl_space)
    {}
    offset_type   offset;   ///< offset
    size_type  rec_space;   ///< size of a separator between records
    size_type  tbl_space;   ///< size of a separator between tables
};

/**
 * The base cursor of a table
 */
template <typename Table>
class base_cursor
{
public:
    typedef Table table_type;
    base_cursor(const base_cursor& o);
    base_cursor(table_type& table, const pos_type pos);

    inline pos_type pos() const;
    base_cursor& operator++ ();
    base_cursor operator++ (int );
    base_cursor& operator-- ();
    base_cursor operator-- (int );

    bool operator== (const base_cursor& o) const;
    bool operator!= (const base_cursor& o) const;

    base_cursor& operator= (const base_cursor& o);
protected:
    inline void pos(const pos_type value) const;
    inline table_type& table() const;
private:
    table_type& m_table;
    mutable pos_type m_pos;
};

/**
 * The basic interface of a table
 */
template <typename Source, typename Key>
class base_table
{
    typedef base_table<Source, Key> self_type;
public:
    enum
    {
        TABLE_TYPE = TABLE_SIMPLE,
        REC_SPACE = 0
    };

    typedef Source source_type;
    typedef Key skey_type;
    typedef base_cursor<self_type> const_cursor;

    base_table(source_type& source, skey_type& skey);
    virtual ~base_table();

    const_cursor begin() const; ///< get the begin cursor
    const_cursor end() const; ///< get the end cursor
    inline pos_type beg_pos() const; ///< get the begin position of records
    inline pos_type end_pos() const; ///< get the end position of records
    inline pos_type inc_pos(const pos_type pos, const count_type count = 1) const; ///< increment the position
    inline pos_type dec_pos(const pos_type pos, const count_type count = 1) const; ///< decrement the position
    inline pos_type front_pos() const; ///< get the position of the first record
    inline pos_type back_pos() const; ///< get the position of the last record

    inline count_type limit() const; ///< get the size of the table by records
    inline count_type count() const; ///< get the count of records
    inline void set_count(const count_type count); ///< set the count of records
    inline bool empty() const; ///< check the table is empty

    void clear(); ///< clear the table

    inline pos_type index() const; ///< get the index of the table in the source of data

    inline revision_type revision() const; ///< get the revision of modifying the table
    inline void set_revision(const revision_type rev); ///< set the revision of modifying the table
    inline revision_type inc_revision(); ///< increment the revision of modifying the table
    inline bool relevant() const; ///< check the table is relevant
    inline bool refresh(); ///< refresh the metadata of the table by the key
    inline void update(); ///< update the key by the metadata of the table
    inline void recovery(); ///< recovery the metadata of the table by the key

    inline void start();  ///< start the transaction
    inline void stop();   ///< stop the transaction
    inline void cancel(); ///< cancel the transaction
    inline transaction_state state() const; ///< get the state of the transaction

    inline const skey_type& skey() const; ///< get the reference to the key of the table
    inline skey_type& cast_skey(); ///< get the cast of the key at the begin of the transaction
    inline source_type& source(); ///< get the source of data
    inline const source_type& source() const; ///< get the source of data

    inline void set_helper(const void *) {} ///< just a stub
#ifdef OUROBOROS_TEST_TOOLS_ENABLED
    void test() const {} ///< test the table
#endif
protected:
    inline void set_beg_pos(const pos_type pos); ///< set the begin position of records
    pos_type inc_beg_pos(const count_type count = 1); ///< increment the begin position of records
    pos_type dec_beg_pos(const count_type count = 1); ///< decrement the begin position of records
    inline void set_end_pos(const pos_type pos); ///< set the end position of records
    pos_type inc_end_pos(const count_type count = 1); ///< increment the end position of records
    pos_type dec_end_pos(const count_type count = 1); ///< decrement the end position of records

    inline const skey_type& cast_skey() const; ///< get the cast of the key at the begin of the transaction
    inline count_type rec_size() const; ///< get the size of a record
    inline count_type rec_space() const; ///< get the size of the records separator
    offset_type rec_offset(const pos_type pos) const; ///< get the offset of the record
    inline offset_type offset() const; ///< get the offset of the table
    inline bool inc_count(const count_type count = 1); ///< increment the records count of the table
    inline bool dec_count(const count_type count = 1); ///< decrement the records count of the table

    virtual pos_type do_inc_pos(const pos_type pos, const count_type count) const = 0; ///< increment the position
    virtual pos_type do_dec_pos(const pos_type pos, const count_type count) const = 0; ///< decrement the position
    inline void read(void *data, const size_type size, const offset_type offset) const; ///< read raw data
    inline void write(const void *data, const size_type size, const offset_type offset); ///< write raw data
    virtual void do_before_remove(const pos_type pos) = 0; ///< perform an action before deleting record
    virtual void do_before_move(const pos_type source, const pos_type dest) = 0; ///< perform an action before moving record
    inline void do_before_remove(pos_type pos, count_type count); ///< perform an action before removing record
    inline void do_before_move(pos_type source, pos_type dest, count_type count); ///< perform an action before moving records
    virtual void do_clear(); ///< clear the table
private:
    base_table();
    base_table(const base_table& );
    base_table& operator= (const base_table& );
private:
    source_type& m_source;  ///< source of the table
    const offset_type m_offset;   ///< offset of the table
    skey_type& m_skey;      ///< reference to the key of the table
    skey_type m_cast_skey;  ///< the cast of the key
};

//==============================================================================
//  base_cursor
//==============================================================================
/**
 * Constructor
 * @param table the table
 * @param pos the current position
 */
template <typename Table>
base_cursor<Table>::base_cursor(table_type& table, const pos_type pos) :
    m_table(table),
    m_pos(pos)
{

}

/**
 * Constructor
 * @param o the another cursor
 */
template <typename Table>
base_cursor<Table>::base_cursor(const base_cursor<Table>& o) :
    m_table(o.m_table),
    m_pos(o.m_pos)
{

}

/**
 * Get current position
 * @return current position
 */
template <typename Table>
inline pos_type base_cursor<Table>::pos() const
{
    return m_pos;
}

/**
 * Set current position
 * @param value current position
 */
template <typename Table>
inline void base_cursor<Table>::pos(const pos_type value) const
{
    m_pos = value;
}

/**
 * Get the table
 * @return the table
 */
template <typename Table>
inline typename base_cursor<Table>::table_type& base_cursor<Table>::table() const
{
    return m_table;
}

/**
 * Operator ++
 * @return the cursor to next position
 */
template <typename Table>
base_cursor<Table>& base_cursor<Table>::operator++ ()
{
    m_pos = m_table.inc_pos(m_pos);
    return *this;
}

/**
 * Operator ++
 * @return the cursor to current position
 */
template <typename Table>
base_cursor<Table> base_cursor<Table>::operator++ (int )
{
    const pos_type pos = m_pos;
    operator++();
    return base_cursor<Table>(m_table, pos);
}

/**
 * Operator --
 * @return the cursor to previous position
 */
template <typename Table>
base_cursor<Table>& base_cursor<Table>::operator-- ()
{
    m_pos = m_table.dec_pos(m_pos);
    return *this;
}

/**
 * Operator --
 * @return the cursor to current position
 */
template <typename Table>
base_cursor<Table> base_cursor<Table>::operator-- (int )
{
    const pos_type pos = m_pos;
    operator--();
    return base_cursor<Table>(m_table, pos);
}

/**
 * Operator ==
 * @param o the another cursor
 * @return the result of the checking
 */
template <typename Table>
bool base_cursor<Table>::operator== (const base_cursor<Table>& o) const
{
    return &m_table == &o.m_table && m_pos == o.m_pos;
}

/**
 * Operator !=
 * @param o the another cursor
 * @return the result of the checking
 */
template <typename Table>
bool base_cursor<Table>::operator!= (const base_cursor<Table>& o) const
{
    return !operator== (o);
}

/**
 * Operator =
 * @param o the another cursor
 * @return reference to yourself
 */
template <typename Table>
base_cursor<Table>& base_cursor<Table>::operator= (const base_cursor<Table>& o)
{
    m_table = o.m_table;
    m_pos = o.m_pos;
    return *this;
}

//==============================================================================
//  base_table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <typename Source, typename Key>
base_table<Source, Key>::base_table(source_type& source, skey_type& skey) :
    m_source(source),
    m_offset(source.table_offset(skey.pos)),
    m_skey(skey),
    m_cast_skey(skey)
{
}

/**
 * Clear the table
 */
//virtual
template <typename Source, typename Key>
void base_table<Source, Key>::do_clear()
{
    m_cast_skey.beg = 0;
    m_cast_skey.end = 0;
    m_cast_skey.count = 0;
}

/**
 * Clear the table
 */
template <typename Source, typename Key>
void base_table<Source, Key>::clear()
{
    base_table<Source, Key>::do_clear();
}

/**
 * Destructor
 */
//virtual
template <typename Source, typename Key>
base_table<Source, Key>::~base_table()
{
}

/**
 * Get the begin cursor
 * @return the begin cursor
 */
template <typename Source, typename Key>
typename base_table<Source, Key>::const_cursor base_table<Source, Key>::begin() const
{
    return const_cursor(*this, beg_pos());
}

/**
 * Get the end cursor
 * @return the end cursor
 */
template <typename Source, typename Key>
typename base_table<Source, Key>::const_cursor base_table<Source, Key>::end() const
{
    return const_cursor(*this, end_pos());
}

/**
 * Get the begin position of records
 * @return the begin position of records
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::beg_pos() const
{
    return m_cast_skey.beg;
}

/**
 * Set the begin position of records
 * @param pos the begin position of records
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::set_beg_pos(const pos_type pos)
{
    m_cast_skey.beg = pos;
}

/**
 * Increment the begin position of records
 * @param count value of the increment
 * @return new begin position of records
 */
template <typename Source, typename Key>
pos_type base_table<Source, Key>::inc_beg_pos(const count_type count)
{
    m_cast_skey.beg = inc_pos(m_cast_skey.beg, count);
    return m_cast_skey.beg;
}

/**
 * Decrement the begin position of records
 * @param count value of the decrement
 * @return new begin position of records
 */
template <typename Source, typename Key>
pos_type base_table<Source, Key>::dec_beg_pos(const count_type count)
{
    m_cast_skey.beg = dec_pos(m_cast_skey.beg, count);
    return m_cast_skey.beg;
}

/**
 * Get the end position of records
 * @return the end position of records
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::end_pos() const
{
    return m_cast_skey.end;
}

/**
 * Set the end position of records
 * @param pos the end position of records
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::set_end_pos(const pos_type pos)
{
    m_cast_skey.end = pos;
}

/**
 * Increment the end position of records
 * @param count value of the increment
 * @return new end position of records
 */
template <typename Source, typename Key>
pos_type base_table<Source, Key>::inc_end_pos(const count_type count)
{
    m_cast_skey.end = inc_pos(m_cast_skey.end, count);
    return m_cast_skey.end;
}

/**
 * Decrement the end position of records
 * @param count value of the decrement
 * @return new end position of records
 */
template <typename Source, typename Key>
pos_type base_table<Source, Key>::dec_end_pos(const count_type count)
{
    m_cast_skey.end = dec_pos(m_cast_skey.end, count);
    return m_cast_skey.end;
}

/**
 * Increment the position
 * @param pos the position of the record
 * @param count the value of the increment
 * @return new position of the record
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::inc_pos(const pos_type pos, const count_type count) const
{
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    const count_type max = limit();
    if (count > max)
    {
        OUROBOROS_THROW_ERROR(range_error, PR(pos) << PR(count) << PR(max) << "the value of increment greater than the size of the table");
    }
#endif
    return do_inc_pos(pos, count);
}

/**
 * Decrement the position
 * @param pos the position of the record
 * @param count the value of the decrement
 * @return new position of the record
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::dec_pos(const pos_type pos, const count_type count) const
{
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    const count_type max = limit();
    if (count > max)
    {
        OUROBOROS_THROW_ERROR(range_error, PR(pos) << PR(count) << PR(max) << "the value of decrement greater than the size of the table");
    }
#endif
    return do_dec_pos(pos, count);
}

/**
 * Get the position of the first record
 * @return the position of the first record
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::front_pos() const
{
    return empty() ? NIL : m_cast_skey.beg;
}

/**
 * Get the position of the last record
 * @return the position of the last record
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::back_pos() const
{
    return empty() ? NIL : dec_pos(m_cast_skey.end);
}

/**
 * Get the size of the table by records
 * @return the size of the table by records
 */
template <typename Source, typename Key>
inline count_type base_table<Source, Key>::limit() const
{
    return m_source.rec_count();
}

/**
 * Get the count of records
 * @return the count of records
 */
template <typename Source, typename Key>
inline count_type base_table<Source, Key>::count() const
{
    return m_cast_skey.count;
}

/**
 * Set the count of records
 * @param count the count of records
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::set_count(const count_type count)
{
    m_cast_skey.count = count;
}

/**
 * Check the table is empty
 * @return the result of the checking
 */
template <typename Source, typename Key>
inline bool base_table<Source, Key>::empty() const
{
    return 0 == m_cast_skey.count;
}

/**
 * Get the index of the table in the source of data
 * @return the index of the table in the source of data
 */
template <typename Source, typename Key>
inline pos_type base_table<Source, Key>::index() const
{
    return m_source.table_index(m_offset);
}

/**
 * Get the revision of modifying the table
 * @return the revision of modifying the table
 */
template <typename Source, typename Key>
inline revision_type base_table<Source, Key>::revision() const
{
    return m_cast_skey.rev;
}

/**
 * Set the revision of modifying the table
 * @param rev the revision of modifying the table
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::set_revision(const revision_type rev)
{
    m_cast_skey.rev = rev;
}

/**
 * Increment the revision of modifying the table
 * @return new revision of modifying the table
 */
template <typename Source, typename Key>
inline revision_type base_table<Source, Key>::inc_revision()
{
    return ++m_cast_skey.rev;
}

/**
 * Check the table is relevant
 * @return the result of the checking
 */
template <typename Source, typename Key>
inline bool base_table<Source, Key>::relevant() const
{
    return m_skey.rev == m_cast_skey.rev;
}

/**
 * Refresh the metadata of the table by the key
 * @return the result of the checking
 */
template <typename Source, typename Key>
inline bool base_table<Source, Key>::refresh()
{
    // check the table is relevant
    if (!relevant())
    {
        // refresh information of the table
        m_source.refresh(m_offset);
        m_cast_skey = m_skey;
        return true;
    }
    return false;
}

/**
 * Update the key by the metadata of the table
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::update()
{
    inc_revision();
    m_skey = m_cast_skey;
}

/**
 * Recovery the metadata of the table by the key
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::recovery()
{
    m_cast_skey = m_skey;
}

/**
 * Start the transaction
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::start()
{
    m_source.start();
}

/**
 * Stop the transaction
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::stop()
{
    m_source.stop();
}

/**
 * Cancel the transaction
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::cancel()
{
    m_source.cancel();
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <typename Source, typename Key>
inline transaction_state base_table<Source, Key>::state() const
{
    return m_source.state();
}

/**
 * Get the reference to the key of the table
 * @return the reference to the key of the table
 */
template <typename Source, typename Key>
inline const typename base_table<Source, Key>::skey_type&
    base_table<Source, Key>::skey() const
{
    ///@todo have to think about the need to add the secure method for the class locked_table
    return m_skey;
}

/**
 * Get the cast of the key at the begin of the transaction
 * @return the cast of the key at the begin of the transaction
 */
template <typename Source, typename Key>
inline typename base_table<Source, Key>::skey_type&
    base_table<Source, Key>::cast_skey()
{
    ///@todo have to think about the need to add the secure method for the class locked_table
    return m_cast_skey;
}

/**
 * Get the cast of the key at the begin of the transaction
 * @return the cast of the key at the begin of the transaction
 */
template <typename Source, typename Key>
inline const typename base_table<Source, Key>::skey_type&
    base_table<Source, Key>::cast_skey() const
{
    ///@todo have to think about the need to add the secure method for the class locked_table
    return m_cast_skey;
}

/**
 * Get the source
 * @return the source
 */
template <typename Source, typename Key>
inline typename base_table<Source, Key>::source_type&
    base_table<Source, Key>::source()
{
    return m_source;
}

/**
 * Get the source of data
 * @return the source of data
 */
template <typename Source, typename Key>
inline const typename base_table<Source, Key>::source_type&
    base_table<Source, Key>::source() const
{
    return m_source;
}

/**
 * Get the size of a record
 * @return the size of a record
 */
template <typename Source, typename Key>
inline size_type base_table<Source, Key>::rec_size() const
{
    return m_source.rec_size();
}

/**
 * Get the size of the records separator
 * @return the size of the records separator
 */
template <typename Source, typename Key>
inline size_type base_table<Source, Key>::rec_space() const
{
    return m_source.rec_space();
}

/**
 * Get the offset of the record
 * @param pos the position of the record
 * @return the offset of the record
 */
template <typename Source, typename Key>
offset_type base_table<Source, Key>::rec_offset(const pos_type pos) const
{
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (pos >= limit())
    {
        OUROBOROS_THROW_ERROR(range_error, PR(pos) << PR(m_cast_skey) << "the position does not exist");
    }
#endif
    return offset() + (rec_size() + rec_space()) * pos;
}

/**
 * Get the offset of the table
 * @return the offset of the table
 */
template <typename Source, typename Key>
inline offset_type base_table<Source, Key>::offset() const
{
    return m_offset;
}

/**
 * Increment the records count of the table
 * @param count value of the increment
 * @return the overflow indication
 */
template <typename Source, typename Key>
inline bool base_table<Source, Key>::inc_count(const count_type count)
{
    const count_type max = limit();
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (count > max)
    {
        OUROBOROS_THROW_ERROR(range_error, PR(count) << PR(max) << "the value of increment greater than the size of the table");
    }
#endif
    m_cast_skey.count += count;
    if (m_cast_skey.count > max)
    {
        m_cast_skey.count = max;
        return true;
    }
    return false;
}

/**
 * Decrement the records count of the table
 * @param count value of the decrement
 * @return the indication that there are not any records in the table
 */
template <typename Source, typename Key>
inline bool base_table<Source, Key>::dec_count(const count_type count)
{
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    if (count > m_cast_skey.count)
    {
        OUROBOROS_THROW_ERROR(range_error, PR(count) << PR(m_cast_skey) << "the value of decrement greater than the count of the records");
    }
#endif
    m_cast_skey.count -= count;
    return 0 == m_cast_skey.count;
}

/**
 * Read raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::read(void *data, const size_type size, const offset_type offset) const
{
    m_source.read(data, size, offset);
}

/**
 * Write raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename Source, typename Key>
inline void base_table<Source, Key>::write(const void *data, const size_type size, const offset_type offset)
{
    m_source.write(data, size, offset);
}

/**
 * Perform an action before removing records
 * @param pos the position of records to be deleted
 * @param count the count of records to be deleted
 */
template <typename Source, typename Key>
void base_table<Source, Key>::do_before_remove(pos_type pos, count_type count)
{
    ///@todo execute the cycle if the method do_before_remove exist
    while (count-- > 0)
    {
        do_before_remove(pos);
        pos = inc_pos(pos);
    }
}

/**
 * Perform an action before moving records
 * @param source the position of records to be moved
 * @param dest the position of records to be deleted
 * @param count the count of records to be moved
 */
template <typename Source, typename Key>
void base_table<Source, Key>::do_before_move(pos_type source, pos_type dest, count_type count)
{
    ///@todo execute the cycle if the method do_before_remove exist
    while (count-- > 0)
    {
        do_before_move(source, dest);
        source = inc_pos(source);
        dest = inc_pos(dest);
    }
}

}   //namespace ouroboros

#endif /* OUROBOROS_BASIC_H */

