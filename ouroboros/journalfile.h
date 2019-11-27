/**
 * @file   journalfile.h
 * The file with support of logging changes
 */

#ifndef OUROBOROS_JOURNALFILE_H
#define	OUROBOROS_JOURNALFILE_H

#include <stdint.h>
#include "ouroboros/backupfile.h"
#include "ouroboros/page.h"

namespace ouroboros
{

/** the status of a journal file page */
enum journal_status_type
{
    JS_CLEAN    = 0,
    JS_DIRTY    = 1,
    JS_FIXED    = 2
};

/**
 * The file with support of caching, with full support of transactions
 * mechanism
 * @attention caching of write operation execute in local memory, when allocated
 * cache memory will be exhausted, the oldest cache page will be stored in
 * the file, but before it, the original data will be copied to the backup file
 * and the index of the cache page will be stored in journal file.
 */
template <typename FilePage, int pageCount = 1024, typename File = file_lock<FilePage>,
        template <typename, int, int> class Cache = cache>
class journal_file : public backup_file<FilePage, pageCount, File, Cache>
{
    typedef File simple_file;
    typedef backup_file<FilePage, pageCount, File, Cache> base_class;
public:
    typedef FilePage file_page_type;
    typedef typename base_class::file_region_type file_region_type;
    typedef typename base_class::page_status_type page_status_type;
    typedef typename status_file_page<file_page_type>::status_type status_type;
    explicit journal_file(const std::string& name);
    journal_file(const std::string& name, const file_region_type& region);
    const bool init(); ///< ititialize
protected:
    const bool init_indexes(); ///< initialize the indexes of backup pages
    virtual void do_before_add_index(const pos_type index, void *page); ///< perform an action before add the index
    virtual void do_after_add_index(const pos_type index, void *page); ///< perform an action after add the index
    virtual void do_after_remove_index(const pos_type index); ///< perform an action after remove the index
    virtual void do_before_clear_indexes(); ///< perform an action before remove all indexes
    virtual void do_after_clear_indexes(); ///< perform an action after remove all indexes
};

//==============================================================================
//  journal_file
//==============================================================================
/**
 * Constructor
 * @param name the name of the file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
journal_file<FilePage, pageCount, File, Cache>::journal_file(const std::string& name) :
    base_class(name)
{
}

/**
 * Constructor
 * @param name the name of a file
 * @param region the region of a file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
journal_file<FilePage, pageCount, File, Cache>::journal_file(const std::string& name,
        const file_region_type& region) :
    base_class(name, region)
{
}

/**
 * Initialize
 * @return result of the initialization
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
const bool journal_file<FilePage, pageCount, File, Cache>::init()
{
    base_class::init();
    return init_indexes();
}

/**
 * Initialize the indexes of backup pages
 * @return the result of the initialization
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
const bool journal_file<FilePage, pageCount, File, Cache>::init_indexes()
{
    // check the last transaction was successful
    bool success = true;
    bool fixed = false;
    char buffer[base_class::CACHE_PAGE_SIZE];
    status_file_page<file_page_type> status_page(buffer);
    const count_type count = base_class::size() / base_class::CACHE_PAGE_SIZE;
    for (pos_type index = 0; index < count; ++index)
    {
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        const status_type status = status_page.get_status();
        if (0 == index && (status & JS_FIXED))
        {
            fixed = true;
            OUROBOROS_INFO("commit the file " << base_class::name());
            continue;
        }
        if (status != JS_CLEAN)
        {
            if (!fixed)
            {
                success = false;
                base_class::do_add_index(index);
            }
            else
            {
                OUROBOROS_INFO("commit the page  " << index);
                status_page.set_status(JS_CLEAN);
                simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
            }
        }
    }
    // the last transactio wasn't finished
    if (fixed)
    {
        OUROBOROS_INFO("commit the main page");
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, 0);
        status_page.set_status(JS_CLEAN);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, 0);
        OUROBOROS_INFO("transaction completed");
        return true;
    }
    // the last transaction wasn't successful
    if (!success)
    {
        base_class::recovery();
        return false;
    }
    return true;
}

/**
 * Perform an action before add the index
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
//virtual
template <typename FilePage, int pageCount,
    typename File, template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_before_add_index(const pos_type index,
    void *page)
{
    // if the page is not fixed then throw exception
    status_file_page<file_page_type> status_page(page);
    OUROBOROS_ASSERT(status_page.get_status() == JS_CLEAN);
#ifdef OUROBOROS_FLUSH_ENABLED
    base_class::flush_backup();
#endif
}

/**
 * Perform an action after add the index
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_add_index(const pos_type index,
    void *page)
{
    // mark the page as not fixed
    status_file_page<file_page_type> status_page(page);
    status_page.set_status(JS_DIRTY);
}

/**
 * Perform an action after remove the index
 * @param index the index of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_remove_index(const pos_type index)
{
    const page_status_type status = base_class::m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        void *page = base_class::m_cache.get_page(status);
        status_file_page<file_page_type> status_page(page);
        status_page.set_status(0 == index ? JS_FIXED : JS_CLEAN);
        simple_file::do_write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    else
    {
        char buffer[base_class::CACHE_PAGE_SIZE];
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        status_file_page<file_page_type> status_page(buffer);
        status_page.set_status(0 == index ? JS_FIXED : JS_CLEAN);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
}

/**
 * Perform an action before remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_before_clear_indexes()
{
    const page_status_type status = base_class::m_cache.page_exists(0);
    if (status.state() != PG_DETACHED)
    {
        void *page = base_class::m_cache.get_page(status);
        status_file_page<file_page_type> status_page(page);
        const status_type status = status_page.get_status();
        OUROBOROS_ASSERT(!(status & JS_FIXED));
        status_page.set_status(status | JS_FIXED);
        simple_file::do_write(page, base_class::CACHE_PAGE_SIZE, 0);
    }
    else
    {
        char buffer[base_class::CACHE_PAGE_SIZE];
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, 0);
        status_file_page<file_page_type> status_page(buffer);
        const status_type status = status_page.get_status();
        OUROBOROS_ASSERT(!(status & JS_FIXED));
        status_page.set_status(status | JS_FIXED);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, 0);
    }
}

/**
 * Perform an action after remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_clear_indexes()
{
    const page_status_type status = base_class::m_cache.page_exists(0);
    if (status.state() != PG_DETACHED)
    {
        void *page = base_class::m_cache.get_page(status);
        status_file_page<file_page_type> status_page(page);
        OUROBOROS_ASSERT(status_page.get_status() == JS_FIXED);
        status_page.set_status(JS_CLEAN);
        simple_file::do_write(page, base_class::CACHE_PAGE_SIZE, 0);
    }
    else
    {
        char buffer[base_class::CACHE_PAGE_SIZE];
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, 0);
        status_file_page<file_page_type> status_page(buffer);
        OUROBOROS_ASSERT(status_page.get_status() == JS_FIXED);
        status_page.set_status(JS_CLEAN);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, 0);
    }
}

}    //namespace ouroboros

#endif	/* OUROBOROS_JOURNALFILE_H */

