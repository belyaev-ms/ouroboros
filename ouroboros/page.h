/**
 * @file    page.h
 * The page is a part fo a data file
 */

#ifndef OUROBOROS_PAGE_H
#define OUROBOROS_PAGE_H

#include "ouroboros/global.h"
#include <vector>

namespace ouroboros
{

/**
 * The base file page that provide access to useful data and to service data
 */
template <int pageSize, int serviceSize = 0>
class file_page
{
public:
    enum
    {
        TOTAL_SIZE      = pageSize,
        SERVICE_SIZE    = serviceSize,
        DATA_SIZE       = TOTAL_SIZE - SERVICE_SIZE
    };

    file_page();
    virtual ~file_page();
    explicit file_page(const pos_type pos);
    file_page(const file_page& page);
    const pos_type pos() const;
    const pos_type index() const;
    void assign(void *ptr);
    void *read(void *buffer) const;
    void *read(void *buffer, const size_type size) const;
    void *read_rest(void *buffer) const;
    const void *write(const void *buffer);
    const void *write(const void *buffer, const size_type size);
    const void *write_rest(const void *buffer);
    const bool valid() const;
    const bool operator== (const file_page& page) const;
    const bool operator< (const file_page& page) const;
    file_page& operator++ ();
    file_page& operator= (const file_page& page);
    static const size_type static_size();
    static const size_type static_data_size();
    static const pos_type static_convert(const pos_type pos);
private:
    void *do_read(void *buffer, const pos_type offset, const size_type size) const;
    const void *do_write(const void *buffer, const pos_type offset, const size_type size);
private:
    pos_type m_pos;
    pos_type m_index;
    void *m_ptr;
};

/**
 * The base file region that solves the problem  of separating a file into
 * several aligned regions
 */
template <int pageSize>
class file_region
{
public:
    enum
    {
        FILE_PAGE_SIZE = pageSize
    };
    typedef std::pair<offset_type, offset_type> range_type;
    typedef std::vector<file_region> region_list;
    file_region();
    file_region(const count_type count, const size_type size);
    file_region(const count_type count, const file_region& region);
    file_region(const count_type count, const region_list& regions);
    file_region& add(const file_region& region);
    const range_type operator[] (const pos_type index) const;
    const offset_type to_offset(const offset_type raw_offset) const;
private:
    const size_type align_size(const size_type size) const;
    const std::pair<offset_type, bool> get_offset(const pos_type index,
            count_type& count, offset_type offset) const;
    const std::pair<offset_type, bool> get_offset(offset_type& raw_offset,
            offset_type offset) const;
private:
    count_type m_count;
    size_type m_size;
    region_list m_regions;
};

//==============================================================================
//  file_page
//==============================================================================
/**
 * Get the size of the file page
 * @return the size of the file page
 */
//static
template <int pageSize, int serviceSize>
const size_type file_page<pageSize, serviceSize>::static_size()
{
    return TOTAL_SIZE;
}

/**
 * Get the size of a block data in the file page
 * @return the size of a block data in the file page
 */
//static
template <int pageSize, int serviceSize>
const size_type file_page<pageSize, serviceSize>::static_data_size()
{
    return DATA_SIZE;
}

/**
 * Convert a position of useful data to a real position of data in a file
 * @param pos a position of useful data
 * @return a real position of data in a file
 */
//static
template <int pageSize, int serviceSize>
const pos_type file_page<pageSize, serviceSize>::static_convert(const pos_type pos)
{
    if (SERVICE_SIZE == 0)
    {
        return pos;
    }
    const pos_type index = pos / DATA_SIZE;
    return index * TOTAL_SIZE + pos % DATA_SIZE;
}

/**
 * Constructor
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>::file_page() :
    m_pos(NIL),
    m_index(NIL),
    m_ptr(NULL)
{
}

//virtual
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>::~file_page()
{

}

/**
 * Constructor
 * @param pos the position in the data file
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>::file_page(const pos_type pos) :
    m_pos(static_convert(pos)),
    m_index(pos / DATA_SIZE),
    m_ptr(NULL)
{
}

/**
 * Constructor
 * @param page another file page
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>::file_page(const file_page& page) :
    m_pos(page.m_pos),
    m_index(page.m_index),
    m_ptr(page.m_ptr)
{
}

/**
 * Get the position in the data file
 * @return the position in the data file
 */
template <int pageSize, int serviceSize>
const pos_type file_page<pageSize, serviceSize>::pos() const
{
    return m_pos;
}

/**
 * Get the index of the file page
 * @return the index of the file page
 */
template <int pageSize, int serviceSize>
const pos_type file_page<pageSize, serviceSize>::index() const
{
    return m_index;
}

/**
 * Assign a memory data page
 * @param ptr the pointer to a memory data page
 */
template <int pageSize, int serviceSize>
void file_page<pageSize, serviceSize>::assign(void *ptr)
{
    m_ptr = ptr;
}

/**
 * Read data into the buffer
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
void *file_page<pageSize, serviceSize>::read(void *buffer) const
{
    const pos_type offset = m_pos % DATA_SIZE;
    const size_type size = DATA_SIZE - offset;
    return do_read(buffer, offset, size);
}

/**
 * Read rest of data into the buffer
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
void *file_page<pageSize, serviceSize>::read_rest(void *buffer) const
{
    const size_type size = m_pos % DATA_SIZE + 1;
    return do_read(buffer, 0, size);
}

/**
 * Read rest of data into the buffer
 * @param buffer the pointer to the buffer
 * @param size the size of the useful memory block
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
void *file_page<pageSize, serviceSize>::read(void *buffer, const size_type size) const
{
    const pos_type offset = m_pos % DATA_SIZE;
    return do_read(buffer, offset, size);
}

/**
 * Read data into the buffer
 * @param buffer the pointer to the buffer
 * @param offset the offset of the useful data
 * @param size the size of the useful data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
void *file_page<pageSize, serviceSize>::do_read(void *buffer, const pos_type offset, const size_type size) const
{
    OUROBOROS_ASSERT(buffer != NULL && valid() && offset + size <= DATA_SIZE);
    memcpy(buffer, static_cast<char *>(m_ptr) + offset, size);
    return static_cast<char *>(buffer) + size;
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
const void *file_page<pageSize, serviceSize>::write(const void *buffer)
{
    const pos_type offset = m_pos % DATA_SIZE;
    const size_type size = DATA_SIZE - offset;
    return do_write(buffer, offset, size);
}

/**
 * Write the rest of data into the file page
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
const void *file_page<pageSize, serviceSize>::write_rest(const void *buffer)
{
    const size_type size = m_pos % DATA_SIZE + 1;
    return do_write(buffer, 0, size);
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @param size the size of data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
const void *file_page<pageSize, serviceSize>::write(const void *buffer, const size_type size)
{
    const pos_type offset = m_pos % DATA_SIZE;
    return do_write(buffer, offset, size);
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @param offset the offset in the file page
 * @param size the size of data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize, int serviceSize>
const void *file_page<pageSize, serviceSize>::do_write(const void *buffer, const pos_type offset, const size_type size)
{
    OUROBOROS_ASSERT(buffer != NULL && valid() && offset + size <= DATA_SIZE);
    memcpy(static_cast<char *>(m_ptr) + offset, buffer, size);
    return static_cast<const char *>(buffer) + size;
}

/**
 * Check the file page is valid
 * @return the result of the checking
 */
template <int pageSize, int serviceSize>
const bool file_page<pageSize, serviceSize>::valid() const
{
    return m_ptr != NULL && m_pos != NIL && m_index != NIL;
}

/**
 * Operator ==
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize, int serviceSize>
const bool file_page<pageSize, serviceSize>::operator== (const file_page& page) const
{
    return m_index == page.m_index;
}

/**
 * Operator <
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize, int serviceSize>
const bool file_page<pageSize, serviceSize>::operator< (const file_page& page) const
{
    return m_index < page.m_index;
}

/**
 * Operator ++
 * @return the next file page
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>& file_page<pageSize, serviceSize>::operator++ ()
{
    ++m_index;
    m_pos = m_index * TOTAL_SIZE;
    m_ptr = NULL;
    return *this;
}

/**
 * Operator =
 * @param page the another file page
 * @return reference to yourself
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>& file_page<pageSize, serviceSize>::operator= (const file_page& page)
{
    m_pos = page.m_pos;
    m_index = page.m_index;
    m_ptr = page.m_ptr;
    return *this;
}

//==============================================================================
//  file_region
//==============================================================================
/**
 * Constructor
 */
template <int pageSize>
file_region<pageSize>::file_region() :
    m_count(0),
    m_size(0)
{
}

/**
 * Constructor
 * @param count the count of subregion
 * @param size the size of a subregion
 */
template <int pageSize>
file_region<pageSize>::file_region(const count_type count, const size_type size) :
    m_count(count),
    m_size(size)
{
}

/**
 * Constructor
 * @param count the count of a subregion
 * @param region the subregion
 */
template <int pageSize>
file_region<pageSize>::file_region(const count_type count, const file_region& region) :
    m_count(count),
    m_size(0),
    m_regions(1, region)
{
}

/**
 * Constructor
 * @param count the count of a list of subregion
 * @param regions the list of subregion
 */
template <int pageSize>
file_region<pageSize>::file_region(const count_type count, const region_list& regions) :
    m_count(count),
    m_size(0),
    m_regions(regions)
{
}

/**
 * Add a subregion
 * @param region the subregion
 * @return reference to yourself
 */
template <int pageSize>
file_region<pageSize>& file_region<pageSize>::add(const file_region& region)
{
    if (m_size != 0)
    {
        OUROBOROS_ASSERT(m_regions.empty());
        m_regions.push_back(*this);
        m_size = 0;
        m_count = 1;
    }
    m_regions.push_back(region);
    return *this;
}

/**
 * Get the range of the index'th region
 * @param index the index of the region
 * @return the range of the region
 */
template <int pageSize>
const typename file_region<pageSize>::range_type file_region<pageSize>::
    operator[] (const pos_type index) const
{
    range_type range(0, 0);
    count_type count = 0;
    std::pair<offset_type, bool> result;
    if (index > 0)
    {
        result = get_offset(index, count, 0);
        OUROBOROS_ASSERT(result.second);
        range.first = result.first;
    }
    count = 0;
    result = get_offset(index + 1, count, 0);
    OUROBOROS_ASSERT(result.second);
    range.second = result.first;
    return range;
}

/**
 * Align a size of a region
 * @param size the size of a region
 * @return the aligned size of a region
 */
template <int pageSize>
const size_type file_region<pageSize>::align_size(const size_type size) const
{
    count_type count = size / pageSize;
    if (size % pageSize != 0)
    {
        ++count;
    }
    return count != 0 ? count * pageSize : pageSize;
}

/**
 * Get the offset of the index'th region
 * @param index the index of the region
 * @param count the count of the passed regions
 * @param offset the current offset
 * @return the offset of the current region and the flag of successful
 */
template <int pageSize>
const std::pair<offset_type, bool> file_region<pageSize>::
    get_offset(const pos_type index, count_type& count, offset_type offset) const
{
    if (m_size > 0)
    {
        const size_type size = align_size(m_size);
        if (m_count + count >= index || m_count == 0)
        {
            return std::make_pair(size * (index - count) + offset, true);
        }
        count += m_count;
        return std::make_pair(m_count * size + offset, false);
    }
    else
    {
        for (size_t i = 0; i < m_count || 0 == m_count; ++i)
        {
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const std::pair<offset_type, bool> result = it->get_offset(index, count, offset);
                if (result.second)
                {
                    return result;
                }
                offset = result.first;
            }
        }
        return std::make_pair(offset, false);
    }
}

/**
 * Convert the raw offset to the real offset in the file
 * @param raw_offset the raw offset
 * @return the real offset in the file
 */
template <int pageSize>
const offset_type file_region<pageSize>::to_offset(const offset_type raw_offset) const
{
    offset_type offset = raw_offset;
    const std::pair<offset_type, bool> result = get_offset(offset, 0);
    OUROBOROS_ASSERT(result.second);
    return result.first;
}

/**
 * Get the real offset in the file
 * @param raw_offset the raw offset
 * @param offset the current offset
 * @return the real offset in the file
 */
template <int pageSize>
const std::pair<offset_type, bool> file_region<pageSize>::get_offset(offset_type& raw_offset, offset_type offset) const
{
    if (m_size > 0)
    {
        const size_type size = align_size(m_size);
        if (raw_offset < m_count * m_size || 0 == m_count)
        {
            const count_type count = raw_offset / m_size;
            return std::make_pair(count * size + raw_offset % m_size + offset, true);
        }
        raw_offset -= m_count * m_size;
        return std::make_pair(m_count * size + offset, false);
    }
    else
    {
        for (size_t i = 0; i < m_count || 0 == m_count; ++i)
        {
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const std::pair<offset_type, bool> result = it->get_offset(raw_offset, offset);
                if (result.second)
                {
                    return result;
                }
                offset = result.first;
            }
        }
        return std::make_pair(offset, false);
    }
}

}   //namespace ouroboros

#endif /* OUROBOROS_PAGE_H */

