/**
 * @file    page.h
 * The page is a part of a data file
 */

#ifndef OUROBOROS_PAGE_H
#define OUROBOROS_PAGE_H

#include "ouroboros/global.h"
#include <vector>
#include <map>
#include <string.h>

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
    file_page(const pos_type pos, const offset_type offset);
    file_page(const file_page& page);
    pos_type pos() const;
    pos_type index() const;
    void assign(void *ptr);
    void *read(void *buffer) const;
    void *read(void *buffer, const size_type size) const;
    void *read_rest(void *buffer) const;
    const void *write(const void *buffer);
    const void *write(const void *buffer, const size_type size);
    const void *write_rest(const void *buffer);
    bool valid() const;
    const void *get() const;
    void *get();
    bool operator== (const file_page& page) const;
    bool operator< (const file_page& page) const;
    file_page& operator++ ();
    file_page& operator= (const file_page& page);
    static size_type static_size();
    static size_type static_data_size();
    static size_type static_align_size(const size_type size);
    static pos_type static_convert(const pos_type pos);
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
template <typename FilePage>
class file_region
{
public:
    typedef FilePage file_page_type;
    typedef std::pair<offset_type, offset_type> range_type;
    typedef std::vector<file_region> region_list;
    file_region();
    file_region(const count_type count, const size_type size);
    file_region(const count_type count, const file_region& region);
    file_region(const count_type count, const region_list& regions);
    file_region& add(const file_region& region);
    range_type operator[] (const pos_type index) const;
    offset_type convert_offset(const offset_type raw_offset) const;
    size_type convert_size(const size_type raw_size) const;
    void make_cache(const size_type size) const;
private:
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
    struct cache_type
    {
        cache_type() :
            valid(false),
            raw_size(0),
            alligned_size(0)
        {}
        bool valid;
        size_type raw_size;
        size_type alligned_size;
    };
    typedef std::pair<offset_type, bool> result_type;
#else
    struct cached_region
    {
        cached_region() :
            offset(0),
            pregion(NULL)
        {}
        cached_region(const offset_type offset_, const file_region *pregion_) :
            offset(offset_),
            pregion(pregion_)
        {}
        bool valid() const
        {
            return pregion != NULL;
        }
        offset_type offset;
        const file_region *pregion;
    };
    typedef std::pair<offset_type, cached_region> result_type;
#endif
#if !defined(OUROBOROS_FILE_REGION_CACHE_TYPE) || (OUROBOROS_FILE_REGION_CACHE_TYPE == 0)
    typedef std::map<offset_type, cached_region> cache_type;
#elif OUROBOROS_FILE_REGION_CACHE_TYPE == 1
    class cache_type
    {
        enum
        {
            REGIONS_COUNT = 3
        };
    public:
        cache_type() :
            m_valid(false)
        {
            for (size_t i = 0; i < REGIONS_COUNT; ++i)
            {
                m_pregions[i] = NULL;
                m_raw_sizes[i] = 0;
                m_alligned_sizes[i] = 0;
            }
        }
        void make(const file_region& region)
        {
            if (region.m_count != 1 || region.m_size > 0 ||
                region.m_regions.size() != 2 || region.m_regions[0].m_count != 1 ||
                0 == region.m_regions[0].m_size || region.m_regions[1].m_regions.size() != 2 ||
                region.m_regions[1].m_regions[0].m_count != 1 || 0 == region.m_regions[1].m_regions[0].m_size ||
                region.m_regions[1].m_regions[1].m_count != 1 || 0 == region.m_regions[1].m_regions[1].m_size)
            {
                m_valid = false;
                return;
            }
            m_pregions[0] = &region.m_regions[0];
            m_pregions[1] = &region.m_regions[1].m_regions[0];
            m_pregions[2] = &region.m_regions[1].m_regions[1];
            for (size_t i = 0; i < REGIONS_COUNT; ++i)
            {
                m_raw_sizes[i] = m_pregions[i]->m_size;
                m_alligned_sizes[i] = file_page_type::static_align_size(m_raw_sizes[i]);
            }
            m_valid = true;
        }
        result_type get_cached_region(const offset_type raw_offset) const
        {
            if (raw_offset < m_raw_sizes[0])
            {
                return std::make_pair(0, cached_region(0, m_pregions[0]));
            }
            const size_type s1 = (raw_offset - m_raw_sizes[0]);
            const size_type s2 = (m_raw_sizes[1] + m_raw_sizes[2]);
            const size_t n = s1 / s2;
            return (s1 - n * s2 < m_raw_sizes[1]) ?
                std::make_pair(m_raw_sizes[0] + n * s2,
                    cached_region(m_alligned_sizes[0] +
                    n * (m_alligned_sizes[1] + m_alligned_sizes[2]), m_pregions[1])) :
                std::make_pair(m_raw_sizes[0] + n * s2 + m_raw_sizes[1],
                    cached_region(m_alligned_sizes[0] + m_alligned_sizes[1] +
                    n * (m_alligned_sizes[1] + m_alligned_sizes[2]), m_pregions[2]));
        }
        inline bool valid() const
        {
            return m_valid;
        }
    private:
        const file_region *m_pregions[REGIONS_COUNT];
        size_type m_raw_sizes[REGIONS_COUNT];
        size_type m_alligned_sizes[REGIONS_COUNT];
        bool m_valid;
    };
#endif
    size_type align_size(const size_type size) const;
    result_type get_offset(const pos_type index, count_type& count, offset_type offset) const;
    result_type get_offset(offset_type& raw_offset, offset_type offset) const;
#if !defined(OUROBOROS_FILE_REGION_CACHE_TYPE) || (OUROBOROS_FILE_REGION_CACHE_TYPE == 0)
    result_type do_make_cache(cache_type& cache, offset_type max_size, offset_type& raw_offset,
            offset_type offset) const;
#endif
private:
    count_type m_count;
    size_type m_size;
    region_list m_regions;
    mutable cache_type m_cache;
};

/**
 * The file page that has a status block
 */
template <typename FilePage, typename Status = unsigned int>
class status_file_page : public FilePage
{
    typedef FilePage base_class;
    ///*@todo need to add a compile time assert (SERVICE_SIZE >= Status)
public:
    typedef Status status_type;
    explicit status_file_page(void *ptr);
    bool verify() const;
    void set_status(const status_type& status);
    status_type get_status() const;
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
size_type file_page<pageSize, serviceSize>::static_size()
{
    return TOTAL_SIZE;
}

/**
 * Get the size of a block data in the file page
 * @return the size of a block data in the file page
 */
//static
template <int pageSize, int serviceSize>
size_type file_page<pageSize, serviceSize>::static_data_size()
{
    return DATA_SIZE;
}

/**
 * Align a size by the page
 * @param size the size
 * @return the aligned size by the page
 */
//static
template <int pageSize, int serviceSize>
size_type file_page<pageSize, serviceSize>::static_align_size(const size_type size)
{
    count_type count = size / DATA_SIZE;
    if (size % DATA_SIZE != 0)
    {
        ++count;
    }
    return count != 0 ? count * TOTAL_SIZE : static_cast<size_type>(TOTAL_SIZE);
}

/**
 * Convert a position of useful data to a real position of data in a file
 * @param pos a position of useful data
 * @return a real position of data in a file
 */
//static
template <int pageSize, int serviceSize>
pos_type file_page<pageSize, serviceSize>::static_convert(const pos_type pos)
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
    m_pos(pos),
    m_index(pos / TOTAL_SIZE),
    m_ptr(NULL)
{
}

/**
 * Constructor
 * @param pos the position in the data file
 * @param offset the offset in the file
 */
template <int pageSize, int serviceSize>
file_page<pageSize, serviceSize>::file_page(const pos_type pos, const offset_type offset) :
    m_pos(pos),
    m_index(pos / TOTAL_SIZE),
    m_ptr(NULL)
{
    const offset_type page_offset = pos % TOTAL_SIZE + offset;
    if (page_offset >= DATA_SIZE)
    {
        m_pos = m_index * TOTAL_SIZE + static_convert(page_offset);
        m_index = m_pos / TOTAL_SIZE;
    }
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
pos_type file_page<pageSize, serviceSize>::pos() const
{
    return m_pos;
}

/**
 * Get the index of the file page
 * @return the index of the file page
 */
template <int pageSize, int serviceSize>
pos_type file_page<pageSize, serviceSize>::index() const
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
    const pos_type offset = m_pos % TOTAL_SIZE;
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
    const size_type size = m_pos % TOTAL_SIZE + 1;
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
    const pos_type offset = m_pos % TOTAL_SIZE;
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
    const pos_type offset = m_pos % TOTAL_SIZE;
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
    const size_type size = m_pos % TOTAL_SIZE + 1;
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
    const pos_type offset = m_pos % TOTAL_SIZE;
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
bool file_page<pageSize, serviceSize>::valid() const
{
    return m_ptr != NULL && m_pos != NIL && m_index != NIL;
}

/**
 * Operator ==
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize, int serviceSize>
bool file_page<pageSize, serviceSize>::operator== (const file_page& page) const
{
    return m_index == page.m_index;
}

/**
 * Operator <
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize, int serviceSize>
bool file_page<pageSize, serviceSize>::operator< (const file_page& page) const
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
file_page<pageSize, serviceSize>& file_page<pageSize, serviceSize>::
    operator= (const file_page& page)
{
    m_pos = page.m_pos;
    m_index = page.m_index;
    m_ptr = page.m_ptr;
    return *this;
}

/**
 * Get a pointer to data
 * @return the pointer to data
 */
template <int pageSize, int serviceSize>
const void *file_page<pageSize, serviceSize>::get() const
{
    return m_ptr;
}

/**
 * Get a pointer to data
 * @return the pointer to data
 */
template <int pageSize, int serviceSize>
void *file_page<pageSize, serviceSize>::get()
{
    return m_ptr;
}

//==============================================================================
//  file_region
//==============================================================================
/**
 * Constructor
 */
template <typename FilePage>
file_region<FilePage>::file_region() :
    m_count(0),
    m_size(0)
{
}

/**
 * Constructor
 * @param count the count of subregion
 * @param size the size of a subregion
 */
template <typename FilePage>
file_region<FilePage>::file_region(const count_type count, const size_type size) :
    m_count(count),
    m_size(size)
{
}

/**
 * Constructor
 * @param count the count of a subregion
 * @param region the subregion
 */
template <typename FilePage>
file_region<FilePage>::file_region(const count_type count, const file_region& region) :
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
template <typename FilePage>
file_region<FilePage>::file_region(const count_type count, const region_list& regions) :
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
template <typename FilePage>
file_region<FilePage>& file_region<FilePage>::add(const file_region& region)
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
template <typename FilePage>
typename file_region<FilePage>::range_type file_region<FilePage>::
    operator[] (const pos_type index) const
{
    range_type range(0, 0);
    count_type count = 0;
    result_type result;
    if (index > 0)
    {
        result = get_offset(index, count, 0);
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
        OUROBOROS_ASSERT(result.second);
#else
        OUROBOROS_ASSERT(result.second.valid());
#endif
        range.first = result.first;
    }
    count = 0;
    result = get_offset(index + 1, count, 0);
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
    OUROBOROS_ASSERT(result.second);
#else
    OUROBOROS_ASSERT(result.second.valid());
#endif
    range.second = result.first;
    return range;
}

/**
 * Get the offset of the index'th region
 * @param index the index of the region
 * @param count the count of the passed regions
 * @param offset the current offset
 * @return the offset of the current region and the flag of successful
 */
template <typename FilePage>
typename file_region<FilePage>::result_type file_region<FilePage>::
    get_offset(const pos_type index, count_type& count, offset_type offset) const
{
    if (m_size > 0)
    {
        const size_type size = file_page_type::static_align_size(m_size);
        if (0 == m_count || m_count + count >= index)
        {
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
            return std::make_pair(size * (index - count) + offset, true);
#else
            return std::make_pair(size * (index - count) + offset, cached_region(offset, this));
#endif
        }
        count += m_count;
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
        return std::make_pair(m_count * size + offset, false);
#else
        return std::make_pair(m_count * size + offset, cached_region());
#endif
    }
    else
    {
        for (size_t i = 0; 0 == m_count || i < m_count; ++i)
        {
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const result_type result = it->get_offset(index, count, offset);
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
                if (result.second)
#else
                if (result.second.valid())
#endif
                {
                    return result;
                }
                offset = result.first;
            }
        }
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
        return std::make_pair(offset, false);
#else
        return std::make_pair(offset, cached_region());
#endif
    }
}

/**
 * Convert the raw offset to the real offset in the file
 * @param raw_offset the raw offset
 * @return the real offset in the file
 */
template <typename FilePage>
offset_type file_region<FilePage>::convert_offset(const offset_type raw_offset) const
{
#if !defined(OUROBOROS_FILE_REGION_CACHE_TYPE) || (OUROBOROS_FILE_REGION_CACHE_TYPE == 0)
    typename cache_type::iterator it = m_cache.lower_bound(raw_offset);
    if (it != m_cache.end() && (it->first == raw_offset || --it != m_cache.end()))
    {
        offset_type offset = raw_offset - it->first;
        return it->second.pregion->get_offset(offset, it->second.offset).first;
    }
    else if (!m_cache.empty())
    {
        typename cache_type::const_reverse_iterator rit = m_cache.rbegin();
        offset_type offset = raw_offset - rit->first;
        const result_type result = rit->second.pregion->get_offset(offset, rit->second.offset);
        if (result.second.valid())
        {
            return result.first;
        }
    }
    offset_type offset = raw_offset;
    const result_type result = get_offset(offset, 0);
    OUROBOROS_ASSERT(result.second.valid());
    offset = raw_offset - offset;
    it = m_cache.find(offset);
    if (it == m_cache.end())
    {
        m_cache.insert(it, std::make_pair(offset, result.second));
    }
    return result.first;
#elif OUROBOROS_FILE_REGION_CACHE_TYPE == 1
    if (!m_cache.valid())
    {
        offset_type offset = raw_offset;
        const result_type result = get_offset(offset, 0);
        OUROBOROS_ASSERT(result.second.valid());
        return result.first;
    }
    else
    {
        result_type cached_region = m_cache.get_cached_region(raw_offset);
        offset_type offset = raw_offset - cached_region.first;
        return cached_region.second.pregion->get_offset(offset, cached_region.second.offset).first;
    }
#elif OUROBOROS_FILE_REGION_CACHE_TYPE == 2
    offset_type offset = raw_offset;
    const result_type result = get_offset(offset, 0);
    OUROBOROS_ASSERT(result.second);
    return result.first;
#endif
}

/**
 * Convert the raw size to the real size in the file
 * @param raw_size the raw size
 * @return the real size in the file
 */
template <typename FilePage>
size_type file_region<FilePage>::convert_size(const size_type raw_size) const
{
    offset_type offset = raw_size;
    const result_type result = get_offset(offset, 0);
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
    OUROBOROS_ASSERT(offset == 0 || result.second);
#else
    OUROBOROS_ASSERT(offset == 0 || result.second.valid());
#endif
    return result.first;
}

/**
 * Get the real offset in the file
 * @param raw_offset the raw offset
 * @param offset the current offset
 * @return the real offset in the file
 */
template <typename FilePage>
typename file_region<FilePage>::result_type file_region<FilePage>::
    get_offset(offset_type& raw_offset, offset_type offset) const
{
    if (m_size > 0)
    {
        const size_type size = file_page_type::static_align_size(m_size);
        if (0 == m_count || raw_offset < m_count * m_size)
        {
            const count_type count = raw_offset / m_size;
            return std::make_pair(count * size +
                file_page_type::static_convert(raw_offset % m_size) + offset,
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
                true);
#else
                cached_region(offset, this));
#endif
        }
        raw_offset -= m_count * m_size;
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
        return std::make_pair(m_count * size + offset, false);
#else
        return std::make_pair(m_count * size + offset, cached_region());
#endif
    }
    else
    {
#if OUROBOROS_FILE_REGION_CACHE_TYPE == 2
        size_t regions_count = m_count;
        if (!m_cache.valid)
        {
            const offset_type prev_raw_offset = raw_offset;
            const offset_type prev_offset = offset;
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const result_type result = it->get_offset(raw_offset, offset);
                if (result.second)
                {
                    return result;
                }
                offset = result.first;
            }
            m_cache.raw_size = prev_raw_offset - raw_offset;
            m_cache.alligned_size = offset - prev_offset;
            --regions_count;
        }
        if (0 == m_count || raw_offset < regions_count * m_cache.raw_size)
        {
            const count_type count = raw_offset / m_cache.raw_size;
            offset += count * m_cache.alligned_size;
            raw_offset -= count * m_cache.raw_size;
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const result_type result = it->get_offset(raw_offset, offset);
                if (result.second)
                {
                    return result;
                }
                offset = result.first;
            }
            OUROBOROS_THROW_BUG("error: wrong result");
        }
        return std::make_pair(regions_count * m_cache.alligned_size + offset, false);
#else
        for (size_t i = 0; i < m_count || 0 == m_count; ++i)
        {
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const result_type result = it->get_offset(raw_offset, offset);
                if (result.second.valid())
                {
                    return result;
                }
                offset = result.first;
            }
        }
        return std::make_pair(offset, cached_region());
#endif
    }
}

#if !defined(OUROBOROS_FILE_REGION_CACHE_TYPE) || (OUROBOROS_FILE_REGION_CACHE_TYPE == 0)
/**
 * Make a cache
 * @param cache the cache
 * @param max_size the size of the file region
 * @param raw_offset the raw offset
 * @param offset the current offset
 * @return the real offset in the file
 */
template <typename FilePage>
typename file_region<FilePage>::result_type file_region<FilePage>::
    do_make_cache(cache_type& cache, offset_type max_size, offset_type& raw_offset, offset_type offset) const
{
    if (m_size > 0)
    {
        const size_type size = file_page_type::static_align_size(m_size);
        if (raw_offset < m_count * m_size || 0 == m_count)
        {
            const count_type count = raw_offset / m_size;
            return std::make_pair(count * size +
                file_page_type::static_convert(raw_offset % m_size) + offset,
                cached_region(offset, this));
        }
        cache.insert(std::make_pair(max_size - raw_offset, cached_region(offset, this)));
        raw_offset -= m_count * m_size;
        return std::make_pair(m_count * size + offset, cached_region());
    }
    else
    {
        for (size_t i = 0; i < m_count || 0 == m_count; ++i)
        {
            for (typename region_list::const_iterator it = m_regions.begin();
                    it != m_regions.end(); ++it)
            {
                const result_type result = it->do_make_cache(cache, max_size, raw_offset, offset);
                if (result.second.valid())
                {
                    return result;
                }
                offset = result.first;
            }
        }
        return std::make_pair(offset, cached_region());
    }
}
#endif

/**
 * Make a cache
 * @param size the size of the file region
 */
template <typename FilePage>
void file_region<FilePage>::make_cache(const size_type size) const
{
#if !defined(OUROBOROS_FILE_REGION_CACHE_TYPE) || (OUROBOROS_FILE_REGION_CACHE_TYPE == 0)
    m_cache.clear();
    offset_type offset = size;
    do_make_cache(m_cache, size, offset, 0);
#elif OUROBOROS_FILE_REGION_CACHE_TYPE == 1
    OUROBOROS_UNUSED(size);
    m_cache.make(*this);
#endif
}

//==============================================================================
//  status_file_page
//==============================================================================
/**
 * Constructor
 * @param ptr the pointer to data of page
 */
template <typename FilePage, typename Status>
status_file_page<FilePage, Status>::status_file_page(void *ptr)
{
    base_class::assign(ptr);
}

/**
 * Verify the page
 * @return the result of the checking
 */
template <typename FilePage, typename Status>
bool status_file_page<FilePage, Status>::verify() const
{
    return true;
}

/**
 * Set a status of the page
 * @param status the status of the page
 */
template <typename FilePage, typename Status>
void status_file_page<FilePage, Status>::set_status(const status_type& status)
{
    char *p = static_cast<char *>(base_class::get());
    memcpy(p + base_class::DATA_SIZE, &status, sizeof(status));
}

/**
 * Get a status of the page
 * @return the status of the page
 */
template <typename FilePage, typename Status>
typename status_file_page<FilePage, Status>::status_type
    status_file_page<FilePage, Status>::get_status() const
{
    const char *p = static_cast<const char *>(base_class::get());
    status_type status;
    memcpy(&status, p + base_class::DATA_SIZE, sizeof(status));
    return status;
}

}   //namespace ouroboros

#endif /* OUROBOROS_PAGE_H */
