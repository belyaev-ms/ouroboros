#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE cache_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/interface.h"
#include "ouroboros/cache.h"

using namespace ouroboros;

/**
 * Helper for testing the write cache
 */
template <int pageSize, int pageCount>
struct CacheSaver
{
    typedef std::set<pos_type> page_list;
    page_list m_pages;
    char m_data[pageSize * pageCount];
    void save_page(const pos_type index, void *page)
    {
        BOOST_TEST_MESSAGE("the page " << index << " saved");
        m_pages.insert(index);
        memcpy(&m_data[pageSize * index], page, pageSize);
    }
    void clear()
    {
        memset(m_data, 0, sizeof(m_data));
        m_pages.clear();
    }
    bool empty()
    {
        return m_pages.empty();
    }
    const bool CheckPage(const pos_type index, const char *data = NULL)
    {
        const page_list::iterator it = m_pages.find(index);
        if (m_pages.end() == it)
        {
            BOOST_TEST_MESSAGE("page " << index << " not found");
            return false;
        }
        else
        {
            if (data != NULL)
            {
                const size_t sn = m_data[index * pageSize];
                const size_t dn = data[index * pageSize];

                BOOST_TEST_MESSAGE(PR(index) << PR(sn) << PE(dn));
                BOOST_REQUIRE(memcmp(&m_data[index * pageSize], &data[index * pageSize], pageSize) == 0);
            }
            m_pages.erase(it);
            memset(&m_data[index * pageSize], 0, pageSize);
            return true;
        }
    }
    void CheckPages(const pos_type beg, const pos_type end, const char *data = NULL)
    {
        for (pos_type i = beg; i < end; ++i)
        {
            BOOST_TEST_MESSAGE("check page " << i);
            BOOST_REQUIRE(CheckPage(i, data));
        }
        for (page_list::const_iterator it = m_pages.begin(); it != m_pages.end(); ++it)
        {
            const pos_type index = *it;
            BOOST_TEST_MESSAGE("dirty page " << index << " saved");
            BOOST_CHECK(memcmp(&m_data[index * pageSize], &data[index * pageSize], pageSize) == 0);
        }
        BOOST_REQUIRE(m_pages.empty());
    }
};

/**
 * Fill the cache
 * @param cache the cache
 * @param data data for the cache
 * @param beg the begin page of the cache
 * @param end the end page of the cache
 */
template <typename Cache>
void fill_cache(const Cache& cache, const char *data, const pos_type beg, const pos_type end)
{
    BOOST_TEST_MESSAGE("fill the cache [" << beg << "; " << end << ")");
    for (pos_type i = beg; i < end; ++i)
    {
        BOOST_TEST_MESSAGE("fill the page " << i);
        memcpy(cache.get_page(i), &data[i * Cache::CACHE_PAGE_SIZE], Cache::CACHE_PAGE_SIZE);
    }
}

/**
 * Modify the cache
 * @param cache the cache
 * @param data data for the cache
 * @param beg the begin page of the cache
 * @param end the end page of the cache
 */
template <typename Cache>
void modify_cache(Cache& cache, const char *data, const pos_type beg, const pos_type end)
{
    BOOST_TEST_MESSAGE("mofift the cache [" << beg << "; " << end << ")");
    for (pos_type i = beg; i < end; ++i)
    {
        BOOST_TEST_MESSAGE("modify the page " << i);
        memcpy(cache.get_page(i), &data[i * Cache::CACHE_PAGE_SIZE], Cache::CACHE_PAGE_SIZE);
    }
}

/**
 * Check if the pages exist in the cache
 * @param cache the cache
 * @param beg the begin page of the cache
 * @param end the end page of the cache
 */
template <typename Cache>
void check_pagecache_attached(Cache& cache, const pos_type beg, const pos_type end)
{
    BOOST_TEST_MESSAGE("check the pages is attached [" << beg << "; " << end << ")");
    for (pos_type i = beg; i < end; ++i)
    {
        BOOST_REQUIRE(cache.page_exists(i).state() != PG_DETACHED);
    }
}

/**
 * Check if the pages don't exist in the cache
 * @param cache the cache
 * @param beg the begin page of the cache
 * @param end the end page of the cache
 */
template <typename Cache>
void check_pagecache_detached(Cache& cache, const pos_type beg, const pos_type end)
{
    BOOST_TEST_MESSAGE("check the pages is detached [" << beg << "; " << end << ")");
    for (pos_type i = beg; i < end; ++i)
    {
        BOOST_REQUIRE(cache.page_exists(i).state() == PG_DETACHED);
    }
}

/**
 * Check the cache
 * @param cache the cache
 * @param data data for the checking
 * @param beg the begin page of the cache
 * @param end the end page of the cache
 * @param count the count of the pages in the cache
 */
template <typename Cache>
void check_cache(const Cache& cache, const char *data, const pos_type beg, const pos_type end,
    const count_type count = Cache::CACHE_PAGE_COUNT)
{
    BOOST_TEST_MESSAGE("check data [" << beg << "; " << end << ")");
    BOOST_CHECK_EQUAL(cache.test_pool_page_count(), count);
    for (pos_type i = 0; i < 10 * Cache::CACHE_PAGE_COUNT; ++i)
    {
        pos_type n = rand() % (end - beg) + beg;
        BOOST_TEST_MESSAGE("check data of the page " << n);
        BOOST_REQUIRE(memcmp(cache.get_page(n), &data[n * Cache::CACHE_PAGE_SIZE], Cache::CACHE_PAGE_SIZE) == 0);
        BOOST_CHECK_EQUAL(cache.test_pool_page_count(), count);
    }
}

/**
 * Check the work of a cache
 * @param cache the cache
 */
template <typename Cache, int DATA_COUNT>
void test_cache(Cache& cache)
{
    char data[Cache::CACHE_PAGE_SIZE * DATA_COUNT];
    for (pos_type i = 0; i < sizeof(data); i++)
    {
        data[i] = rand() % 100;
    }

    /* fill the pages [0; 20), the cache must have only pages [10; 20) */
    BOOST_TEST_MESSAGE("### test_cache 1");
    fill_cache(cache, data, 0, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 2 * Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    check_cache(cache, data, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);

    /* fill the pages [0; 100), the cache must have only pages [90; 100) */
    BOOST_TEST_MESSAGE("### test_cache 2");
    fill_cache(cache, data, 0, DATA_COUNT);
    check_pagecache_detached(cache, 0, DATA_COUNT - Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, DATA_COUNT - Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    check_cache(cache, data, DATA_COUNT - Cache::CACHE_PAGE_COUNT, DATA_COUNT);
}

/**
 * Check the work of a dirty cache
 * @param cache the cache
 * @param saver the saver of the slave cache
 */
template <typename Cache, int DATA_COUNT>
void test_cache(Cache& cache, typename Cache::saver_type& saver)
{
    char data[Cache::CACHE_PAGE_SIZE * DATA_COUNT];
    for (pos_type i = 0; i < sizeof(data); i++)
    {
        data[i] = i % Cache::CACHE_PAGE_SIZE != 0 ?
            rand() % DATA_COUNT : i / Cache::CACHE_PAGE_SIZE;
    }

    /* 1. modify the pages of the cache [0; 20), the cache must have only pages [10; 20)
     * 2. clean the cache, the all modified pages [0; 20) must be saved */
    BOOST_TEST_MESSAGE("### test_dirty_cache 1");
    modify_cache(cache, data, 0, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 2 * Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    check_cache(cache, data, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    saver.CheckPages(0, Cache::CACHE_PAGE_COUNT, data);
    cache.clean();
    saver.CheckPages(Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT, data);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 2 * Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    cache.free();
    BOOST_REQUIRE(saver.empty());
    check_pagecache_detached(cache, 0, DATA_COUNT);

    /* 1. modify the pages of the cache [0; 20), the cache must have only pages [10; 20)
     * 2. free the cache, the all modified pages [0; 20) must be saved */
    BOOST_TEST_MESSAGE("### test_dirty_cache 2");
    modify_cache(cache, data, 0, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 2 * Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    check_cache(cache, data, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    saver.CheckPages(0, Cache::CACHE_PAGE_COUNT, data);
    cache.free();
    saver.CheckPages(Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT, data);
    check_pagecache_detached(cache, 0, DATA_COUNT);

    /* 1. fill the pages of the cache [0; 5), the cache must have all pages [0; 5)
     * 2. modify the pages of the cache [5; 10), the cache must have all pages [0; 10)
     * 3. fill the pages of the cache [10; 15), the cache must have only pages [5; 15)
     * 4. fill the pages of the cache [15; 20), the cache must have only pages [10; 20)
     * 5. the modified pages of the cache [5, 10) must be saved
     * 6. modify the pages of the cache [20; 25), the cache must have only pages [15; 25)
     * 7. clean the cache, the modified pages [20; 25) must be saved
     * 8. the cache must have the pages [15; 20)
     * 9. free the cache */
    BOOST_TEST_MESSAGE("### test_dirty_cache 3");
    fill_cache(cache, data, 0, Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_attached(cache, 0, Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_detached(cache, Cache::CACHE_PAGE_COUNT / 2, DATA_COUNT);
    check_cache(cache, data, 0, Cache::CACHE_PAGE_COUNT / 2, Cache::CACHE_PAGE_COUNT / 2);
    BOOST_REQUIRE(saver.empty());
    modify_cache(cache, data, Cache::CACHE_PAGE_COUNT / 2, Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, 0, Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    BOOST_REQUIRE(saver.empty());
    fill_cache(cache, data, Cache::CACHE_PAGE_COUNT, 3 * Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT / 2, 3 * Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_detached(cache, 3 * Cache::CACHE_PAGE_COUNT / 2, DATA_COUNT);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT / 2);
    BOOST_REQUIRE(saver.empty());
    fill_cache(cache, data, 3 * Cache::CACHE_PAGE_COUNT / 2, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_attached(cache, Cache::CACHE_PAGE_COUNT, 2 * Cache::CACHE_PAGE_COUNT);
    check_pagecache_detached(cache, 2 * Cache::CACHE_PAGE_COUNT, DATA_COUNT);
    check_pagecache_detached(cache, 0, Cache::CACHE_PAGE_COUNT);
    saver.CheckPages(Cache::CACHE_PAGE_COUNT / 2, Cache::CACHE_PAGE_COUNT, data);
    modify_cache(cache, data, 2 * Cache::CACHE_PAGE_COUNT, 5 * Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_attached(cache, 3 * Cache::CACHE_PAGE_COUNT / 2, 5 * Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_detached(cache, 5 * Cache::CACHE_PAGE_COUNT / 2, DATA_COUNT);
    check_pagecache_detached(cache, 0, 3 * Cache::CACHE_PAGE_COUNT / 2);
    BOOST_REQUIRE(saver.empty());
    cache.clean();
    saver.CheckPages(2 * Cache::CACHE_PAGE_COUNT, 5 * Cache::CACHE_PAGE_COUNT / 2, data);
    check_pagecache_attached(cache, 3 * Cache::CACHE_PAGE_COUNT / 2, 5 * Cache::CACHE_PAGE_COUNT / 2);
    check_pagecache_detached(cache, 5 * Cache::CACHE_PAGE_COUNT / 2, DATA_COUNT);
    check_pagecache_detached(cache, 0, 3 * Cache::CACHE_PAGE_COUNT / 2);
    cache.free();
    BOOST_REQUIRE(saver.empty());
    check_pagecache_detached(cache, 0, DATA_COUNT);
}

//==============================================================================
//  Check the cache filling with data
//==============================================================================
BOOST_AUTO_TEST_CASE(cache_test)
{
    const count_type page_count = 10;
    const size_type page_size  = 1024;
    const count_type data_count = 100;

    typedef cache<CacheSaver<page_size, data_count>, page_size, page_count> cache_type;
    cache_type cache;
    test_cache<cache_type, data_count>(cache);
}

//==============================================================================
//  Check the cache filling with dirty data
//==============================================================================
BOOST_AUTO_TEST_CASE(dirty_cache_test)
{
    const count_type page_count = 10;
    const size_type page_size  = 1024;
    const count_type data_count = 100;

    typedef cache<CacheSaver<page_size, data_count>, page_size, page_count> cache_type;

    CacheSaver<page_size, data_count> saver;
    cache_type cache(saver);
    test_cache<cache_type, data_count>(cache, saver);
}

