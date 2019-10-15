/**
 * @file   journalfile.h
 * The file with support of logging changes
 */

#ifndef OUROBOROS_JOURNALFILE_H
#define	OUROBOROS_JOURNALFILE_H

#include <stdint.h>
#include "ouroboros/backupfile.h"

namespace ouroboros
{

/**
 * The file with support of caching, with full support of transactions
 * mechanism
 * @attention caching of write operation execute in local memory, when allocated
 * cache memory will be exhausted, the oldest cache page will be stored in
 * the file, but before it, the original data will be copied to the backup file
 * and the index of the cache page will be stored in journal file.
 */
template <typename FilePage, int pageCount = 1024, typename File = file_lock,
        template <typename, int, int> class Cache = cache>
class journal_file : public backup_file<FilePage, pageCount, File, Cache>
{
    typedef File simple_file;
    typedef backup_file<FilePage, pageCount, File, Cache> base_class;
public:
    explicit journal_file(const std::string& name);
    const bool init(); ///< ititialize
    const size_type resize(const size_type size); ///< change the size of the file
    static void remove(const std::string& name); ///< remove a file by the name
protected:
    const bool init_indexes(); ///< initialize the indexes of backup pages
    virtual void do_before_add_index(const pos_type& pos) const; ///< perform an action before add the index
    virtual void do_after_add_index(const pos_type& pos) const; ///< perform an action after add the index
    virtual void do_after_remove_index(const pos_type& pos) const; ///< perform an action after remove the index
    virtual void do_before_clear_indexes() const; ///< perform an action before remove all indexes
    virtual void do_after_clear_indexes() const; ///< perform an action after remove all indexes
private:
    mutable simple_file m_journal;
};

//==============================================================================
//  journal_file
//==============================================================================
/**
 * Remove a file by the name
 * @param name the name of the file
 */
//static
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::remove(const std::string& name)
{
    simple_file::remove(name + ".jrn");
    base_class::remove(name);
}

/**
 * Constructor
 * @param name the name of the file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
inline journal_file<FilePage, pageCount, File, Cache>::journal_file(const std::string& name) :
    base_class(name),
    m_journal(name + ".jrn")
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
 * Change the size of the file
 * @param size the size of the file
 */
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
const size_type journal_file<FilePage, pageCount, File, Cache>::resize(const size_type size)
{
    const size_type result = base_class::resize(size);
    m_journal.resize(result / base_class::CACHE_PAGE_SIZE);
    return result;
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
    char buffer[base_class::CACHE_PAGE_SIZE];
    const size_type size = m_journal.size();
    for (pos_type pos = 0; pos < size; pos += sizeof(buffer))
    {
        const size_type l = size - pos;
        const size_type count = l < sizeof(buffer) ? l : sizeof(buffer);
        m_journal.read(buffer, count, pos);
        for (pos_type i = 0; i < count; ++i)
        {
            if (buffer[i] != 0)
            {
                success = false;
                base_class::add_index(pos + i);
            }
        }
    }
    // the last transaction wasn't successful
    if (!success)
    {
        OUROBOROS_INFO("restore the file " << base_class::name());
        base_class::recovery();
    }
    return success;
}

/**
 * Perform an action before add the index
 * @param pos the index of the page
 */
//virtual
template <typename FilePage, int pageCount,
    typename File, template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_before_add_index(const pos_type& pos) const
{
#ifdef OUROBOROS_FLUSH_ENABLED
    base_class::flush_backup();
#endif
}

/**
 * Perform an action after add the index
 * @param pos the index of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_add_index(const pos_type& pos) const
{
    // store the index in the log file
    const uint8_t flag = 1;
    m_journal.write(&flag, sizeof(flag), pos);
#ifdef OUROBOROS_FLUSH_ENABLED
    m_journal.flush();
#endif
}

/**
 * Perform an action after remove the index
 * @param pos the index of the page
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_remove_index(const pos_type& pos) const
{
    // remove the index from the log file
    const uint8_t flag = 0;
    m_journal.write(&flag, sizeof(flag), pos);
}

/**
 * Perform an action before remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_before_clear_indexes() const
{
#ifdef OUROBOROS_FLUSH_ENABLED
    base_class::flush();
#endif
}

/**
 * Perform an action after remove all indexes
 */
//virtual
template <typename FilePage, int pageCount, typename File,
    template <typename, int, int> class Cache>
void journal_file<FilePage, pageCount, File, Cache>::do_after_clear_indexes() const
{
#ifdef OUROBOROS_FLUSH_ENABLED
    m_journal.flush();
#endif
}

}    //namespace ouroboros

#endif	/* OUROBOROS_JOURNALFILE_H */

