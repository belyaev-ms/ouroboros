/**
 * @file   cachefile.h
 * The file with support of cache
 */

#ifndef OUROBOROS_CACHEFILE_H
#define	OUROBOROS_CACHEFILE_H

#include <string.h>

#include "ouroboros/filelock.h"
#include "ouroboros/cache.h"

namespace ouroboros
{

/**
 * The file with support of caching, with partial support of transactions
 * mechanism
 * @attention caching of write operation execute in local memory, when allocated
 * cache memory will be exhausted, the oldest cache page will be stored in
 * the file.
 */
template <int pageSize = 1024, int pageCount = 1024, typename File = file_lock,
        template <typename, int, int> class Cache = cache>
class cache_file : public File
{
    typedef File base_class;
public:
    enum
    {
        CACHE_PAGE_SIZE = pageSize,
        CACHE_PAGE_COUNT = pageCount
    };

    typedef Cache<cache_file, pageSize, pageCount> cache_type;
    typedef typename cache_type::page_status_type page_status_type;

    explicit cache_file(const std::string& name);
#ifdef OUROBOROS_TEST_ENABLED
    virtual ~cache_file()
    {
        reset();
    }
#endif
    inline void read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    void write(const void *buffer, size_type size, const pos_type pos); ///< write data
    const size_type resize(const size_type size); ///< change the size of the file

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    const transaction_state state() const; ///< get the state of the transaction
    void reset(); ///< reset all caches
    void refresh(size_type size, const pos_type pos); ///< refresh data

    virtual void save_page(const pos_type index, void *data); ///< save data of the cache page
protected:
    virtual void *get_page(const pos_type index); ///< get the buffer of the cache page
    virtual void *get_page(const pos_type index) const; ///< get the buffer of the cache page
    void do_read(void *buffer, size_type size, pos_type pos) const; ///< read data
    void do_write(const void *buffer, size_type size, pos_type pos); ///< write data
protected:
    cache_type m_cache; ///< the cache
    transaction_state m_trans; ///< the state of the transaction
};

//==============================================================================
//  cache_file
//==============================================================================
/**
 * Constructor
 * @param name the name of the file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
inline cache_file<pageSize, pageCount, File, Cache>::cache_file(const std::string& name) :
    base_class(name),
    m_cache(*this),
    m_trans(TR_STOPPED)
{
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
inline void cache_file<pageSize, pageCount, File, Cache>::read(void *buffer, size_type size, const pos_type pos) const
{
    do_read(buffer, size, pos);
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::do_read(void *buffer, size_type size, pos_type pos) const
{
    const pos_type beg = pos;  // the first byte for read
    const pos_type end = beg + size - 1; // the last byte for read
    const pos_type index0 = beg / CACHE_PAGE_SIZE; // the index that has the first byte
    const pos_type index1 = end / CACHE_PAGE_SIZE; // the index that has the last byte

    pos_type offset = pos % CACHE_PAGE_SIZE; // the position in the cache page for read the first byte
    // check the cache page has full data for read
    if (index0 == index1)
    {
        void *page = get_page(index0);
        memcpy(buffer, static_cast<char *>(page) + offset, size);
    }
    else
    {
        // read data from the first cache page
        size_type size0 = CACHE_PAGE_SIZE - offset; // size of useful data in the first cache page
        void *page = get_page(index0);
        memcpy(buffer, static_cast<char *>(page) + offset, size0);
        buffer = static_cast<char *>(buffer) + size0;
        // read data from next cache pages
        for (pos_type i = index0 + 1; i < index1; ++i)
        {
            page = get_page(i);
            memcpy(buffer, page, CACHE_PAGE_SIZE);
            buffer = static_cast<char *>(buffer) + CACHE_PAGE_SIZE;
        }
        // read data from the last cache page
        size_type size1 = end % CACHE_PAGE_SIZE + 1; // size of useful data in the last cache page
        page = get_page(index1);
        memcpy(buffer, page, size1);
    }
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::write(const void *buffer, size_type size, const pos_type pos)
{
    // check the transaction is active
    if (TR_STARTED != m_trans)
    {
        // there isn't the transaction, write data to the file
        base_class::write(buffer, size, pos);
    }
    // write data to the cache
    do_write(buffer, size, pos);
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::do_write(const void *buffer, size_type size, pos_type pos)
{
    const pos_type beg = pos;  // the first byte for write
    const pos_type end = beg + size - 1; // the last byte for write
    const pos_type index0 = beg / CACHE_PAGE_SIZE; // the index that has the first byte
    const pos_type index1 = end / CACHE_PAGE_SIZE; // the index that has the last byte

    pos_type offset = pos % CACHE_PAGE_SIZE; // the position in the cache page for write the first byte
    // check the cache page has full data for write
    if (index0 == index1)
    {
        void *page = get_page(index0);
        if (page != NULL)
        {
            memcpy(static_cast<char *>(page) + offset, buffer, size);
        }
    }
    else
    {
        // write data from the first cache page
        size_type size0 = CACHE_PAGE_SIZE - offset; // size of useful data in the first cache page
        void *page = get_page(index0);
        if (page != NULL)
        {
            memcpy(static_cast<char *>(page) + offset, buffer, size0);
        }
        buffer = static_cast<const char *>(buffer) + size0;
        // write data from next cache pages
        for (pos_type i = index0 + 1; i < index1; ++i)
        {
            page = get_page(i);
            if (page != NULL)
            {
                memcpy(page, buffer, CACHE_PAGE_SIZE);
            }
            buffer = static_cast<const char *>(buffer) + CACHE_PAGE_SIZE;
        }
        // write data from the last cache page
        size_type size1 = end % CACHE_PAGE_SIZE + 1; // size of useful data in the last cache page
        page = get_page(index1);
        if (page != NULL)
        {
            memcpy(page, buffer, size1);
        }
    }
}

/**
 * Change the size of the file
 * @param size the size of the file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
const size_type cache_file<pageSize, pageCount, File, Cache>::resize(const size_type size)
{
    const size_type rsize = m_cache.aligned_size(size);
    base_class::resize(rsize);
    return rsize;
}

/**
 * Get the buffer of the cache page for read
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for read
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void *cache_file<pageSize, pageCount, File, Cache>::get_page(const pos_type index) const
{
    const page_status_type status = m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        return m_cache.get_page(status);
    }
    else
    {
        void *page = m_cache.get_page(status);
        base_class::read(page, CACHE_PAGE_SIZE, status.index() * CACHE_PAGE_SIZE);
        return page;
    }
}

/**
 * Get the buffer of the cache page for write
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for write
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void *cache_file<pageSize, pageCount, File, Cache>::get_page(const pos_type index)
{
    const page_status_type status = m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        return m_cache.get_page(status);
    }
    else
    {
        void *page = m_cache.get_page(status);
        base_class::read(page, CACHE_PAGE_SIZE, status.index() * CACHE_PAGE_SIZE);
        return page;
    }
}

/**
 * Start the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::start()
{
    if (TR_STARTED == m_trans)
    {
        OUROBOROS_THROW_BUG("there already is some transaction");
    }
    m_trans = TR_STARTED;
}

/**
 * Stop the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::stop()
{
    if (TR_STARTED != m_trans)
    {
        OUROBOROS_THROW_BUG("there isn't any transaction");
    }
    m_trans = TR_STOPPED;
    m_cache.clean();
}

/**
 * Cancel the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::cancel()
{
    if (TR_STARTED != m_trans)
    {
        OUROBOROS_THROW_BUG("there isn't any transaction");
    }
    m_trans = TR_CANCELED;
    m_cache.clean();
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
inline const transaction_state cache_file<pageSize, pageCount, File, Cache>::state() const
{
    return m_trans;
}

/**
 * Save data of the cache page
 * @param index the index of the cache page
 * @param page the cache page
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::save_page(const pos_type index, void *page)
{
    OUROBOROS_ASSERT(page != NULL);
    if (TR_CANCELED != m_trans)
    {
        base_class::write(page, CACHE_PAGE_SIZE, index * CACHE_PAGE_SIZE);
    }
}

/**
 * Reset all caches
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::reset()
{
    m_trans = TR_CANCELED;
    m_cache.free();
    m_trans = TR_STOPPED;
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void cache_file<pageSize, pageCount, File, Cache>::refresh(size_type size, const pos_type pos)
{
    OUROBOROS_ASSERT(m_trans != TR_STARTED || m_cache.empty());
    const pos_type beg = pos / CACHE_PAGE_SIZE;
    const pos_type end = (pos + size) / CACHE_PAGE_SIZE + 1;
    for (pos_type i = beg; i < end; ++i)
    {
        m_cache.free_page(i);
    }
}

}   //namespace ouroboros

#endif	/* OUROBOROS_CACHEFILE_H */

