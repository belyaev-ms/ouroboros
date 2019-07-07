/**
 * @file   cache.h
 * The classes for cache data
 */

#ifndef OUROBOROS_CACHE_H
#define	OUROBOROS_CACHE_H

#include <stddef.h>
#include <string>

#include "ouroboros/global.h"
#include "ouroboros/object.h"
#include "ouroboros/hashmap.h"

namespace ouroboros
{

/** the state of the cache page */
enum page_state
{
    PG_DETACHED,    ///< the page detached
    PG_ATTACHED,    ///< the page attached
    PG_DIRTY,       ///< the page is dirty
};

/**
 * The status of a cache page
 */
template <typename PPage>
class page_status
{
    template <typename, int, int> friend class cache;
public:
    typedef PPage page_pointer;
    page_status(const pos_type index, page_pointer page);
    inline const page_state state() const; ///< get the state of the page
    inline const pos_type index() const; ///< get the index of the page
protected:
    inline page_pointer page() const; ///< get the pointer to the page
private:
    page_status();
private:
    const pos_type m_index; ///< the index of the page
    page_pointer m_page; ///< the pointer to the page
};

/**
 * The cache page is an element of a circular doubly linked list
 * that has pointers to data whose size is equal the pageSize argument
 */
template <typename Cache, int pageSize = 1024>
class cache_page
{
    template <typename, int, int> friend class cache;
public:
    enum {SIZE = pageSize};
    typedef Cache cache_type;
    typedef typename cache_type::page_status_type page_status_type;

    inline cache_page(cache_page *prev, cache_page *next);
    inline cache_page();
    inline ~cache_page();

    inline void prev(cache_page *page); ///< set the previous page
    inline void next(cache_page *page); ///< set the next page
    inline cache_page* prev() const; ///< get the previous page
    inline cache_page* next() const; ///< get the next page
    inline void* Data(); ///< get data of the page
    inline void* Data() const; ///< get data of the page
    inline void free(); ///< release the page from the pool
    inline void attach(cache_type& cache, const pos_type index); ///< attach the page to the cache
    inline void detach(); ///< detach the page from the cache
    inline const bool attached(const cache_type& cache) const; ///< check the page is attached to the cache
    inline const pos_type index() const; /// get the index of the page
    inline const page_state state() const; ///< get the state of the page
    inline const page_status_type status(); ///< get the status of the page
    inline void dirty(); ///< dirty the page
    inline void clean(); ///< clean the page
protected:
    typedef typename cache_type::iterator iterator;
    inline void detach(const iterator& it); ///< detach the page from the cache
    inline void clean(const iterator& it); ///< clean the page in the cache
    inline void reset(); ///< reset the page
private:
    cache_page *m_prev; ///< the previous page
    cache_page *m_next; ///< the next page
    cache_type *m_cache; ///< the cache
    pos_type m_index; ///< the index of the page
    bool m_dirty; ///< the sign of dirty data
    char m_data[SIZE]; ///< data of the page
};

/**
 * The page pool is a circular doubly linked list that has cache pages whose
 * amount is equal the pageCount argument
 */
template <typename Cache, int pageSize = 1024, int pageCount = 1024>
class cache_pool
{
public:
    enum {COUNT = pageCount};
    typedef Cache cache_type;
    typedef cache_page<cache_type, pageSize> page_type;

    inline cache_pool();

    inline page_type* make_page(); ///< make the new page of the cache
    inline void up_page(page_type *page); ///< increment the raiting of the page
    inline void init(); ///< initialize the pool
#ifdef OUROBOROS_TEST_ENABLED
    const count_type test_page_count() const; ///< test the count of the cache page
    const pos_type test_page_index(page_type *page) const; ///< test the index of the cache page
#endif
private:
    page_type m_pages[pageCount];
    page_type *m_beg;
    page_type *m_end;
};

/**
 * The cache
 * The size of the page is determined pageSize argument
 * The count of the page is determined pageCount argument
 * The amount of the page required for a full cache object is determined
 * when the cache is created
 */
template <typename Saver, int pageSize = 1024, int pageCount = 1024>
class cache
{
    friend class cache_page<cache<Saver, pageSize, pageCount>, pageSize>;
public:
    enum
    {
        CACHE_PAGE_SIZE = pageSize,   ///< the size of the page
        CACHE_PAGE_COUNT = pageCount  ///< the count of the pages
    };
    typedef Saver saver_type;
    typedef cache<Saver, pageSize, pageCount> cache_type;
    typedef cache_pool<cache_type, pageSize, pageCount> page_pool_type;
    typedef cache_page<cache_type, pageSize> page_type;
    typedef page_status<page_type *> page_status_type;

    inline cache();
    inline cache(saver_type& saver);

    inline page_status_type page_exists(const pos_type index) const; ///< check the page exists
    inline void* get_page(const pos_type index); ///< get data of the page
    inline void* get_page(const page_status_type& status); ///< get data of the page
    inline void* get_page(const pos_type index) const; ///< get data of the page
    inline void* get_page(const page_status_type& status) const; ///< get data of the page
    inline const size_type aligned_size(const size_type size); ///< change the size of the cache object
    inline const size_type size() const; ///< get the size of the cache
    inline const bool empty() const; ///< chech the cache is empty
#ifdef OUROBOROS_TEST_ENABLED
    const count_type test_pool_page_count() const; ///< test the count of the page in the cache pool
#endif
    inline void clean(); ///< clean all dirty pages
    inline void free(); ///< release the cache
    inline void free_page(const pos_type index); ///< release the page from the pool
protected:
    typedef hash_map<pos_type, page_type *, pageCount> page_list;
    typedef typename page_list::iterator iterator;
    inline page_type* do_page_exists(const pos_type index) const; ///< check the page exists
    inline page_type* do_get_page(const pos_type index) const; ///< get the page
    inline page_type* do_get_page(const page_status_type& status) const; ///< get the page
    inline const count_type calc_page_count(const size_type size) const; ///< calculate the count of pages required for the full object
    inline const count_type get_page_count() const; ///< get the count of the pages
    inline void detach(const pos_type index); ///< detach the page from the cache
    inline void detach(const iterator& it); ///< detach the page from the cache
    inline void dirty(page_type *page); ///< dirty the page
    inline void clean(page_type *page); ///< clean the page
    inline void clean(const iterator& it); ///< clean the page
    void save_page(const page_type *page); ///< save the page
    inline void init_pool(); ///< initialize the cache pool
private:
    saver_type *m_saver; ///< the saver of the pages
    mutable page_pool_type m_pool; ///< the page pool
    mutable page_list m_pages; ///< the map of the pages
    page_list m_dirty_pages; ///< the map of the dirty pages
};

/**
 * Calculate the count of pages required for the full object
 * @param size the size of the object
 * @param pageSize size of the cache page
 * @return the count of pages
 */
inline const count_type calc_cache_page_count(const size_type size, const size_type pageSize)
{
    count_type count = size / pageSize;
    if (size % pageSize != 0)
    {
        ++count;
    }
    return count != 0 ? count : 1;
}

/**
 * Calculate the size of the cache required for the full object
 * @param size the size of the cache
 * @param pageSize size of the cache page
 * @return the size of the cache required for the full object
 */
inline const size_type calc_cache_size(const size_type size, const size_type pageSize)
{
    const count_type count = calc_cache_page_count(size, pageSize);
    return pageSize * count;
}

//==============================================================================
//  page_status
//==============================================================================
/**
 * Constructor
 * @param index the index of the page
 * @param page the pointer to the page
 */
template <typename PPage>
page_status<PPage>::page_status(const pos_type index, page_pointer page) :
    m_index(index),
    m_page(page)
{
}

/**
 * Get the state of the page
 * @return the state of the page
 */
template <typename PPage>
inline const page_state page_status<PPage>::state() const
{
    return NULL == m_page ? PG_DETACHED : m_page->state();
}

/**
 * Get the pointer to the page
 * @return the pointer to the page
 */
template <typename PPage>
inline typename page_status<PPage>::page_pointer page_status<PPage>::page() const
{
    return m_page;
}

/**
 * Get the index to the page
 * @return the index to the page
 */
template <typename PPage>
inline const pos_type page_status<PPage>::index() const
{
    return m_index;
}

//==============================================================================
//  cache_page
//==============================================================================
/**
 * Constructor
 * @param prev the previous page
 * @param next the next page
 */
template <typename Cache, int pageSize>
inline cache_page<Cache, pageSize>::cache_page(cache_page *prev, cache_page *next) :
    m_prev(prev),
    m_next(next),
    m_cache(NULL),
    m_index(NIL),
    m_dirty(false)
{
}

/**
 * Constructor
 */
template <typename Cache, int pageSize>
inline cache_page<Cache, pageSize>::cache_page() :
    m_prev(NULL),
    m_next(NULL),
    m_cache(NULL),
    m_index(NIL),
    m_dirty(false)
{
}

/**
 * Destructor
 */
template <typename Cache, int pageSize>
inline cache_page<Cache, pageSize>::~cache_page()
{
}

/**
 * Set the previous page
 * @param page the previous page
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::prev(cache_page *page)
{
    m_prev = page;
}

/**
 * Set the next page
 * @param page the next page
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::next(cache_page *page)
{
    m_next = page;
}

/**
 * Get the previous page
 * @return the previous page
 */
template <typename Cache, int pageSize>
inline cache_page<Cache, pageSize>* cache_page<Cache, pageSize>::prev() const
{
    return m_prev;
}

/**
 * Get the next page
 * @return the next page
 */
template <typename Cache, int pageSize>
inline cache_page<Cache, pageSize>* cache_page<Cache, pageSize>::next() const
{
    return m_next;
}

/**
 * Get data of the page
 * @return data of the page
 */
template <typename Cache, int pageSize>
inline void* cache_page<Cache, pageSize>::Data()
{
    return m_data;
}

/**
 * Get data of the page
 * @return data of the page
 */
template <typename Cache, int pageSize>
inline void* cache_page<Cache, pageSize>::Data() const
{
    return const_cast<char *>(m_data);
}

/**
 * Release the page from the pool
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::free()
{
    m_prev->next(m_next);
    m_next->prev(m_prev);
}

/**
 * Attach the page to the cache
 * @param cache the cache
 * @param index the index of the page in the cache
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::attach(cache_type& cache, const pos_type index)
{
    m_cache = &cache;
    m_index = index;
}

/**
 * Detach the page from the cache
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::detach()
{
    if (m_cache != NULL)
    {
        m_cache->detach(m_index);
        reset();
    }
}

/**
 * Detach the page from the cache
 * @param it the page iterator of the cache
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::detach(const iterator& it)
{
    if (m_cache != NULL)
    {
        m_cache->detach(it);
        reset();
    }
}

/**
 * Check the page is attached to the cache
 * @param cache the cache
 * @return the result of the checking
 */
template <typename Cache, int pageSize>
inline const bool cache_page<Cache, pageSize>::attached(const cache_type& cache) const
{
    return m_cache == &cache;
}

/**
 * Get the index of the page
 * @return the index of the page
 */
template <typename Cache, int pageSize>
inline const pos_type cache_page<Cache, pageSize>::index() const
{
    return m_index;
}

/**
 * Get the state of the page
 * @return the state of the page
 */
template <typename Cache, int pageSize>
inline const page_state cache_page<Cache, pageSize>::state() const
{
    return NULL == m_cache ? PG_DETACHED : m_dirty ? PG_DIRTY : PG_ATTACHED;
}

/**
 * Get the status of the page
 * @return the status of the page
 */
template <typename Cache, int pageSize>
inline const typename cache_page<Cache, pageSize>::page_status_type
    cache_page<Cache, pageSize>::status()
{
    return page_status_type(m_cache != NULL ? this : NULL);
}

/**
 * Dirty the page
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::dirty()
{
    if (!m_dirty && m_cache != NULL)
    {
        m_dirty = true;
        m_cache->dirty(this);
    }
}

/**
 * Clean the page
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::clean()
{
    if (m_dirty && m_cache != NULL)
    {
        m_dirty = false;
        m_cache->clean(this);
    }
}

/**
 * Clean the page in the cache
 * @param it the dirty page iterator of the cache
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::clean(const iterator& it)
{
    if (m_cache != NULL)
    {
        m_dirty = false;
        m_cache->clean(it);
    }
}

/**
 * Reset the page
 */
template <typename Cache, int pageSize>
inline void cache_page<Cache, pageSize>::reset()
{
    m_cache = NULL;
    m_index = NIL;
    m_dirty = false;
}

//==============================================================================
//  cache_pool
//==============================================================================
/**
 * Constructor
 */
template <typename Cache, int pageSize, int pageCount>
inline cache_pool<Cache, pageSize, pageCount>::cache_pool()
{
    init();
}

/**
 * Initialize the pool
 */
template <typename Cache, int pageSize, int pageCount>
inline void cache_pool<Cache, pageSize, pageCount>::init()
{
    m_beg = &m_pages[0];
    m_end = NULL;

    m_pages[0].next(&m_pages[1]);
    m_pages[0].prev(&m_pages[pageCount - 1]);
    for (pos_type i = 1; i < pageCount - 1; ++i)
    {
        m_pages[i].prev(&m_pages[i - 1]);
        m_pages[i].next(&m_pages[i + 1]);
    }
    m_pages[pageCount - 1].prev(&m_pages[pageCount - 2]);
    m_pages[pageCount - 1].next(&m_pages[0]);
}

/**
 * Make the new page of the cache
 * @return the new page of the cache
 */
template <typename Cache, int pageSize, int pageCount>
inline typename cache_pool<Cache, pageSize, pageCount>::page_type*
    cache_pool<Cache, pageSize, pageCount>::make_page()
{
    if (m_end == m_beg)
    {
        m_beg->detach();
        m_beg = m_beg->next();
    }
    m_end = m_end != NULL ? m_end->next() : m_beg->next();
    return m_end->prev();
}

/**
 * Increment the raiting of the page
 * @param page the page of the cache
 */
template <typename Cache, int pageSize, int pageCount>
inline void cache_pool<Cache, pageSize, pageCount>::up_page(page_type *page)
{
    page_type *tail = m_end->prev();
    if (page == tail)
    {
        return;
    }
    if (page == m_beg)
    {
        if (m_end == m_beg)
        {
            m_end = m_end->next();
        }
        m_beg = m_beg->next();
    }
    page->free();
    tail->next(page);
    page->prev(tail);
    page->next(m_end);
    m_end->prev(page);
}

#ifdef OUROBOROS_TEST_ENABLED
/**
 * Test the count of the cache page
 * @return the count of the cache pages
 */
template <typename Cache, int pageSize, int pageCount>
const count_type cache_pool<Cache, pageSize, pageCount>::test_page_count() const
{
    page_type *page = m_beg;
    count_type count = 0;
    if (m_end != NULL)
    {
        do
        {
            std::cout << test_page_index(page) << "->";
            page = page->next();
            ++count;
            if (count > pageCount)
            {
                std::cout << "Error";
                return count;
            }
        } while (page != m_end);
        std::cout << test_page_index(page) << std::endl;
    }
    return count;
}

/**
 * Test the index of the cache page
 * @param page the page of the cache
 * @return the index of the page
 */
template <typename Cache, int pageSize, int pageCount>
const pos_type cache_pool<Cache, pageSize, pageCount>::test_page_index(page_type *page) const
{
    return pos_type(page - m_pages);
}
#endif

//==============================================================================
//  cache
//==============================================================================
/**
 * Constructor
 */
template <typename Saver, int pageSize, int pageCount>
inline cache<Saver, pageSize, pageCount>::cache() :
    m_saver(NULL)
{
}

/**
 * Constructor
 * @param saver the saver of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline cache<Saver, pageSize, pageCount>::cache(saver_type& saver) :
    m_saver(&saver)
{
}

/**
 * Check the page exists
 * @param index the index of the page
 * @return the pointer to the page
 */
template <typename Saver, int pageSize, int pageCount>
inline typename cache<Saver, pageSize, pageCount>::page_type*
    cache<Saver, pageSize, pageCount>::do_page_exists(const pos_type index) const
{
    const typename page_list::const_iterator it = m_pages.find(index);
    return m_pages.end() == it ? NULL : it->second;
}

/**
 * Check the page exists
 * @param index the index of the page
 * @return the status of the cache page
 */
template <typename Saver, int pageSize, int pageCount>
inline typename cache<Saver, pageSize, pageCount>::page_status_type
    cache<Saver, pageSize, pageCount>::page_exists(const pos_type index) const
{
    return page_status_type(index, do_page_exists(index));
}

/**
 * Get the page
 * @param the index of the page
 * @return the pointer to the page
 */
template <typename Saver, int pageSize, int pageCount>
inline typename cache<Saver, pageSize, pageCount>::page_type*
    cache<Saver, pageSize, pageCount>::do_get_page(const pos_type index) const
{
    const page_status_type status = page_exists(index);
    return do_get_page(status);
}

/**
 * Get the page
 * @param status the status of the page
 * @return the pointer to the page
 */
template <typename Saver, int pageSize, int pageCount>
inline typename cache<Saver, pageSize, pageCount>::page_type*
    cache<Saver, pageSize, pageCount>::do_get_page(const page_status_type& status) const
{
    page_type *page = status.page();
    if (page != NULL)
    {
        m_pool.up_page(page);
    }
    else
    {
        const pos_type index = status.index();
        page = m_pool.make_page();
        m_pages.insert(std::make_pair(index, page));
        page->attach(*const_cast<cache<Saver, pageSize, pageCount> *>(this), index);
    }
    return page;
}

/**
 * Get data of the page
 * @param the index of the page
 * @return data of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void* cache<Saver, pageSize, pageCount>::get_page(const pos_type index)
{
    page_type *page = do_get_page(index);
    page->dirty();
    return page->Data();
}

/**
 * Get data of the page
 * @param status the status of the page
 * @return data of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void* cache<Saver, pageSize, pageCount>::get_page(const page_status_type& status)
{
    page_type *page = do_get_page(status);
    page->dirty();
    return page->Data();
}

/**
 * Get data of the page
 * @param the index of the page
 * @return data of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void* cache<Saver, pageSize, pageCount>::get_page(const pos_type index) const
{
    return do_get_page(index)->Data();
}

/**
 * Get data of the page
 * @param status the status of the page
 * @return data of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void* cache<Saver, pageSize, pageCount>::get_page(const page_status_type& status) const
{
    return do_get_page(status)->Data();
}

/**
 * dirty the page
 * @param page the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::dirty(page_type *page)
{
    m_dirty_pages.insert(std::make_pair(page->index(), page));
}

/**
 * Clean the page
 * @param page the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::clean(page_type *page)
{
    save_page(page);
    m_dirty_pages.erase(page->index());
}

/**
 * Clean the page
 * @param it the dirty page iterator of the cache
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::clean(const iterator& it)
{
    save_page(it->second);
    m_dirty_pages.erase(it);
}

/**
 * Get the aligned size of object
 * @param size the size of object
 * @return the aligned size of object
 */
template <typename Saver, int pageSize, int pageCount>
inline const size_type cache<Saver, pageSize, pageCount>::aligned_size(const size_type size)
{
    const count_type count = calc_page_count(size);
    return pageSize * count;
}

/**
 * Get the size of the cache
 * @return the size of the cache
 */
template <typename Saver, int pageSize, int pageCount>
inline const size_type cache<Saver, pageSize, pageCount>::size() const
{
    return pageSize * get_page_count();
}

/**
 * Check the cache is empty
 * @return the result of the checking
 */
template <typename Saver, int pageSize, int pageCount>
inline const bool cache<Saver, pageSize, pageCount>::empty() const
{
    return m_pages.empty();
}

#ifdef OUROBOROS_TEST_ENABLED
/**
 * Test the count of the page in the cache pool
 * @return the count of the pages in the cache pool
 */
template <typename Saver, int pageSize, int pageCount>
const count_type cache<Saver, pageSize, pageCount>::test_pool_page_count() const
{
    return m_pool.test_page_count();
}
#endif

/**
 * Calculate the count of pages required for the full object
 * @param size the size of the object
 * @return the count of pages
 */
template <typename Saver, int pageSize, int pageCount>
inline const count_type cache<Saver, pageSize, pageCount>::calc_page_count(const size_type size) const
{
    return calc_cache_page_count(size, pageSize);
}

/**
 * Get the count of the pages
 * @return the count of the pages
 */
template <typename Saver, int pageSize, int pageCount>
inline const count_type cache<Saver, pageSize, pageCount>::get_page_count() const
{
    return m_pages.size();
}

/**
 * Initialize the cache pool
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::init_pool()
{
    m_pool.init();
}

/**
 * Detach the page from the cache
 * @param index the index of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::detach(const pos_type index)
{
    iterator it = cache_type::m_pages.find(index);
    if (it != cache_type::m_pages.end())
    {
        detach(it);
    }
}

/**
 * Detach the page from the cache
 * @param it the page iterator of the cache
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::detach(const iterator& it)
{
    if (it->second->state() == PG_DIRTY)
    {
        save_page(it->second);
        m_dirty_pages.erase(it->first);
    }
    m_pages.erase(it);
}

/**
 * Save the page
 * @param page the page
 */
template <typename Saver, int pageSize, int pageCount>
void cache<Saver, pageSize, pageCount>::save_page(const page_type *page)
{
    if (m_saver != NULL)
    {
        m_saver->save_page(page->index(), page->Data());
    }
}

/**
 * Clean all dirty pages
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::clean()
{
    iterator it = m_dirty_pages.begin();
    while (it != m_dirty_pages.end())
    {
        page_type *page = it->second;
        page->clean(it);
        it = m_dirty_pages.begin();
    }
}

/**
 * Release the cache
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::free()
{
    iterator it = m_pages.begin();
    while (it != m_pages.end())
    {
        page_type *page = it->second;
        page->detach(it);
        it = m_pages.begin();
    }
    init_pool();
}

/**
 * Release the page from the pool
 * @param index the index of the page
 */
template <typename Saver, int pageSize, int pageCount>
inline void cache<Saver, pageSize, pageCount>::free_page(const pos_type index)
{
    iterator it = m_pages.find(index);
    if (it != m_pages.end())
    {
        page_type *page = it->second;
        page->detach(it);
    }
}

}   //namespace ouroboros

#endif	/* OUROBOROS_CACHE_H */

