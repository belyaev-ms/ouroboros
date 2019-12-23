/**
 * @file    backupfile.h
 * The file with support of transaction mechanism
 */

#ifndef OUROBOROS_BUCKUPFILE_H
#define	OUROBOROS_BUCKUPFILE_H

#include <set>
#include "ouroboros/cachefile.h"

namespace ouroboros
{

/**
 * The file with support of caching, with full support of transactions
 * mechanism
 * @attention caching of write operation execute in local memory, when allocated
 * cache memory will be exhausted, the oldest cache page will be stored in
 * the file, but before it, the original data will be copied to the backup file.
 * When a transaction is canceled due to an error, the original file will
 * be recovered from the backup file.
 */
template <typename FilePage, int pageCount = 1024, typename File = file_lock<FilePage>,
        template <typename, int, int> class Cache = cache>
class backup_file : public cache_file<FilePage, pageCount, File, Cache>
{
    typedef typename File::simple_file simple_file;
    typedef cache_file<FilePage, pageCount, File, Cache> base_class;
public:
    typedef typename base_class::file_region_type file_region_type;
    typedef typename base_class::page_status_type page_status_type;
    explicit backup_file(const std::string& name);
    backup_file(const std::string& name, const file_region_type& region);

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction

    static void remove(const std::string& name); ///< remove a file by the name
protected:
    virtual size_type do_resize(const size_type size); ///< change the size of the file
    virtual void *get_page(const pos_type index); ///< get the buffer of the cache page
    virtual void *get_page(const pos_type index) const; ///< get the buffer of the cache page
    void add_index(const pos_type index, void *page); ///< add the index of the page to the backup set
    void do_add_index(const pos_type index); ///< add the index of the page to the backup set
    void remove_index(const pos_type index); ///< remove the index of the page to the backup set
    void recovery(); ///< restore the file from the backup file
    void clear_indexes(); ///< remove all indexes of pages from the backup set
#ifdef OUROBOROS_FLUSH_ENABLED
    void flush_backup() const; ///< forced synchronization data of the backup file
#endif
private:
    virtual void do_before_add_index(const pos_type index, void *page); ///< perform an action before add the index
    virtual void do_after_add_index(const pos_type index, void *page); ///< perform an action after add the index
    virtual void do_before_remove_index(const pos_type index); ///< perform an action before remove the index
    virtual void do_after_remove_index(const pos_type index); ///< perform an action after remove the index
    virtual void do_before_clear_indexes(); ///< perform an action before remove all indexes
    virtual void do_after_clear_indexes(); ///< perform an action after remove all indexes

    typedef std::set<pos_type> index_list; ///< containter of a backup set
protected:
    simple_file m_backup; ///< the backup file
    index_list m_indexes; ///< the backup set
};

//==============================================================================
//  backup_file
//==============================================================================
/**
 * Remove a file by the name
 * @param name the name of the file
 */
//static
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::remove(const std::string& name)
{
    base_class::remove(name + ".bak");
    base_class::remove(name);
}

/**
 * Constructor
 * @param name the name of the file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
backup_file<FilePage, pageCount, File, Cache>::backup_file(const std::string& name) :
    base_class(name),
    m_backup(name + ".bak")
{
}

/**
 * Constructor
 * @param name the name of a file
 * @param region the region of a file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
backup_file<FilePage, pageCount, File, Cache>::backup_file(const std::string& name,
        const file_region_type& region) :
    base_class(name, region),
    m_backup(name + ".bak")
{
}

/**
 * Change the size of the file
 * @param size the size of the file
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
size_type backup_file<FilePage, pageCount, File, Cache>::do_resize(const size_type size)
{
    return m_backup.resize(base_class::do_resize(size));
}

/**
 * Get the buffer of the cache page for write
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for write
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void *backup_file<FilePage, pageCount, File, Cache>::get_page(const pos_type index)
{
    const page_status_type status = base_class::m_cache.page_exists(index);
    void *page = base_class::m_cache.get_page(status);
    // check the page was loaded
    if (status.state() == PG_DETACHED)
    {
        // if the page has not been loaded then load data to the page
        simple_file::do_read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    // check if the page exists in the backup set
    if (TR_STARTED == base_class::state() && m_indexes.find(index) == m_indexes.end())
    {
        // store the page into the backup file
        m_backup.write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        add_index(index, page);
    }
    return page;
}

/**
 * Get the buffer of the cache page for read
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for read
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void *backup_file<FilePage, pageCount, File, Cache>::get_page(const pos_type index) const
{
    const page_status_type status = base_class::m_cache.page_exists(index);
    void *page = base_class::m_cache.get_page(status);
    // check the page was loaded
    if (status.state() == PG_DETACHED)
    {
        // if the page has not been loaded then load data to the page
        simple_file::do_read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    return page;
}

/**
 * Start the transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::start()
{
    base_class::start();
}

/**
 * Stop the transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::stop()
{
    base_class::stop();
    clear_indexes();
}

/**
 * Cancel the transaction
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::cancel()
{
    base_class::cancel();
    recovery();
}

/**
 * Restore the file from the backup file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
inline void backup_file<FilePage, pageCount, File, Cache>::recovery()
{
    OUROBOROS_INFO("restore the file " << base_class::name());
    char page[base_class::CACHE_PAGE_SIZE];
    index_list::const_iterator end = m_indexes.end();
    for (index_list::const_iterator it = m_indexes.begin(); it != end; ++it)
    {
        const pos_type index = *it;
        OUROBOROS_INFO("\trestore the page " << index);
        m_backup.read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        simple_file::do_write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        base_class::m_cache.free_page(index);
    }
    m_indexes.clear();
    OUROBOROS_INFO("recovery completed");
}

/**
 * Add the index of the page to the backup set
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_add_index(const pos_type index)
{
    m_indexes.insert(index);
}

/**
 * Add the index of the page to the backup set
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::add_index(const pos_type index,
    void *page)
{
    do_before_add_index(index, page);
    do_add_index(index);
    do_after_add_index(index, page);
}

/**
 * Remove the index of the page to the backup set
 * @param index the index of the page
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
inline void backup_file<FilePage, pageCount, File, Cache>::remove_index(const pos_type index)
{
    do_before_remove_index(index);
    m_indexes.erase(index);
    do_after_remove_index(index);
}

/**
 * Remove all indexes of pages from the backup set
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::clear_indexes()
{
    do_before_clear_indexes();
    index_list::const_iterator it = m_indexes.begin();
    while (it != m_indexes.end())
    {
        const pos_type index = *it;
        do_before_remove_index(index);
        m_indexes.erase(it);
        do_after_remove_index(index);
        it = m_indexes.begin();
    }
    do_after_clear_indexes();
}

/**
 * Perform an action before add the index
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_before_add_index(const pos_type index,
    void *page)
{

}

/**
 * Perform an action after add the index
 * @param index the index of the page
 * @param page the pointer to the buffer of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_after_add_index(const pos_type index,
    void *page)
{

}

/**
 * Perform an action before remove the index
 * @param index the index of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_before_remove_index(const pos_type index)
{

}

/**
 * Perform an action after remove the index
 * @param index the index of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_after_remove_index(const pos_type index)
{

}

/**
 * Perform an action before remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_before_clear_indexes()
{

}

/**
 * Perform an action after remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::do_after_clear_indexes()
{

}

#ifdef OUROBOROS_FLUSH_ENABLED
/**
 * Forced synchronization data of the backup file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void backup_file<FilePage, pageCount, File, Cache>::flush_backup() const
{
    m_backup.flush();
}
#endif

}   //namespace ouroboros


#endif	/* OUROBOROS_BUCKUPFILE_H */

