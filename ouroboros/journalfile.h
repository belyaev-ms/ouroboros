/**
 * @file   journalfile.h
 * The file with support of logging changes
 */

#ifndef OUROBOROS_JOURNALFILE_H
#define	OUROBOROS_JOURNALFILE_H

#include <unistd.h>
#include <stdint.h>
#include <map>
#include <vector>
#include "ouroboros/backupfile.h"
#include "ouroboros/page.h"

namespace ouroboros
{

/** the state of a journal file page */
enum journal_state_type
{
    JS_CLEAN    = 0,
    JS_DIRTY    = 1,
    JS_FIXED    = 2
};

/** the status of a journal file page */
struct journal_status_type
{
    journal_status_type() :
        transaction_id(0),
        state(JS_CLEAN)
    {}
    journal_status_type(const pos_type id, const journal_state_type st) :
        transaction_id(id),
        state(st)
    {}
    pos_type transaction_id;
    journal_state_type state;
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
    typedef status_file_page<file_page_type, journal_status_type> status_file_page_type;
    typedef typename status_file_page_type::status_type status_type;
    explicit journal_file(const std::string& name);
    journal_file(const std::string& name, const file_region_type& region);
    const bool init(); ///< initialize
protected:
    const bool init_indexes(); ///< initialize the indexes of backup pages
    virtual void do_before_add_index(const pos_type index, void *page); ///< perform an action before add the index
    virtual void do_after_add_index(const pos_type index, void *page); ///< perform an action after add the index
    virtual void do_after_remove_index(const pos_type index); ///< perform an action after remove the index
    virtual void do_after_clear_indexes(); ///< perform an action after remove all indexes
private:
    typedef std::vector<pos_type> page_list_type;
    void restore_transaction(const page_list_type& page_list); ///< restore a transaction
    void commit_transaction(const page_list_type& page_list); ///< commit a transaction
protected:
    static pos_type s_transaction_id;
    pos_type m_reference_index;
};

//==============================================================================
//  journal_file
//==============================================================================
//static
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
pos_type journal_file<FilePage, pageCount, File, Cache>::s_transaction_id = getpid();

/**
 * Constructor
 * @param name the name of the file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
journal_file<FilePage, pageCount, File, Cache>::journal_file(const std::string& name) :
    base_class(name),
    m_reference_index(NIL)
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
    base_class(name, region),
    m_reference_index(NIL)
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
 * Restore a transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::
    restore_transaction(const page_list_type& page_list)
{
    char buffer[base_class::CACHE_PAGE_SIZE];
    page_list_type::const_iterator end = page_list.end();
    for (page_list_type::const_iterator it = page_list.begin(); it != end; ++it)
    {
        const pos_type index = *it;
        OUROBOROS_INFO("\t\trestore the page " << index);
        base_class::m_backup.read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
}

/**
 * Commit a transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::
    commit_transaction(const page_list_type& page_list)
{
    char buffer[base_class::CACHE_PAGE_SIZE];
    status_file_page_type status_page(buffer);
    page_list_type::const_iterator end = page_list.end();
    for (page_list_type::const_iterator it = page_list.begin(); it != end; ++it)
    {
        const pos_type index = *it;
        OUROBOROS_INFO("\t\tcommit the page " << index);
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        status_page.set_status(journal_status_type());
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
}

/**
 * Initialize the indexes of backup pages
 * @return the result of the initialization
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
const bool journal_file<FilePage, pageCount, File, Cache>::init_indexes()
{
    typedef std::pair<pos_type, page_list_type> transaction_state_type;
    typedef std::map<pos_type, transaction_state_type> transaction_list_type;
    transaction_list_type transaction_list;
    // look for all transaction that were not finished
    char buffer[base_class::CACHE_PAGE_SIZE];
    status_file_page_type status_page(buffer);
    const count_type count = base_class::size() / base_class::CACHE_PAGE_SIZE;
    for (pos_type index = 0; index < count; ++index)
    {
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        journal_status_type index_status = status_page.get_status();
        transaction_list_type::iterator it = transaction_list.find(index_status.transaction_id);
        switch (index_status.state)
        {
            case JS_DIRTY:
                if (it != transaction_list.end())
                {
                    it->second.second.push_back(index);
                }
                else
                {
                    transaction_list.insert(std::make_pair(index_status.transaction_id,
                            std::make_pair(NIL, page_list_type(1, index))));
                }
                break;
            case JS_FIXED:
                if (it != transaction_list.end())
                {
                    OUROBOROS_ASSERT(it->second.first == NIL);
                    it->second.first = index;
                }
                else
                {
                    transaction_list.insert(std::make_pair(index_status.transaction_id,
                            std::make_pair(index, page_list_type())));
                }
                break;
            case JS_CLEAN:
            default:
                break;
        }
    }
    if (!transaction_list.empty())
    {
        // pocessing found transactions
        OUROBOROS_INFO("restore the file " << base_class::name());
        for (transaction_list_type::iterator transaction = transaction_list.begin();
                transaction != transaction_list.end(); ++transaction)
        {
            if (NIL == transaction->second.first)
            {
                OUROBOROS_INFO("\trestore the transaction " << transaction->first);
                restore_transaction(transaction->second.second);
            }
            else
            {
                transaction->second.second.push_back(transaction->second.first);
                OUROBOROS_INFO("\tcommit the transaction " << transaction->first);
                commit_transaction(transaction->second.second);
            }
        }
        OUROBOROS_INFO("recovery completed");
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
    status_file_page_type status_page(page);
    OUROBOROS_ASSERT(status_page.get_status().state == JS_CLEAN);
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
    status_file_page_type status_page(page);
    status_page.set_status(journal_status_type(s_transaction_id, JS_DIRTY));
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
    journal_status_type index_status;
    if (NIL == m_reference_index)
    {
        index_status = journal_status_type(s_transaction_id, JS_FIXED);
        m_reference_index = index;
    }
    const page_status_type status = base_class::m_cache.page_exists(index);
    if (status.state() != PG_DETACHED)
    {
        void *page = base_class::m_cache.get_page(status);
        status_file_page_type status_page(page);
        status_page.set_status(index_status);
        simple_file::do_write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    else
    {
        char buffer[base_class::CACHE_PAGE_SIZE];
        simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        status_file_page_type status_page(buffer);
        status_page.set_status(index_status);
        simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
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
    if (m_reference_index != NIL)
    {
        const page_status_type status = base_class::m_cache.page_exists(m_reference_index);
        if (status.state() != PG_DETACHED)
        {
            void *page = base_class::m_cache.get_page(status);
            status_file_page_type status_page(page);
            OUROBOROS_ASSERT(status_page.get_status().state == JS_FIXED);
            status_page.set_status(journal_status_type());
            simple_file::do_write(page, base_class::CACHE_PAGE_SIZE,
                m_reference_index * base_class::CACHE_PAGE_SIZE);
            base_class::clean();
        }
        else
        {
            char buffer[base_class::CACHE_PAGE_SIZE];
            simple_file::do_read(buffer, base_class::CACHE_PAGE_SIZE,
                m_reference_index * base_class::CACHE_PAGE_SIZE);
            status_file_page_type status_page(buffer);
            OUROBOROS_ASSERT(status_page.get_status().state == JS_FIXED);
            status_page.set_status(journal_status_type());
            simple_file::do_write(buffer, base_class::CACHE_PAGE_SIZE,
                m_reference_index * base_class::CACHE_PAGE_SIZE);
        }
        m_reference_index = NIL;
    }
}

}    //namespace ouroboros

#endif	/* OUROBOROS_JOURNALFILE_H */

