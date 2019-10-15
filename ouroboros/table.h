/**
 * @file   table.h
 * The base table and data source
 */

#ifndef OUROBOROS_TABLE_H
#define	OUROBOROS_TABLE_H

#include <stddef.h>
#include <string.h>

#include "ouroboros/basic.h"
#include "ouroboros/page.h"

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

    const pos_type read(void *data, const pos_type pos) const; ///< read a record
    const pos_type read(void *data, const pos_type beg, const count_type count) const; ///< read records [beg, beg + count)
    const pos_type rread(void *data, const pos_type pos) const; ///< reverse read a record
    const pos_type write(const void *data, const pos_type pos); ///< write a record
    const pos_type write(const void *data, const pos_type beg, const count_type count); ///< write records [beg, beg + count)
    const pos_type rwrite(const void *data, const pos_type pos); ///< reverse write a record
    const pos_type add(const void *data); ///< add a record
    const pos_type add(const void *data, const count_type count); ///< add records
    const pos_type remove(const pos_type pos); ///< remove a record
    const pos_type remove(const pos_type beg, const count_type count); ///< remove records [beg, beg + count)
    const count_type remove_back(const count_type count); ///< remove records from the back of the table
    const pos_type read_front(void *data) const; ///< read the first record
    const pos_type read_back(void *data) const; ///< read the last record
    const pos_type find(const void *data, const pos_type beg, const count_type count) const; ///< find a record [beg, beg + count)
    const pos_type rfind(const void *data, const pos_type end, const count_type count) const; ///< reverse find a record [end - count, end)

    inline const count_type distance(const pos_type beg, const pos_type end) const; ///< calculate the count of pages in the range [beg, end)
protected:
    inline void do_remove(const pos_type beg, const pos_type end); ///< remove records [beg, end)
    inline const pos_type do_find(const void *data, const pos_type beg, const pos_type end) const; ///< find a record [beg, end)
    inline const pos_type do_rfind(const void *data, const pos_type beg, const pos_type end) const; ///< reverse find a record [beg. end)

    inline const bool valid_pos(const pos_type pos) const; ///< check the position is valid
    inline const bool valid_range(const pos_type beg, const pos_type end) const; ///< check the range [beg, end) is valid
private:
    virtual const pos_type do_inc_pos(const pos_type pos, const count_type count) const; ///< increment the position
    virtual const pos_type do_dec_pos(const pos_type pos, const count_type count) const; ///< decrement the position
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
template <typename File, typename FilePage>
class source
{
    template <typename Key, typename Record, template <typename> class Index, typename Interface>
    friend class data_set;
public:
    typedef File file_type; ///< type of a file source
    typedef FilePage file_page_type; ///< type of a file page
    typedef file_region<file_page_type> file_region_type; ///< type of a file region

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
    const size_type size() const; ///< get the size of the source
    const offset_type table_offset(const pos_type index) const; ///< get the offset of the table
    const pos_type table_index(const offset_type table_offset) const; ///< get the index of the table
    inline const size_type table_size() const; ///< get the size of a table
    inline const count_type table_count() const; ///< get the count of tables
    inline const size_type table_space() const; ///< get the size of a tables separator
    inline const size_type rec_size() const; ///< get the size of the table by records
    inline const count_type rec_count() const; ///< get the count of the records in the table
    inline const size_type rec_space() const; ///< get the size of a records separator
    inline const offset_type offset() const; ///< get the offset of the source

    inline void start();  ///< start the transaction
    inline void stop();   ///< stop the transaction
    inline void cancel(); ///< cancel the transaction
    inline const transaction_state state() const; ///< get the state of the transaction

    inline void read(void *data, const size_type size, const offset_type offset); ///< read raw data
    inline void write(const void *data, const size_type size, const offset_type offset); ///< write raw data
    inline void refresh(const offset_type offset); /// refresh data of the table

    static void remove(const std::string& name); ///< remove the source
protected:
    void resize(); ///< change the file of the source
    const file_type& file() const; ///< get the file of the source
    inline const offset_type to_offset(const offset_type raw_offset) const; ///< convert the raw offset to the real offset in the file
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
    const file_region_type *m_file_region; ///< the file region
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
const pos_type table<Source, Key>::read(void *data, const pos_type pos) const
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
const pos_type table<Source, Key>::read(void *data, const pos_type beg, const count_type count) const
{
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
const pos_type table<Source, Key>::rread(void *data, const pos_type pos) const
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
const pos_type table<Source, Key>::write(const void *data, const pos_type pos)
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
const pos_type table<Source, Key>::write(const void *data, const pos_type beg, const count_type count)
{
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
const pos_type table<Source, Key>::rwrite(const void *data, const pos_type pos)
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
const pos_type table<Source, Key>::add(const void *data)
{
    base_class::cast_skey().end = write(data, base_class::cast_skey().end);
    if (base_class::inc_count())
    {
        base_class::cast_skey().beg = base_class::cast_skey().end;
    }
    return base_class::cast_skey().end;
}

/**
 * Add records
 * @param data data of the records
 * @param count the count of the records
 * @return the end position of the records
 */
template <typename Source, typename Key>
const pos_type table<Source, Key>::add(const void *data, const count_type count)
{
    base_class::cast_skey().end = write(data, base_class::cast_skey().end, count);
    if (base_class::inc_count(count))
    {
        base_class::cast_skey().beg = base_class::cast_skey().end;
    }
    return base_class::cast_skey().end;
}

/**
 * Remove a record
 * @param pos the position of the record
 * @return the position of the next record
 */
template <typename Source, typename Key>
const pos_type table<Source, Key>::remove(const pos_type pos)
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
const pos_type table<Source, Key>::remove(const pos_type beg, const count_type count)
{
    const pos_type end = base_class::inc_pos(beg, count);
    if (valid_range(beg, end))
    {
        if (base_class::count() == count)
        {
            base_class::do_clear();
            return base_class::cast_skey().beg;
        }
        else if (beg == base_class::cast_skey().beg)
        {
            base_class::do_before_remove(beg, count);
            base_class::dec_count(count);
            return base_class::inc_beg_pos(count);
        }
        else if (end == base_class::cast_skey().end)
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
const count_type table<Source, Key>::remove_back(const count_type count)
{
    if (count >= base_class::count())
    {
        base_class::do_clear();
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
    pos_type ibeg = beg;
    pos_type iend = end;
    do
    {
        do_before_move(iend, ibeg);
        iend = read(buffer.get(), iend);
        ibeg = write(buffer.get(), ibeg);
    } while (iend != base_class::cast_skey().end);
}

/**
 * Read the first record
 * @param data data of the first record
 * @return the position of the first record
 */
template <typename Source, typename Key>
const pos_type table<Source, Key>::read_front(void* data) const
{
    const pos_type pos = base_class::front_pos();
    if (pos != NIL)
    {
        read(data, pos);
    }
    return pos;
}

/**
 * Read the last record
 * @param data data of the last record
 * @return the position of the last record
 */
template <typename Source, typename Key>
const pos_type table<Source, Key>::read_back(void* data) const
{
    const pos_type pos = base_class::back_pos();
    if (pos != NIL)
    {
        read(data, pos);
    }
    return pos;
}

/**
 * Find a record in the range [beg, end)
 * @param data data of the record
 * @param beg the begin position of the records
 * @param end the end position of the records
 * @return the position of the found record
 */
template <typename Source, typename Key>
inline const pos_type table<Source, Key>::do_find(const void *data, const pos_type beg, const pos_type end) const
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
inline const pos_type table<Source, Key>::do_rfind(const void *data, const pos_type beg, const pos_type end) const
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
const pos_type table<Source, Key>::find(const void *data, const pos_type beg, const count_type count) const
{
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
const pos_type table<Source, Key>::rfind(const void *data, const pos_type end, const count_type count) const
{
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
const pos_type table<Source, Key>::do_inc_pos(const pos_type pos, const count_type count) const
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
inline const pos_type table<Source, Key>::do_dec_pos(const pos_type pos, const count_type count) const
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
inline const count_type table<Source, Key>::distance(const pos_type beg, const pos_type end) const
{
    return end > beg ? end - beg : base_class::limit() - beg + end;
}

/**
 * Check the position is valid
 * @param pos the position
 * @return the result of the checking
 */
template <typename Source, typename Key>
inline const bool table<Source, Key>::valid_pos(const pos_type pos) const
{
    const count_type max = base_class::limit();
    const pos_type rbeg = base_class::cast_skey().beg;
    const pos_type rend = base_class::cast_skey().end;
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
inline const bool table<Source, Key>::valid_range(const pos_type beg, const pos_type end) const
{
    ///@todo not found checking for beg < end end etc.
    const count_type max = base_class::limit();
    const pos_type rbeg = base_class::cast_skey().beg;
    const pos_type rend = base_class::cast_skey().end;
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
template <typename File, typename FilePage>
void source<File, FilePage>::remove(const std::string& name)
{
    file_type::remove(name);
}

/**
 * Constructor that is used when parameters of the source are not known
 * @attention after use, you must initialize (method init)
 * @param file the file of the data source
 * @param rec_size the size of the records
 * @param options additional options
 */
template <typename File, typename FilePage>
source<File, FilePage>::source(file_type& file, const size_type rec_size,
        const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
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
template <typename File, typename FilePage>
source<File, FilePage>::source(file_type& file, const count_type tbl_count,
        const size_type rec_size, const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(tbl_count),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
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
template <typename File, typename FilePage>
source<File, FilePage>::source(file_type& file, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size,
        const options_type& options) :
    m_file(&file),
    m_owner(false),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
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
template <typename File, typename FilePage>
source<File, FilePage>::source(const std::string& name, const size_type rec_size,
        const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
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
template <typename File, typename FilePage>
source<File, FilePage>::source(const std::string& name, const count_type tbl_count,
        const size_type rec_size, const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(tbl_count),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
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
template <typename File, typename FilePage>
source<File, FilePage>::source(const std::string& name, const count_type tbl_count,
        const count_type rec_count, const size_type rec_size,
        const options_type& options) :
    m_owner(true),
    m_rec_size(rec_size),
    m_tbl_count(0),
    m_rec_count(0),
    m_options(options),
    m_file_region(NULL)
{
    m_file = new file_type(name);
    init(tbl_count, rec_count);
}

/**
 * Destructor
 */
template <typename File, typename FilePage>
source<File, FilePage>::~source()
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
template <typename File, typename FilePage>
void source<File, FilePage>::set_file_region(const file_region_type& region)
{
    m_file_region = &region;
    resize();
}

/**
 * Initialize the data source
 * @attention use after calling constructor without full parameters
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 */
template <typename File, typename FilePage>
void source<File, FilePage>::init(const count_type tbl_count, const count_type rec_count)
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
template <typename File, typename FilePage>
inline const std::string& source<File, FilePage>::name() const
{
    return m_file->name();
}

/**
 * Get the size of the data source
 * @return the size of the data source
 */
template <typename File, typename FilePage>
const size_type source<File, FilePage>::size() const
{
    return 0 == table_count() ? 0 : table_count() * (table_size() + table_space());
}

/**
 * Convert the raw offset to the real offset in the file
 * @param raw_offset the raw offset
 * @return the real offset in the file
 */
template <typename File, typename FilePage>
inline const offset_type source<File, FilePage>::to_offset(const offset_type raw_offset) const
{
    return NULL == m_file_region ? raw_offset : m_file_region->to_offset(raw_offset);
}

/**
 * Read raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::read(void *data, const size_type size,
        const offset_type offset)
{
    m_file->read(data, size, to_offset(offset));
}

/**
 * Write raw data
 * @param data the buffer of the data
 * @param size the size of the data
 * @param offset the offset of the data
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::write(const void *data, const size_type size,
        const offset_type offset)
{
    m_file->write(data, size, to_offset(offset));
}

/**
 * Refresh data of the table
 * @param offset the offset of the table
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::refresh(const offset_type offset)
{
    m_file->refresh(table_size(), to_offset(offset));
}

/**
 * Change the size of the data source
 */
template <typename File, typename FilePage>
void source<File, FilePage>::resize()
{
    const size_type new_size = to_offset(offset() + size());
    if (m_file->size() < new_size)
    {
        m_file->resize(new_size);
    }
}

/**
 * Get the offset of the table
 * @param index the index of the table
 * @return the offset of the table
 */
template <typename File, typename FilePage>
const offset_type source<File, FilePage>::table_offset(const pos_type index) const
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
template <typename File, typename FilePage>
const pos_type source<File, FilePage>::table_index(const offset_type table_offset) const
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
template <typename File, typename FilePage>
inline const size_type source<File, FilePage>::table_size() const
{
    return rec_count() * (rec_size() + rec_space());
}

/**
 * Get the count of tables
 * @return the count of tables
 */
template <typename File, typename FilePage>
inline const count_type source<File, FilePage>::table_count() const
{
    return m_tbl_count;
}

/**
 * Get the size of a tables separator
 * @return the size of a tables separator
 */
template <typename File, typename FilePage>
inline const size_type source<File, FilePage>::table_space() const
{
    return m_options.tbl_space;
}

/**
 * Get the size of the record
 * @return the size of the record
 */
template <typename File, typename FilePage>
inline const size_type source<File, FilePage>::rec_size() const
{
    return m_rec_size;
}

/**
 * Get the count of the records in the table
 * @return the count of the records in the table
 */
template <typename File, typename FilePage>
inline const size_type source<File, FilePage>::rec_count() const
{
    return m_rec_count;
}

/**
 * Get the size of a records separator
 * @return the size of a records separator
 */
template <typename File, typename FilePage>
inline const size_type source<File, FilePage>::rec_space() const
{
    return m_options.rec_space;
}

/**
 * Get the offset of the data source
 * @return the offset of the data source
 */
template <typename File, typename FilePage>
inline const offset_type source<File, FilePage>::offset() const
{
    return m_options.offset;
}

/**
 * Start the transaction
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::start()
{
    m_file->start();
}

/**
 * Stop the transaction
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::stop()
{
    m_file->stop();
}

/**
 * Cancel the transaction
 */
template <typename File, typename FilePage>
inline void source<File, FilePage>::cancel()
{
    m_file->cancel();
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <typename File, typename FilePage>
inline const transaction_state source<File, FilePage>::state() const
{
    return m_file->state();
}

/**
 * Get the file of the data source
 * @return the file of the data source
 */
template <typename File, typename FilePage>
inline const typename source<File, FilePage>::file_type& source<File, FilePage>::file() const
{
    return *m_file;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_TABLE_H */

