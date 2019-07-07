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
template <int pageSize = 1024, int pageCount = 1024, typename File = file_lock,
        template <typename, int, int> class Cache = cache>
class backup_file : public cache_file<pageSize, pageCount, File, Cache>
{
    typedef File simple_file;
    typedef cache_file<pageSize, pageCount, File, Cache> base_class;
public:
    typedef typename base_class::page_status_type page_status_type;
    explicit backup_file(const std::string& name);

    const size_type resize(const size_type size); ///< change the size of the file
    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction

    static void remove(const std::string& name); ///< remove a file by the name
protected:
    virtual void *get_page(const pos_type index); ///< get the buffer of the cache page
    virtual void *get_page(const pos_type index) const; ///< get the buffer of the cache page
    void add_index(const pos_type& pos) const; ///< add the index of the page to the backup set
    void remove_index(const pos_type& pos) const; ///< remove the index of the page to the backup set
    void recovery(); ///< restore the file from the backup file
    void clear_indexes(); ///< remove all indexes of pages from the backup set
#ifdef OUROBOROS_FLUSH_ENABLED
    void flush_backup() const; ///< forced synchronization data of the backup file
#endif
private:
    virtual void do_before_add_index(const pos_type& pos) const; ///< perform an action before add the index
    virtual void do_after_add_index(const pos_type& pos) const; ///< perform an action after add the index
    virtual void do_before_remove_index(const pos_type& pos) const; ///< perform an action before remove the index
    virtual void do_after_remove_index(const pos_type& pos) const; ///< perform an action after remove the index
    virtual void do_before_clear_indexes() const; ///< perform an action before remove all indexes
    virtual void do_after_clear_indexes() const; ///< perform an action after remove all indexes

    typedef std::set<pos_type> index_list; ///< containter of a backup set
private:
    mutable simple_file m_backup; ///< the backup file
    mutable index_list m_indexes; ///< the backup set
};

//==============================================================================
//  backup_file
//==============================================================================
/**
 * Remove a file by the name
 * @param name the name of the file
 */
//static
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::remove(const std::string& name)
{
    base_class::remove(name + ".bak");
    base_class::remove(name);
}

/**
 * Constructor
 * @param name the name of the file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
backup_file<pageSize, pageCount, File, Cache>::backup_file(const std::string& name) :
    base_class(name),
    m_backup(name + ".bak")
{
}

/**
 * Change the size of the file
 * @param size the size of the file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
const size_type backup_file<pageSize, pageCount, File, Cache>::resize(const size_type size)
{
    return m_backup.resize(base_class::resize(size));
}

/**
 * Get the buffer of the cache page for write
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for write
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void *backup_file<pageSize, pageCount, File, Cache>::get_page(const pos_type index)
{
    const page_status_type status = base_class::m_cache.page_exists(index);
    void *page = base_class::m_cache.get_page(status);
    // check the page was loaded
    if (status.state() == PG_DETACHED)
    {
        // if the page has not been loaded then load data to the page
        simple_file::read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    // check if the page exists in the backup set
    if (TR_STARTED == base_class::state() && m_indexes.find(index) == m_indexes.end())
    {
        m_backup.write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        add_index(index);
    }
    return page;
}

/**
 * Get the buffer of the cache page for read
 * @param index the index of the cache page
 * @return the pointer to the buffer of the cache page for read
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void *backup_file<pageSize, pageCount, File, Cache>::get_page(const pos_type index) const
{
    const page_status_type status = base_class::m_cache.page_exists(index);
    void *page = base_class::m_cache.get_page(status);
    // check the page was loaded
    if (status.state() == PG_DETACHED)
    {
        // if the page has not been loaded then load data to the page
        simple_file::read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
    }
    return page;
}

/**
 * Start the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::start()
{
    base_class::start();
    clear_indexes();
}

/**
 * Stop the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::stop()
{
    base_class::stop();
    clear_indexes();
}

/**
 * Cancel the transaction
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::cancel()
{
    base_class::cancel();
    recovery();
}

/**
 * Restore the file from the backup file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
inline void backup_file<pageSize, pageCount, File, Cache>::recovery()
{
    char page[base_class::CACHE_PAGE_SIZE];
    index_list::const_iterator end = m_indexes.end();
    for (index_list::const_iterator it = m_indexes.begin(); it != end; ++it)
    {
        const pos_type index = *it;
        OUROBOROS_INFO("restore the page  " << index);
        m_backup.read(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        simple_file::write(page, base_class::CACHE_PAGE_SIZE, index * base_class::CACHE_PAGE_SIZE);
        base_class::m_cache.free_page(index);
    }
    clear_indexes();
}

/**
 * Add the index of the page to the backup set
 * @param pos the index of the page
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::add_index(const pos_type& pos) const
{
    do_before_add_index(pos);
    m_indexes.insert(pos);
    do_after_add_index(pos);
}

/**
 * Remove the index of the page to the backup set
 * @param pos the index of the page
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
inline void backup_file<pageSize, pageCount, File, Cache>::remove_index(const pos_type& pos) const
{
    do_before_remove_index(pos);
    m_indexes.erase(pos);
    do_after_remove_index(pos);
}

/**
 * Remove all indexes of pages from the backup set
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::clear_indexes()
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
 * @param pos the index of the page
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_before_add_index(const pos_type& pos) const
{

}

/**
 * Perform an action after add the index
 * @param pos the index of the page
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_after_add_index(const pos_type& pos) const
{

}

/**
 * Perform an action before remove the index
 * @param pos the index of the page
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_before_remove_index(const pos_type& pos) const
{

}

/**
 * Perform an action after remove the index
 * @param pos the index of the page
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_after_remove_index(const pos_type& pos) const
{

}

/**
 * Perform an action before remove all indexes
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_before_clear_indexes() const
{

}

/**
 * Perform an action after remove all indexes
 */
//virtual
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::do_after_clear_indexes() const
{

}

#ifdef OUROBOROS_FLUSH_ENABLED
/**
 * Forced synchronization data of the backup file
 */
template <int pageSize, int pageCount, typename File, template <typename, int, int> class Cache>
void backup_file<pageSize, pageCount, File, Cache>::flush_backup() const
{
    m_backup.flush();
}
#endif

}   //namespace ouroboros


#endif	/* OUROBOROS_BUCKUPFILE_H */

