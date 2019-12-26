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
template <typename FilePage, int pageCount = 1024, typename File = file_lock<FilePage>,
    template <typename, int, int> class Cache = cache>
class cache_file : public File
{
    typedef File base_class;
public:
    typedef FilePage file_page_type;
    typedef typename base_class::file_region_type file_region_type;
    enum
    {
        CACHE_PAGE_SIZE = file_page_type::TOTAL_SIZE,
        CACHE_PAGE_COUNT = pageCount
    };

    typedef Cache<cache_file, CACHE_PAGE_SIZE, CACHE_PAGE_COUNT> cache_type;
    typedef typename cache_type::page_status_type page_status_type;

    explicit cache_file(const std::string& name);
    cache_file(const std::string& name, const file_region_type& region);

#ifdef OUROBOROS_TEST_ENABLED
    virtual ~cache_file()
    {
        reset();
    }
#endif

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    transaction_state state() const; ///< get the state of the transaction
    void reset(); ///< reset the cache

    virtual void save_page(const pos_type index, void *data); ///< save data of the cache page
protected:
    void clean(); ///< clean dirty pages of a cache
    virtual void do_read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    virtual void do_write(const void *buffer, size_type size, const pos_type pos); ///< write data
    virtual void do_refresh(size_type size, const pos_type pos); ///< refresh data
    virtual void *get_page(const pos_type index); ///< get the buffer of the cache page
    virtual void *get_page(const pos_type index) const; ///< get the buffer of the cache page
    virtual void save_page(const file_page_type& page); /// save data of the file page
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
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
cache_file<FilePage, pageCount, File, Cache>::cache_file(const std::string& name) :
    base_class(name),
    m_cache(*this),
    m_trans(TR_STOPPED)
{
}

/**
 * Constructor
 * @param name the name of a file
 * @param region the region of a file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
cache_file<FilePage, pageCount, File, Cache>::cache_file(const std::string& name,
        const file_region_type& region) :
    base_class(name, region),
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
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::
    do_read(void *buffer, size_type size, const offset_type pos) const
{
    file_page_type page0(pos);
    file_page_type page1(pos, size - 1);
    if (page0 == page1)
    {
        page0.assign(get_page(page0.index()));
        page0.read(buffer, size);
    }
    else
    {
        for (file_page_type page = page0; page < page1; ++page)
        {
            page.assign(get_page(page.index()));
            buffer = page.read(buffer);
        }
        page1.assign(get_page(page1.index()));
        page1.read_rest(buffer);
    }
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::
    do_write(const void *buffer, size_type size, const pos_type pos)
{
    file_page_type page0(pos);
    file_page_type page1(pos, size - 1);
    if (page0 == page1)
    {
        page0.assign(get_page(page0.index()));
        page0.write(buffer, size);
        save_page(page0);
    }
    else
    {
        for (file_page_type page = page0; page < page1; ++page)
        {
            page.assign(get_page(page.index()));
            buffer = page.write(buffer);
            save_page(page);
        }
        page1.assign(get_page(page1.index()));
        page1.write_rest(buffer);
        save_page(page1);
    }
}

/**
 * Get the buffer of the cache page for read
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for read
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void *cache_file<FilePage, pageCount, File, Cache>::get_page(const pos_type index) const
{
    const page_status_type status = m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        return m_cache.get_page(status);
    }
    else
    {
        void *page = m_cache.get_page(status);
        base_class::do_read(page, CACHE_PAGE_SIZE, status.index() * CACHE_PAGE_SIZE);
        return page;
    }
}

/**
 * Get the buffer of the cache page for write
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for write
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void *cache_file<FilePage, pageCount, File, Cache>::get_page(const pos_type index)
{
    const page_status_type status = m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        return m_cache.get_page(status);
    }
    else
    {
        void *page = m_cache.get_page(status);
        base_class::do_read(page, CACHE_PAGE_SIZE, status.index() * CACHE_PAGE_SIZE);
        return page;
    }
}

/**
 * Start the transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::start()
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
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::stop()
{
    if (TR_STARTED != m_trans)
    {
        OUROBOROS_THROW_BUG("there isn't any transaction");
    }
    /*@attention if the line will be executed after the next line then
     * your data may be lost! */
    m_cache.clean();
    m_trans = TR_STOPPED;
}

/**
 * Cancel the transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::cancel()
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
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
inline transaction_state cache_file<FilePage, pageCount, File, Cache>::state() const
{
    return m_trans;
}

/**
 * Save data of the cache page
 * @param index the index of the cache page
 * @param page the cache page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::save_page(const pos_type index, void *page)
{
    OUROBOROS_ASSERT(page != NULL);
    if (TR_CANCELED != m_trans)
    {
        base_class::do_write(page, CACHE_PAGE_SIZE, index * CACHE_PAGE_SIZE);
    }
}

/**
 * Save data of the cache page
 * @param page the file page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::save_page(const file_page_type& page)
{
    if (TR_STARTED != m_trans)
    {
        base_class::do_write(page.get(), CACHE_PAGE_SIZE, page.index() * CACHE_PAGE_SIZE);
    }
}

/**
 * Reset the cache
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::reset()
{
    m_trans = TR_CANCELED;
    m_cache.free();
    m_trans = TR_STOPPED;
}

/**
 * Clean dirty pages of a cache
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::clean()
{
    m_cache.clean();
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void cache_file<FilePage, pageCount, File, Cache>::do_refresh(size_type size, const pos_type pos)
{
    OUROBOROS_ASSERT(m_trans != TR_STARTED || !m_cache.dirty());
    const pos_type beg = pos / CACHE_PAGE_SIZE;
    const pos_type end = (pos + size) / CACHE_PAGE_SIZE + 1;
    for (pos_type i = beg; i < end; ++i)
    {
        m_cache.free_page(i);
    }
}

}   //namespace ouroboros

#endif	/* OUROBOROS_CACHEFILE_H */

