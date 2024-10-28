/**
 * @file   table.h
 * The base table and data source
 */

#ifndef OUROBOROS_TABLE_H
#define OUROBOROS_TABLE_H

#include <stddef.h>
#include <string.h>

#include "ouroboros/basic.h"

namespace ouroboros
{

/**
 * The base table that has records of the same size
 */
template <typename Source, typename Key>
class table : public base_table<Source, Key>
{
    typedef base_table<Source, Key> base_class;
public:
    typedef typename base_class::source_type source_type;
    typedef typename base_class::skey_type skey_type;

    table(source_type& source, skey_type& skey);

    pos_type read(void *data, const pos_type pos) const; ///< read a record
    pos_type read(void *data, const pos_type beg, const count_type count) const; ///< read records [beg, beg + count)
    pos_type rread(void *data, const pos_type pos) const; ///< reverse read a record
    pos_type write(const void *data, const pos_type pos); ///< write a record
    pos_type write(const void *data, const pos_type beg, const count_type count); ///< write records [beg, beg + count)
    pos_type rwrite(const void *data, const pos_type pos); ///< reverse write a record
    pos_type add(const void *data); ///< add a record
    pos_type add(const void *data, const count_type count); ///< add records
    pos_type remove(const pos_type pos); ///< remove a record
    pos_type remove(const pos_type beg, const count_type count); ///< remove records [beg, beg + count)
    count_type remove_back(const count_type count); ///< remove records from the back of the table
    pos_type read_front(void *data) const; ///< read the first record
    pos_type read_front(void *data, const count_type count) const; ///< read the first records
    pos_type read_back(void *data) const; ///< read the last record
    pos_type read_back(void *data, const count_type count) const; ///< read the last records
    pos_type find(const void *data, const pos_type beg, const count_type count) const; ///< find a record [beg, beg + count)
    pos_type rfind(const void *data, const pos_type end, const count_type count) const; ///< reverse find a record [end - count, end)

    inline count_type distance(const pos_type beg, const pos_type end) const; ///< calculate the count of pages in the range [beg, end)
protected:
    inline void do_remove(const pos_type beg, const pos_type end); ///< remove records [beg, end)
    inline pos_type do_find(const void *data, const pos_type beg, const pos_type end) const; ///< find a record [beg, end)
    inline pos_type do_rfind(const void *data, const pos_type beg, const pos_type end) const; ///< reverse find a record [beg. end)

    inline bool valid_pos(const pos_type pos) const; ///< check the position is valid
    inline bool valid_range(const pos_type beg, const pos_type end) const; ///< check the range [beg, end) is valid
private:
    virtual pos_type do_inc_pos(const pos_type pos, const count_type count) const; ///< increment the position
    virtual pos_type do_dec_pos(const pos_type pos, const count_type count) const; ///< decrement the position
    virtual void do_before_remove(const pos_type pos); ///< perform an action before removing record
    virtual void do_before_move(const pos_type source, const pos_type dest); ///< perform an action before moving record
    void do_read(void *data, const pos_type beg, const pos_type end) const; ///< read records [beg, end)
    void do_write(const void *data, const pos_type beg, const pos_type end); ///< write records [beg, end)
};

/**
 * The interface class adapter for table
 */
template <typename Source, typename Key, typename Interface>
class interface_table : public table<Source, Key>
{
    typedef table<Source, Key> base_class;
public:
    typedef Key skey_type;
    typedef Source source_type;
    interface_table(source_type& source, skey_type& key) :
        base_class(source, key)
    {
    }
};

/**
 * The base source that has the tables of the same size
 * @attention the source has an external object File which
 * can be used for building several different sources
 */
template <typename File>
class source
{
    template <typename Key, typename Record, template <typename> class Index, typename Interface>
    friend class data_set;
public:
    typedef File file_type; ///< type of a file source
    typedef typename file_type::file_page_type file_page_type; ///< type of a file page
    typedef typename file_type::file_region_type file_region_type; ///< type of a file region

    /** constructors using an external table source file */
    source(file_type& file, const size_type rec_size, const options_type& options = options_type());
    source(file_type& file, const count_type tbl_count, const size_type rec_size, const options_type& options = options_type());
    source(file_type& file, const count_type tbl_count, const count_type rec_count, const size_type rec_size, const options_type& options = options_type());

    /** constructors using their own table source file */
    source(const std::string& name, const size_type rec_size, const options_type& options = options_type());
    source(const std::string& name, const count_type tbl_count, const size_type rec_size, const options_type& options = options_type());
    source(const std::string& name, const count_type tbl_count, const count_type rec_count, const size_type rec_size, const options_type& options = options_type());

    ~source();

    void set_file_region(const file_region_type& region); ///< set the file region
    void init(const count_type tbl_count, const count_type rec_count); ///< initialize the source

    inline const std::string& name() const; ///< get the name of the source
    size_type size() const; ///< get the size of the source
    offset_type table_offset(const pos_type index) const; ///< get the offset of the table
    pos_type table_index(const offset_type table_offset) const; ///< get the index of the table
    inline size_type table_size() const; ///< get the size of a table
    inline count_type table_count() const; ///< get the count of tables
    inline size_type table_space() const; ///< get the size of a tables separator
    inline size_type rec_size() const; ///< get the size of the table by records
    inline count_type rec_count() const; ///< get the count of the records in the table
    inline size_type rec_space() const; ///< get the size of a records separator
    inline offset_type offset() const; ///< get the offset of the source

    inline void start();  ///< start the transaction
    inline void stop();   ///< stop the transaction
    inline void cancel(); ///< cancel the transaction
    inline transaction_state state() const; ///< get the state of the transaction

    inline void read(void *data, const size_type size, const offset_type offset); ///< read raw data
    inline void write(const void *data, const size_type size, const offset_type offset); ///< write raw data
    inline void refresh(const offset_type offset); /// refresh data of the table

    static void remove(const std::string& name); ///< remove the source
    static void copy(const std::string& source, const std::string& dest); ///< copy the source to dest
protected:
    void resize(); ///< change the file of the source
    const file_type& file() const; ///< get the file of the source
private:
    source();
    source(const source& );
    source& operator= (const source& );
private:
    file_type *m_file; ///< the file of source
    const bool m_owner; ///< the ownership flag of the file
    const size_type m_rec_size; ///< the size of the tables record
    count_type m_tbl_count; ///< the count of the tables
    count_type m_rec_count; ///< the count of the records in a table
    options_type m_options; ///< the additional options
};

//==============================================================================
//  table
//==============================================================================
/**
 * Constructor
 * @param source the source of data
 * @param skey the key of the table
 */
template <typename Source, typename Key>
table<Source, Key>::table(source_type& source, skey_type& skey) :
    base_class(source, skey)
{
}

/**
 * Read a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read(void *data, const pos_type pos) const
{
    base_class::read(data, base_class::rec_size(), base_class::rec_offset(pos));
    return base_class::inc_pos(pos);
}

/**
 * Read records in the range [beg, beg + count)
 * @param data data of the records
 * @param beg the begin position of the records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read(void *data, const pos_type beg, const count_type count) const
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    const pos_type end = base_class::inc_pos(beg, count);
    if (0 == base_class::rec_space())
    {
        do_read(data, beg, end);
    }
    else
    {
        const size_type rec_size = base_class::rec_size();
        char *buffer = static_cast<char *>(data);
        pos_type pos = beg;
        do
        {
            pos = read(buffer, pos);
            buffer += rec_size;
        } while (pos != end);
    }
    return end;
}

/**
 * Reverse read a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::rread(void *data, const pos_type pos) const
{
    base_class::read(data, base_class::rec_size(), base_class::rec_offset(pos));
    return base_class::dec_pos(pos);
}

/**
 * Read records in the range [beg, end)
 * @attention the method reads both the records and the separators between the
 * records
 * @param data data of the records
 * @param beg the begin position of the records
 * @param end the end position of the records
 */
template <typename Source, typename Key>
void table<Source, Key>::do_read(void *data, const pos_type beg, const pos_type end) const
{
    const size_type rec_size = base_class::rec_size() + base_class::rec_space();
    if (end > beg)
    {
        const count_type count = end - beg;
        base_class::read(data, rec_size * count, base_class::rec_offset(beg));
    }
    else
    {
        const count_type count = base_class::limit() - beg;
        char *buffer = static_cast<char *>(data);
        base_class::read(buffer, rec_size * count, base_class::rec_offset(beg));
        if (end > 0)
        {
            buffer += rec_size * count;
            base_class::read(buffer, rec_size * end, base_class::rec_offset(0));
        }
    }
}

/**
 * Write a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::write(const void *data, const pos_type pos)
{
    base_class::write(data, base_class::rec_size(), base_class::rec_offset(pos));
    return base_class::inc_pos(pos);
}

/**
 * Write records in the range [beg, beg + count)
 * @param data data of the records
 * @param beg the begin position of the records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::write(const void *data, const pos_type beg, const count_type count)
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    const pos_type end = base_class::inc_pos(beg, count);
    if (0 == base_class::rec_space())
    {
        do_write(data, beg, end);
    }
    else
    {
        const size_type rec_size = base_class::rec_size();
        const char *buffer = static_cast<const char *>(data);
        pos_type pos = beg;
        do
        {
            pos = write(buffer, pos);
            buffer += rec_size;
        } while (pos != end);
    }
    return end;
}

/**
 * Reverse write a record
 * @param data data of the record
 * @param pos the position of the record
 * @return the position of the previous record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::rwrite(const void *data, const pos_type pos)
{
    base_class::write(data, base_class::rec_size(), base_class::rec_offset(pos));
    return base_class::dec_pos(pos);
}

/**
 * Write records in the range [beg, end)
 * @attention the method rewrites both the records and the separators between the
 * records and if the separators have useful data then the data will be lost
 * @param data data of the records
 * @param beg the begin position of the records
 * @param end the end position of the records
 */
template <typename Source, typename Key>
void table<Source, Key>::do_write(const void *data, const pos_type beg, const pos_type end)
{
    const size_type rec_size = base_class::rec_size() + base_class::rec_space();
    if (end > beg)
    {
        const count_type count = end - beg;
        base_class::write(data, rec_size * count, base_class::rec_offset(beg));
    }
    else
    {
        const count_type count = base_class::limit() - beg;
        const char *buffer = static_cast<const char *>(data);
        base_class::write(buffer, rec_size * count, base_class::rec_offset(beg));
        if (end > 0)
        {
            buffer += rec_size * count;
            base_class::write(buffer, rec_size * end, base_class::rec_offset(0));
        }
    }
}

/**
 * Add a record
 * @param data data of the record
 * @return the end position of the records
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::add(const void *data)
{
    skey_type& cast_skey = base_class::cast_skey();
    cast_skey.end = write(data, cast_skey.end);
    if (base_class::inc_count())
    {
        cast_skey.beg = cast_skey.end;
    }
    return cast_skey.end;
}

/**
 * Add records
 * @param data data of the records
 * @param count the count of the records
 * @return the end position of the records
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::add(const void *data, const count_type count)
{
    skey_type& cast_skey = base_class::cast_skey();
    cast_skey.end = write(data, cast_skey.end, count);
    if (base_class::inc_count(count))
    {
        cast_skey.beg = cast_skey.end;
    }
    return cast_skey.end;
}

/**
 * Remove a record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::remove(const pos_type pos)
{
    return remove(pos, 1);
}

/**
 * Remove records in the range [beg, beg + count)
 * @param beg the begin position of the record
 * @param count the count of the delete record
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::remove(const pos_type beg, const count_type count)
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    const pos_type end = base_class::inc_pos(beg, count);
    if (valid_range(beg, end))
    {
        const skey_type& cast_skey = base_class::cast_skey();
        if (base_class::count() == count)
        {
            this->do_clear();
            return cast_skey.beg;
        }
        else if (beg == cast_skey.beg)
        {
            base_class::do_before_remove(beg, count);
            base_class::dec_count(count);
            return base_class::inc_beg_pos(count);
        }
        else if (end == cast_skey.end)
        {
            base_class::do_before_remove(beg, count);
            base_class::dec_count(count);
            return base_class::dec_end_pos(count);
        }
        else
        {
            // have to move the tail to the place of the deleted records
            base_class::do_before_remove(beg, count);
            do_remove(beg, end);
            base_class::dec_count(count);
            base_class::dec_end_pos(count);
            return beg;
        }
    }
    OUROBOROS_THROW_ERROR(range_error, PR(beg) << PR(count) << PR(base_class::cast_skey()) << "the range is invalid");
    return 0;
}

/**
 * Remove records from the back of the table
 * @param count the count of deleted records
 * @return the count of remaining records
 */
template <typename Source, typename Key>
count_type table<Source, Key>::remove_back(const count_type count)
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    if (count >= base_class::count())
    {
        this->do_clear();
        return 0;
    }
    else
    {
        ///@todo need to replace the most small part of records (head or tail)
        const pos_type pos = base_class::dec_pos(base_class::cast_skey().end, count);
        base_class::do_before_remove(pos, count);
        base_class::dec_end_pos(count);
        base_class::dec_count(count);
        return base_class::count();
    }
}

/**
 * Perform an action before removing record
 * @param pos the position of record to be deleted
 */
//virtual
template <typename Source, typename Key>
void table<Source, Key>::do_before_remove(const pos_type pos)
{
    OUROBOROS_UNUSED(pos);
}

/**
 * Perform an action before moving record
 * @param source the position of record to be moved
 * @param dest the position of record to be deleted
 */
//virtual
template <typename Source, typename Key>
void table<Source, Key>::do_before_move(const pos_type source, const pos_type dest)
{
    OUROBOROS_UNUSED(source);
    OUROBOROS_UNUSED(dest);
}

/**
 * Remove records in the range [beg, end)
 * @param beg the begin position of the records
 * @param end the end position of the records
 */
template <typename Source, typename Key>
inline void table<Source, Key>::do_remove(const pos_type beg, const pos_type end)
{
    ///@todo need to replace the most small part of records (head or tail)
    scoped_buffer<void> buffer(base_class::rec_size());
    void *pbuffer = buffer.get();
    pos_type ibeg = beg;
    pos_type iend = end;
    const skey_type& cast_skey = base_class::cast_skey();
    do
    {
        do_before_move(iend, ibeg);
        iend = read(pbuffer, iend);
        ibeg = write(pbuffer, ibeg);
    } while (iend != cast_skey.end);
}

/**
 * Read the first record
 * @param data data of the first record
 * @return the position of the first record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read_front(void* data) const
{
    const pos_type pos = base_class::front_pos();
    if (pos != NIL)
    {
        read(data, pos);
    }
    return pos;
}

/**
 * Read the first record
 * @param data data of the first records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read_front(void* data, const count_type count) const
{
    const pos_type pos = base_class::front_pos();
    if (pos != NIL && count <= base_class::count())
    {
        return read(data, pos, count);
    }
    return NIL;
}

/**
 * Read the last record
 * @param data data of the last record
 * @return the position of the last record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read_back(void* data) const
{
    const pos_type pos = base_class::back_pos();
    if (pos != NIL)
    {
        read(data, pos);
    }
    return pos;
}

/**
 * Read the last record
 * @param data data of the last records
 * @param count the count of the read records
 * @return the position of the next record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::read_back(void* data, const count_type count) const
{
    pos_type pos = base_class::back_pos();
    if (pos != NIL && count <= base_class::count())
    {
        OUROBOROS_RANGE_ASSERT(count > 0);
        const size_type rec_size = base_class::rec_size();
        char *buffer = static_cast<char *>(data);
        for (count_type i = 0; i < count; ++i)
        {
            pos = rread(buffer, pos);
            buffer += rec_size;
        }
        return pos;
    }
    return NIL;
}

/**
 * Find a record in the range [beg, end)
 * @param data data of the record
 * @param beg the begin position of the records
 * @param end the end position of the records
 * @return the position of the found record
 */
template <typename Source, typename Key>
inline pos_type table<Source, Key>::do_find(const void *data, const pos_type beg, const pos_type end) const
{
    const size_type rec_size = base_class::rec_size();
    scoped_buffer<void> buffer(rec_size);
    for (pos_type pos = beg; pos < end; ++pos)
    {
        base_class::read(buffer.get(), rec_size, base_class::rec_offset(pos));
        if (memcmp(buffer.get(), data, rec_size) == 0)
        {
            return pos;
        }
    }
    return NIL;
}

/**
 * Reverse find a record in the range [beg, end)
 * @param data data of the record
 * @param beg the begin position of the records
 * @param end the end position of the records
 * @return the position of the found record
 */
template <typename Source, typename Key>
inline pos_type table<Source, Key>::do_rfind(const void *data, const pos_type beg, const pos_type end) const
{
    const size_type rec_size = base_class::rec_size();
    scoped_buffer<void> buffer(rec_size);
    pos_type pos = base_class::dec_pos(end);
    do
    {
        base_class::read(buffer.get(), rec_size, base_class::rec_offset(pos));
        if (memcmp(buffer.get(), data, rec_size) == 0)
        {
            return pos;
        }
    } while (--pos != beg);
    return NIL;
}

/**
 * Find a record in the range [beg, beg + count)
 * @param data data of the record
 * @param beg the begin position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::find(const void *data, const pos_type beg, const count_type count) const
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    const pos_type end = base_class::inc_pos(beg, count);
    if (end > beg)
    {
        return do_find(data, beg, end);
    }
    const pos_type pos = do_find(data, beg, base_class::limit());
    if (pos != NIL)
    {
        return pos;
    }
    return do_find(data, 0, end);
}

/**
 * Reverse find a record in the range [end - count, end)
 * @param data data of the record
 * @param end the end position of the records
 * @param count the count of the find records
 * @return the position of the found record
 */
template <typename Source, typename Key>
pos_type table<Source, Key>::rfind(const void *data, const pos_type end, const count_type count) const
{
    OUROBOROS_RANGE_ASSERT(count > 0);
    const pos_type beg = base_class::dec_pos(end, count);
    if (end > beg)
    {
        return do_rfind(data, beg, end);
    }
    const pos_type pos = (0 == end) ? NIL : do_rfind(data, 0, end);
    if (pos != NIL)
    {
        return pos;
    }
    return do_rfind(data, beg, base_class::limit());
}

/**
 * Increment the position
 * @param pos the position of the record
 * @param count the value of the increment
 * @return new position of the record
 */
//virtual
template <typename Source, typename Key>
pos_type table<Source, Key>::do_inc_pos(const pos_type pos, const count_type count) const
{
    return (pos + count) % base_class::limit();
}

/**
 * Decrement the position
 * @param pos the position of the record
 * @param count the value of the decrement
 * @return new position of the record
 */
//virtual
template <typename Source, typename Key>
inline pos_type table<Source, Key>::do_dec_pos(const pos_type pos, const count_type count) const
{
    return pos >= count ? pos - count : base_class::limit() + pos - count;
}

/**
 * Calculate the count of pages in the range [beg, end)
 * @param beg the begin position of the records
 * @param end the end position of the records
 * @return the count of pages in the range
 */
template <typename Source, typename Key>
inline count_type table<Source, Key>::distance(const pos_type beg, const pos_type end) const
{
    return end > beg ? end - beg : base_class::limit() - beg + end;
}

/**
 * Check the position is valid
 * @param pos the position
 * @return the result of the checking
 */
template <typename Source, typename Key>
inline bool table<Source, Key>::valid_pos(const pos_type pos) const
{
    const count_type max = base_class::limit();
    const skey_type& cast_skey = base_class::cast_skey();
    const pos_type rbeg = cast_skey.beg;
    const pos_type rend = cast_skey.end;
    return !base_class::empty() && pos < max &&
        (rbeg < rend ? (rbeg <= pos && pos < rend) : (rbeg <= pos || pos < rend));
}

/**
 * Check the range of the positions if valid
 * @param beg the begin position of the range
 * @param end the end position of the range
 * @return result the checking
 */
template <typename Source, typename Key>
inline bool table<Source, Key>::valid_range(const pos_type beg, const pos_type end) const
{
    ///@todo not found checking for beg < end end etc.
    const count_type max = base_class::limit();
    const skey_type& cast_skey = base_class::cast_skey();
    const pos_type rbeg = cast_skey.beg;
    const pos_type rend = cast_skey.end;
    return !base_class::empty() && beg < max && end < max &&
        (rbeg < rend ?
            (rbeg <= beg && beg < rend) && (rbeg < end && end <= rend) :
            (rbeg <= beg || beg < rend) && (rbeg < end || end <= rend));
}

//==============================================================================
//  source
//==============================================================================
/**
 * Remove the source of data
 * @param name the name of the source of data
 */
//static
template <typename File>
void source<File>::remove(const std::string& name)
{
    file_type::remove(name);
}

/**
 * Copy the source to dest
 * @param source the source name
 * @param dest the dest name
 */
//static
template <typename File>
void source<File>::copy(const std::string& source, const std::string& dest)
{
    file_type::copy(source, dest);
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param file the file of the data source
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename File>
source<File>::source(file_type& file, const size_type rec_size,
        const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options)
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
template <typename File>
source<File>::source(file_type& file, const count_type tbl_count,
        const size_type rec_size, const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(tbl_count),
    m_rec_count(0),
    m_options(options)
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
template <typename File>
source<File>::source(file_type& file, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size,
        const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options)
{
    init(tbl_count, rec_count);
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param name the file name of the data source
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename File>
source<File>::source(const std::string& name, const size_type rec_size,
        const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options)
{
    m_file = new file_type(name);
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param name the file name of the data source
 * @param tbl_count the count of the tables
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename File>
source<File>::source(const std::string& name, const count_type tbl_count,
        const size_type rec_size, const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(tbl_count),
    m_rec_count(0),
    m_options(options)
{
    m_file = new file_type(name);
}

/**
 * Constructor that is used when parameters of the source are known
 * @param name the file name of the data source
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename File>
source<File>::source(const std::string& name, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size,
        const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options)
{
    m_file = new file_type(name);
    init(tbl_count, rec_count);
}

/**
 * Destructor
 */
template <typename File>
source<File>::~source()
{
    if (m_owner)
    {
        delete m_file;
    }
}

/**
 * Set the file region
 * @param region the file region
 */
template <typename File>
void source<File>::set_file_region(const file_region_type& region)
{
    m_file->set_region(region);
    resize();
}

/**
 * Initialize the data source
 * @attention use after calling constructor without full parameters
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 */
template <typename File>
void source<File>::init(const count_type tbl_count, const count_type rec_count)
{
    if (m_rec_count != 0)
    {
        OUROBOROS_THROW_BUG(PR(name()) << PR(m_tbl_count) << PR(m_rec_count)
            << PR(tbl_count) << PR(rec_count) << "attempt to reinitialize");
    }
    m_tbl_count = tbl_count;
    m_rec_count = rec_count;
    resize();
}

/**
 * Get the name of the data source
 * @return the name of the data source
 */
template <typename File>
inline const std::string& source<File>::name() const
{
    return m_file->name();
}

/**
 * Get the size of the data source
 * @return the size of the data source
 */
template <typename File>
size_type source<File>::size() const
{
    return 0 == table_count() ? 0 : table_count() * (table_size() + table_space());
}

/**
 * Read raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename File>
inline void source<File>::read(void *data, const size_type size,
        const offset_type offset)
{
    m_file->read(data, size, offset);
}

/**
 * Write raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename File>
inline void source<File>::write(const void *data, const size_type size,
        const offset_type offset)
{
    m_file->write(data, size, offset);
}

/**
 * Refresh data of the table
 * @param offset the offset of the table
 */
template <typename File>
inline void source<File>::refresh(const offset_type offset)
{
    m_file->refresh(table_size(), offset);
}

/**
 * Change the size of the data source
 */
template <typename File>
void source<File>::resize()
{
    const size_type new_size = offset() + size();
    m_file->sizeup(new_size);
}

/**
 * Get the offset of the table
 * @param index the index of the table
 * @return the offset of the table
 */
template <typename File>
offset_type source<File>::table_offset(const pos_type index) const
{
    if (index < table_count())
    {
        return offset() + index * (table_size() + table_space());
    }
    else
    {
        OUROBOROS_THROW_ERROR(range_error, PR(index) << PR(table_count()) << "the table is not found");
        return NIL;
    }
}

/**
 * Get the index of the table
 * @param table_offset the offset of the table
 * @return the index of the table
 */
template <typename File>
pos_type source<File>::table_index(const offset_type table_offset) const
{
    if ((table_offset - offset()) % (table_size() + table_space()) != 0)
    {
        OUROBOROS_THROW_BUG(PR(table_offset) << "the offset of the table is not aligned");
    }
    return (table_offset - offset()) / (table_size() + table_space());
}

/**
 * Get the size of a table
 * @return the size of a table
 */
template <typename File>
inline size_type source<File>::table_size() const
{
    return rec_count() * (rec_size() + rec_space());
}

/**
 * Get the count of tables
 * @return the count of tables
 */
template <typename File>
inline count_type source<File>::table_count() const
{
    return m_tbl_count;
}

/**
 * Get the size of a tables separator
 * @return the size of a tables separator
 */
template <typename File>
inline size_type source<File>::table_space() const
{
    return m_options.tbl_space;
}

/**
 * Get the size of the record
 * @return the size of the record
 */
template <typename File>
inline size_type source<File>::rec_size() const
{
    return m_rec_size;
}

/**
 * Get the count of the records in the table
 * @return the count of the records in the table
 */
template <typename File>
inline size_type source<File>::rec_count() const
{
    return m_rec_count;
}

/**
 * Get the size of a records separator
 * @return the size of a records separator
 */
template <typename File>
inline size_type source<File>::rec_space() const
{
    return m_options.rec_space;
}

/**
 * Get the offset of the data source
 * @return the offset of the data source
 */
template <typename File>
inline offset_type source<File>::offset() const
{
    return m_options.offset;
}

/**
 * Start the transaction
 */
template <typename File>
inline void source<File>::start()
{
    m_file->start();
}

/**
 * Stop the transaction
 */
template <typename File>
inline void source<File>::stop()
{
    m_file->stop();
}

/**
 * Cancel the transaction
 */
template <typename File>
inline void source<File>::cancel()
{
    m_file->cancel();
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <typename File>
inline transaction_state source<File>::state() const
{
    return m_file->state();
}

/**
 * Get the file of the data source
 * @return the file of the data source
 */
template <typename File>
inline const typename source<File>::file_type& source<File>::file() const
{
    return *m_file;
}

}   //namespace ouroboros

#endif  /* OUROBOROS_TABLE_H */

