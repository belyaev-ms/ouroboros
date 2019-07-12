/**
 * @file   dataset.h
 * The dataset of the datasources
 */

#ifndef OUROBOROS_DATASET_H
#define	OUROBOROS_DATASET_H

#include <map>
#include "ouroboros/key.h"
#include "ouroboros/info.h"
#include "ouroboros/object.h"
#include "ouroboros/container.h"
#include "ouroboros/session.h"
#include "ouroboros/transaction.h"
#include "ouroboros/lockedtable.h"

namespace ouroboros
{

/**@todo think about who must reset the cache of key table and what
 * process will read after the cancel of current transaction */

/**@todo determine the behavior when the transaction is active and a dataset and
 * a table are destroyed */

/**
 * The dataset of the datasources
 * @attention example of the dataset file:
 *
 * /--page0--/--page1--/--page2--/--page3--/--page4--/--page5--/--page6--/<br>
 * +----+----+----+----+----------------+--+----+----+----------------+--+<br>
 * |info|####|key0|####|table0          |##|key1|####|table1          |##|<br>
 * +----+----+----+----+----------------+--+----+----+----------------+--+
 *
 * - info - the header of the dataset has:
 *      -# the version of the dataset;
 *      -# the count of the tables in the dataset;
 *      -# the count of the records in each table.
 * - key0 - the key of the table number 0:
 *      -# the value of the key (the identifier of the table);
 *      -# the position (the index) of the table in the datasource;
 *      -# the begin position of the records in the table;
 *      -# the end position of the records in the table;
 *      -# the count of the records in the table.
 * - table0 - the table number 0.
 *
 * The info, key and table regions are aligned in the dataset by the cache page,
 * because when a transaction is executing some cache page should not have
 * data of different table.
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
class data_set
{
public:
    typedef Key skey_type; ///< the structure of the key
    typedef typename skey_type::key_type key_type; ///< the type of key field
    typedef info info_type; ///< the struct of the information about the dataset
    typedef Interface interface_type; ///< the interface
    typedef typename interface_type::file_type file_type; ///< the file of the dataset
    typedef typename interface_type::template table_type<Record, Index, Key> table_type; ///< the table of data
    typedef typename interface_type::template key_table_type<skey_type, simple_key> key_table_type; ///< the table of keys
    typedef typename interface_type::template info_table_type<info_type, simple_key> info_table_type; ///< the table of inforamtion about the dataset
    typedef sharable_session<data_set> session_read; ///< the session for read data from a table
    typedef scoped_session<data_set> session_write; ///< the session for write data to a table
    typedef typename table_type::record_type record_type; ///< the record of data
    typedef typename table_type::record_list record_list; ///< the list of records
    typedef std::vector<key_type> key_list; ///< the list of keys
    typedef global_lock<interface_type> lock_type; ///< the lock for the dataset
    typedef global_lazy_lock<interface_type> lazy_lock_type; ///< the lazy lock for the dataset

    data_set(const std::string& name);
    data_set(const std::string& name, const count_type tbl_count, const count_type rec_count,
        const count_type ver = 0, const void *user_data = NULL, const size_type user_size = 0);
    ~data_set();
    void open(); ///< open the dataset
    inline const std::string& name() const; ///< get the name of the dataset

    const pos_type add_table(const key_type key); ///< add the table to the dataset
    const count_type remove_table(const key_type key); ///< remove the table from the dataset
    const bool table_exists(const key_type key); ///< check the table exists in the dataset

    inline session_read session_rd(const key_type key); ///< open the session to read data from the table
    inline session_write session_wr(const key_type key); ///< open the session to write data to the table
    inline void start();  ///< start the transaction
    inline void stop();   ///< stop the transaction
    inline void cancel(); ///< cancel the transaction
    inline void lazy_start();  ///< start the lazy transaction
    inline void lazy_stop();   ///< stop the lazy transaction
    inline void lazy_cancel(); ///< cancel the lazy transaction
    inline const transaction_state state() const; ///< get the state of the transaction
    inline const bool lazy_transaction_exists() const; ///< check the lazy transaction exists

    void get_key_list(key_list& list) const; ///< get the list of the keys

    const count_type rec_count(); ///< get the count of the records in each table of the dataset
    const count_type table_count(); ///< get the count of the table
    const revision_type version(); ///< get the version of the dataset
    const size_type get_user_data(void *buffer, const size_type size); ///< get the region of the users data
    const size_type set_user_data(const void *buffer, const size_type size); ///< set the region of the users data

    static void remove(const std::string& name); ///< remove the dataset
protected:
    friend class base_session<data_set>;
    friend class sharable_session<data_set>;
    friend class scoped_session<data_set>;
    friend class lazy_transaction<data_set>;
    typedef typename table_type::source_type source_type;
    typedef typename key_table_type::source_type key_source_type;
    typedef typename info_table_type::source_type info_source_type;
    typedef std::map<key_type, table_type*> table_list;
    typedef map<key_type, skey_type, interface_type::template skey_list> skey_list;
    typedef sharable_table_lock<key_table_type> lock_read;
    typedef scoped_table_lock<key_table_type> lock_write;
    typedef typename table_type::unsafe_table unsafe_table;
    typedef typename key_table_type::unsafe_table unsafe_table_key;
    typedef typename info_table_type::unsafe_table unsafe_table_info;
    typedef typename table_type::raw_record_type raw_record_type;
    typedef ouroboros::lazy_transaction<data_set> lazy_transaction_type;

    void init(const info_type& info); ///< initialize the dataset
    inline table_type* table(const key_type key); ///< get the table by the key
    const bool check_table(const key_type key); ///< check the table is not removed

    void recovery(); ///< recovery the dataset
    inline void update_info(); ///< update the information about the dataset
    inline const bool do_key_exists(const key_type key) const; ///< check the key exists
    inline skey_type& load_key(const pos_type pos); ///< load the key
    inline void update_key(table_type& table); ///< update the key of the table
    inline void lazy_transaction(lazy_transaction_type *transact); ///< set current lazy transaction
    inline void store_session(session_write& session); ///< put the session in the context of the lazy transaction

protected:
    const std::string m_name; ///< the name of the dataset
    bool m_opened; ///< the sign that the dataset is open
    file_type m_file; ///< the file of the dataset
    info_type m_info; ///< the information about the dataset
    info_source_type m_info_source; ///< the source of the information
    object<skey_type, interface_type::template object_type> m_skey_info; ///< the key of the information table
    info_table_type m_info_table; ///< the table of the infrormation
    source_type m_source; ///< the datasource
    table_list m_tables; ///< the list of the datatables
    key_source_type m_key_source; ///< the source of the table keys
    object<skey_type, interface_type::template object_type> m_skey_key; ///< the key of the keys table
    key_table_type m_key_table; ///< the table of the keys
    skey_list m_skeys; ///< the list of the table keys
    object<pos_type, interface_type::template object_type> m_hole_count; ///< the count of removed keys
    lazy_transaction_type *m_lazy_transaction; ///< the pointer to current lazy transaction
};

/**
 * Make the file name for the dataset
 * @param name the name of the dataset
 * @return the file name for the dataset
 */
inline const std::string make_dbname(const std::string& name)
{
    return name + ".dat";
}

/**
 * Get the size of object which is aligned by the cache page
 * @param size the size of object
 * @return the size of object which is aligned by the cache page
 */
template <typename File>
inline const size_type cache_alignment(const size_type size)
{
    const size_type result = calc_cache_size(size, File::CACHE_PAGE_SIZE);
    return result;
}

/**
 * Get the size of the separator for aligning the object by the cache page
 * @param size the size of object
 * @return the size of the separator
 */
template <typename File>
inline const size_type separator_size(const size_type size)
{
    return cache_alignment<File>(size) - size;
}

//==============================================================================
//  data_set
//==============================================================================
/**
 * Remove the dataset
 * @param name the name of the dataset
 */
//static
template <typename Key, typename Record, template <typename> class Index, typename Interface>
void data_set<Key, Record, Index, Interface>::remove(const std::string& name)
{
    source_type::remove(make_dbname(name).c_str());
}

/**
 * Constructor that is used when parameters of the dataset are not known
 * @attention after use, you must open (method open)
 * @param name the name of the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
data_set<Key, Record, Index, Interface>::data_set(const std::string& name) :
    m_name(name),
    m_opened(false),
    m_file(make_dbname(name).c_str()),
    m_info(0, 0),
    m_info_source(m_file, 1, 1),
    m_skey_info(make_object_name(m_info_source.name(), "info")),
    m_info_table(m_info_source, m_skey_info(), typename info_table_type::guard_type()),
    m_source(m_file, options_type(cache_alignment<file_type>(m_info_source.size()) +
        cache_alignment<file_type>(skey_type::static_size()), 0, NIL)),
    m_key_source(m_file, 1, options_type(cache_alignment<file_type>(m_info_source.size()), NIL, 0)),
    m_skey_key(make_object_name(m_key_source.name(), "key")),
    m_key_table(m_key_source, m_skey_key(), typename key_table_type::guard_type()),
    m_skeys(make_object_name(name, "keyList")),
    m_hole_count(make_object_name(name, "cntHole"), 0),
    m_lazy_transaction(NULL)
{
}

/**
 * Constructor that is used when parameters of the dataset are known
 * @param name the name of the dataset
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records
 * @param ver the version of the dataset
 * @param user_data the pointer to the user data
 * @param user_size the size of the user data
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
data_set<Key, Record, Index, Interface>::data_set(const std::string& name, const count_type tbl_count,
        const count_type rec_count, const count_type ver, const void *user_data, const size_type user_size) :
    m_name(name),
    m_opened(false),
    m_file(make_dbname(name).c_str()),
    m_info(tbl_count, rec_count, 0, ver, user_data, user_size),
    m_info_source(m_file, 1, 1),
    m_skey_info(make_object_name(m_info_source.name(), "info")),
    m_info_table(m_info_source, m_skey_info(), typename info_table_type::guard_type()),
    m_source(m_file, tbl_count, rec_count, options_type(cache_alignment<file_type>(m_info_source.size()) +
        cache_alignment<file_type>(skey_type::static_size()), table_type::REC_SPACE,
        separator_size<file_type>((raw_record_type::static_size() + table_type::REC_SPACE) * rec_count) +
        cache_alignment<file_type>(skey_type::static_size()))),
    m_key_source(m_file, 1, tbl_count, options_type(cache_alignment<file_type>(m_info_source.size()),
        cache_alignment<file_type>(m_source.table_size()) + separator_size<file_type>(skey_type::static_size()), 0)),
    m_skey_key(make_object_name(m_key_source.name(), "key")),
    m_key_table(m_key_source, m_skey_key(), typename key_table_type::guard_type()),
    m_skeys(make_object_name(name, "keyList")),
    m_hole_count(make_object_name(name, "cntHole"), 0),
    m_lazy_transaction(NULL)
{
    OUROBOROS_DEBUG("create the dataset " << PR(name) << PR(tbl_count) << PE(rec_count));
    // set the global lock until the end of the initialization
    lock_type glock(5 * OUROBOROS_LOCK_TIMEOUT);
    m_info_table.recovery();
    m_key_table.recovery();
    // read the information about the dataset
    info_type info;
    m_info_table.read(info, 0);
    if (m_info.version > 0 && info.version > 0 && info.version != m_info.version)
    {
        OUROBOROS_THROW_ERROR(version_error, PR(m_info.version) << PR(info.version) << "the version of the dataset is not supported");
    }
    // check the current information about the dataset
    if (info.tbl_count > 0)
    {
        // check the count of the records in the tables
        if (m_info.rec_count != info.rec_count)
        {
            OUROBOROS_THROW_ERROR(compatibility_error, PR(m_info.rec_count) << PR(info.rec_count) << "the count of the records is different");
        }
        m_info.tbl_count = std::max(m_info.tbl_count, info.tbl_count);
        m_info.key_count = info.key_count;
        m_source.m_tbl_count = m_info.tbl_count;
        m_key_source.m_rec_count = m_info.tbl_count;
    }
    // initialize the dataset
    init(m_info);
    // update the information about the dataset
    update_info();
}

/**
 * Destructor
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
data_set<Key, Record, Index, Interface>::~data_set()
{
    OUROBOROS_DEBUG("close db " << PE(m_name));
    if (m_opened)
    {
        // remove the objects of the tables
        const typename table_list::iterator itend = m_tables.end();
        for (typename table_list::iterator it = m_tables.begin(); it != itend; ++it)
        {
            table_type *table = it->second;
            delete table;
        }
    }
}

/**
 * Initialize the dataset
 * @param info the information about the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
void data_set<Key, Record, Index, Interface>::init(const info_type& info)
{
    OUROBOROS_DEBUG("init db " << PR(m_name) << PE(info));
    if (!m_opened)
    {
        m_opened = true;
        // initialize the file of the dataset
        const bool success = m_file.init();
        // check the need to generate a list of keys
        if (!m_skeys->empty())
        {
            // if the incomplete transaction is found
            if (!success)
            {
                OUROBOROS_INFO("recovery db " <<  PR(m_name) << PE(info));
                //... then roll back the transaction
                recovery();
            }
            return;
        }
    }
    // read data of the keys and generate the list of the key
    m_hole_count() = 0;
    typename table_type::guard_type guard(false);
    for (pos_type pos = 0; pos < info.key_count; ++pos)
    {
        // read the data of the key
        skey_type& skey = load_key(pos);
        // check the key is valid
        if (!skey.valid())
        {
            OUROBOROS_THROW_BUG(PR(skey) << "the key is damaged");
        }
        // check the key is removed
        if (skey.pos < 0)
        {
            ++m_hole_count();
            continue;
        }
#ifdef OUROBOROS_OPEN_TABLE_IMMEDIATELY
        // generate the list of the tables
        table_type *table = new table_type(m_source, skey, guard);
        m_tables.insert(typename table_list::value_type(skey.key, table));
        table->recovery();
#endif
    }
    // set the position to the key
    if (info.key_count > 0)
    {
        m_key_table.set_end_pos(info.key_count);
        m_key_table.set_count(info.key_count);
        m_key_table.update();
    }
}

/**
 * Open the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
void data_set<Key, Record, Index, Interface>::open()
{
    OUROBOROS_DEBUG("open db " << PE(m_name));
    // set the global lock until the end of the initialization
    lock_type glock(5 * OUROBOROS_LOCK_TIMEOUT);
    m_info_table.recovery();
    m_key_table.recovery();
    // read the information about the dataset
    info_type info;
    m_info_table.read(info, 0);
    if (0 == info.tbl_count || 0 == info.rec_count)
    {
        OUROBOROS_THROW_BUG("error opening the dataset " << PE(m_name));
    }
    // initialize the dataset
    m_info = info;
    m_source.m_options.rec_space = table_type::REC_SPACE;
    m_source.m_options.tbl_space = separator_size<file_type>((raw_record_type::static_size() +
            table_type::REC_SPACE) * info.rec_count) + cache_alignment<file_type>(skey_type::static_size());
    m_source.init(info.tbl_count, info.rec_count);
    m_key_source.m_options.rec_space = cache_alignment<file_type>(m_source.table_size()) +
            separator_size<file_type>(skey_type::static_size());
    m_key_source.init(1, info.tbl_count);
    init(info);
}

/**
 * Get the name of the dataset
 * @return the name of the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline const std::string& data_set<Key, Record, Index, Interface>::name() const
{
    return m_name;
}

/**
 * Add the table to the dataset
 * @param key the key of the tabel
 * @return the postion of the table
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const pos_type data_set<Key, Record, Index, Interface>::add_table(const key_type key)
{
    lock_type glock;
    lock_write lock(m_key_table);
    if (do_key_exists(key))
    {
        OUROBOROS_THROW_BUG(PR(key) << "another table has the key");
    }
    if (m_skeys->size() >= m_info.tbl_count && 0 == m_hole_count())
    {
        OUROBOROS_THROW_ERROR(range_error, PR(key) << PR(m_info.tbl_count) << "the count of the table is too large");
    }

    // check the removed keys are exists
    if (m_hole_count() > 0)
    {
        // look for the firts removed key
        const typename skey_list::iterator itend = m_skeys->end();
        for (typename skey_list::iterator it = m_skeys->begin(); it != itend; ++it)
        {
            const spos_type pos = it->second.pos;
            if (pos < 0)
            {
                // replace the key with the new key
                m_skeys->erase(it);
                skey_type& skey = m_skeys->insert(typename skey_list::value_type(key,
                        skey_type(key, -pos - 1, 0, 0, 0, 0))).first->second;
                table_type *table = new table_type(m_source, skey);
                m_tables.insert(typename table_list::value_type(key, table));
                table->clear();
                table->recovery();
                m_key_table.write(skey, skey.pos);
                --m_hole_count();
                return m_key_table.back_pos();
            }
        }
        OUROBOROS_THROW_BUG("the sign of removed key is exists, but the key is not found!");
    }

    // add the key and the table to the dataset
    const pos_type pos = m_key_table.end_pos();
    skey_type& skey = m_skeys->insert(typename skey_list::value_type(key,
            skey_type(key, pos, 0, 0, 0, 0))).first->second;
    table_type *table = new table_type(m_source, skey);
    m_tables.insert(typename table_list::value_type(key, table));
    table->clear();
    table->recovery();
    update_info();
    const pos_type result = m_key_table.add(skey);
    m_key_table.update();
    return result;
}

/**
 * Remove the table from the dataset
 * @param key the key of the table
 * @return the count of the tables
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const count_type data_set<Key, Record, Index, Interface>::remove_table(const key_type key)
{
    lock_type glock;
    lock_write lock(m_key_table);
    if (!do_key_exists(key))
    {
        OUROBOROS_THROW_BUG(PR(key) << "the key is not found");
    }

    // check the table is exists
    typename table_list::iterator it = m_tables.find(key);
    if (m_tables.end() != it)
    {
        // remove the table from the list of the tables
        table_type *table = it->second;
        update_key(*table);
        delete table;
        m_tables.erase(it);
    }

    /**@attention Simple remove the key from the keys table is not unacceptable,
     * because then the dataset will have the table on the position but will not
     * have the key. And just the position will be lost forever.
     * Don't remove the key but change its parameter pos = -pos - 1 (negative position)!
     */
    skey_type& skey = m_skeys()[key];
    const spos_type pos = skey.pos;
    skey.pos = -pos - 1;
    m_key_table.write(&skey, pos);
    m_key_table.update();
    ++m_hole_count();
    update_info();
    return m_key_table.end_pos();
}

/**
 * Check the table is not removed
 * @param key the key of the table
 * @return the table is not removed
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const bool data_set<Key, Record, Index, Interface>::check_table(const key_type key)
{
    bool result = true;
    if (!m_key_table.relevant())
    {
        const typename skey_list::const_iterator itend = m_skeys->end();
        for (typename skey_list::const_iterator it = m_skeys->begin(); it != itend; ++it)
        {
            if (it->second.pos < 0)
            {
                typename table_list::iterator table = m_tables.find(it->first);
                if (table != m_tables.end())
                {
                    delete table->second;
                    m_tables.erase(table);
                    if (key == it->first)
                    {
                        result = false;
                    }
                }
            }
        }
        m_key_table.recovery();
    }
    return result;
}

/**
 * Get the table
 * @param key the key of the table
 * @return the table
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline typename data_set<Key, Record, Index, Interface>::table_type*
    data_set<Key, Record, Index, Interface>::table(const key_type key)
{
    lock_read lock(m_key_table);
    // check the table is not removed
    if (!check_table(key))
    {
        OUROBOROS_ERROR(PR(key) << "the table is removed");
        return NULL;
    }
    // check the table is exists
    typename table_list::iterator it = m_tables.find(key);
    if (m_tables.end() != it)
    {
        return it->second;
    }
    else if (!do_key_exists(key))
    {
        OUROBOROS_ERROR(PR(key) << "the key is not found");
        return NULL;
    }
    else
    {
        /* If the table is not found but the key is exists then the table was
         * added by another process. Add the table to the dataset
         */
        skey_type& skey = m_skeys()[key];
        table_type *table = new table_type(m_source, skey, typename table_type::guard_type());
        m_tables.insert(typename table_list::value_type(key, table));
        table->recovery();
        return table;
    }
}

/**
 * Check the table is exists
 * @param key the key of the table
 * @return the result of the checking
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const bool data_set<Key, Record, Index, Interface>::table_exists(const key_type key)
{
    lock_read lock(m_key_table);
    // check the table is removed
    if (!check_table(key))
    {
        // the table is removed
        return false;
    }
    return m_tables.find(key) != m_tables.end() || do_key_exists(key);
}

/**
 * Check the key is exists
 * @param key the key
 * @return the result of the checking
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline const bool data_set<Key, Record, Index, Interface>::do_key_exists(const key_type key) const
{
    if (m_skeys->empty())
    {
        return false;
    }
    else
    {
        const typename skey_list::const_iterator it = m_skeys->find(key);
        return it != m_skeys->end() && it->second.pos >= 0;
    }
}

/**
 * Open the session to read data from the table
 * @param key the key of the table
 * @return the session
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline typename data_set<Key, Record, Index, Interface>::session_read
    data_set<Key, Record, Index, Interface>::session_rd(const key_type key)
{
    return session_read(*this, key);
}

/**
 * Open the session to write data to the table
 * @param key the key of the table
 * @return the session
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline typename data_set<Key, Record, Index, Interface>::session_write
    data_set<Key, Record, Index, Interface>::session_wr(const key_type key)
{
    return session_write(*this, key);
}

/**
 * Start the transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::start()
{
    m_key_table.lock_scoped();
    try
    {
        m_file.start();
    }
    catch (...)
    {
        m_key_table.unlock_scoped();
        throw;
    }
}

/**
 * Start the lazy transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::lazy_start()
{
    m_key_table.lock_sharable();
    try
    {
        m_file.start();
    }
    catch (...)
    {
        m_key_table.unlock_sharable();
        throw;
    }
}

/**
 * Stop the transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::stop()
{
    try
    {
        m_file.stop();
    }
    catch (...)
    {
        m_key_table.unlock_scoped();
        throw;
    }
    m_key_table.unlock_scoped();
}

/**
 * Stop the lazy transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::lazy_stop()
{
    try
    {
        m_file.stop();
    }
    catch (...)
    {
        m_key_table.unlock_sharable();
        throw;
    }
    m_key_table.unlock_sharable();
}

/**
 * Cancel the transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::cancel()
{
    try
    {
        m_file.cancel();
        recovery();
    }
    catch (...)
    {
        m_key_table.unlock_scoped();
        throw;
    }
    m_key_table.unlock_scoped();
}

/**
 * Cancel the lazy transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::lazy_cancel()
{
    try
    {
        m_file.cancel();
    }
    catch (...)
    {
        m_key_table.unlock_sharable();
        throw;
    }
    m_key_table.unlock_sharable();
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline const transaction_state data_set<Key, Record, Index, Interface>::state() const
{
    return m_file.state();
}

/**
 * Recovery the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::recovery()
{
    // recovery the table of the keys
    info_type info;
    m_info_table.read(info, 0);
    m_info.key_count = info.key_count;

    // reset current list of the tables
    {
        const typename table_list::iterator itend = m_tables.end();
        for (typename table_list::iterator it = m_tables.begin();
            it != itend; ++it)
        {
            delete it->second;
        }
        m_tables.clear();
    }

    // reload the keys
    m_hole_count() = 0;
    typedef std::map<key_type, skey_type> reloaded_key_list;
    reloaded_key_list reloaded_keys;
    for (pos_type pos = 0; pos < info.key_count; ++pos)
    {
        skey_type skey;
        m_key_table.read(skey, pos);
        const key_type key = skey.key;
        reloaded_keys.insert(typename reloaded_key_list::value_type(key, skey));
        if (skey.pos < 0)
        {
            ++m_hole_count();
        }
    }

    // recovery changed keys
    {
        typename skey_list::iterator it = m_skeys->begin();
        while (it != m_skeys->end())
        {
            typename reloaded_key_list::iterator skey = reloaded_keys.find(it->first);
            if (skey == reloaded_keys.end())
            {
                m_skeys->erase(it++);
            }
            else
            {
                it->second = skey->second;
                reloaded_keys.erase(skey);
                ++it;
            }
        }
    }

    // recovery removed keys
    {
        const typename reloaded_key_list::const_iterator itend = reloaded_keys.end();
        for (typename reloaded_key_list::const_iterator it = reloaded_keys.begin(); it != itend; ++it)
        {
            m_skeys->insert(*it);
        }
        reloaded_keys.clear();
    }

    m_key_table.set_end_pos(info.key_count);
    m_key_table.set_count(info.key_count);
    m_key_table.update();
}

/**
 * Load the key
 * @param pos the position of the ket
 * @return the data of the key
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline typename data_set<Key, Record, Index, Interface>::skey_type&
    data_set<Key, Record, Index, Interface>::load_key(const pos_type pos)
{
    skey_type skey;
    m_key_table.read(skey, pos);
    const key_type key = skey.key;
    OUROBOROS_DEBUG(PR(skey) << "initialize the table");
    // check the key is unique
    if (do_key_exists(key))
    {
        OUROBOROS_THROW_BUG(PR(key) << "another table has the key");
    }
    // add the key to the list of the keys
    return m_skeys->insert(typename skey_list::value_type(key, skey)).first->second;
}

/**
 * Update the key of the table
 * @param table the table witch has the key
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::update_key(table_type& table)
{
    const skey_type& skey = table.skey();
    if (skey.pos >= 0)
    {
        static_cast<unsafe_table_key&>(m_key_table).write(&skey, skey.pos);
    }
}

/**
 * Update the information about the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::update_info()
{
    m_info.key_count = m_skeys->size();
    static_cast<unsafe_table_info&>(m_info_table).write(&m_info, 0);
}

/**
 * Get the list of the keys
 * @param [out] list the list of the keys
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
void data_set<Key, Record, Index, Interface>::get_key_list(key_list& list) const
{
    const typename skey_list::t_type::const_iterator itend = m_skeys->end();
    for (typename skey_list::t_type::const_iterator it = m_skeys->begin();
        it != itend; ++it)
    {
        list.push_back(it->first);
    }
}

/**
 * Get the count of the records
 * @return the count of the records
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const count_type data_set<Key, Record, Index, Interface>::rec_count()
{
    count_type count = m_source.rec_count();
    if (0 == count)
    {
        info_type info;
        m_info_table.read(info, 0);
        count = info.rec_count;
    }
    return count;
}

/**
 * Get the count of the tables
 * @return the count of the tables
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const count_type data_set<Key, Record, Index, Interface>::table_count()
{
    count_type count = m_key_source.rec_count();
    if (0 == count)
    {
        info_type info;
        m_info_table.read(info, 0);
        count = info.tbl_count;
    }
    return count;
}

/**
 * Get the version of the dataset
 * @return the version of the dataset
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const revision_type data_set<Key, Record, Index, Interface>::version()
{
    revision_type ver = m_info.version;
    if (0 == ver)
    {
        info_type info;
        m_info_table.read(&info, 0);
        ver = info.version;
    }
    return ver;
}

/**
 * Get the region of the user data
 * @param buffer the buffer for user data
 * @param size the size of the buffer
 * @return the size of getting data
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const size_type data_set<Key, Record, Index, Interface>::get_user_data(void *buffer, const size_type size)
{
    info_type info;
    if (!info.compare_data(m_info))
    {
        return m_info.get_data(buffer, size);
    }
    else
    {
        m_info_table.read(&info, 0);
        return info.get_data(buffer, size);
    }
}

/**
 * Set the region of the user data
 * @param buffer the buffer for user data
 * @param size the size of the buffer
 * @return the size of the setting data
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
const size_type data_set<Key, Record, Index, Interface>::set_user_data(const void *buffer, const size_type size)
{
    const size_type count = m_info.set_data(buffer, size);
    if (count > 0)
    {
        m_info_table.write(&m_info, 0);
    }
    return count;
}

/**
 * Check the lazy transaction exists
 * @return the result of the checking
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline const bool data_set<Key, Record, Index, Interface>::lazy_transaction_exists() const
{
    return m_lazy_transaction != NULL;
}

/**
 * Set current lazy transaction
 * @param transact the current lazy transaction
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::lazy_transaction(lazy_transaction_type *transact)
{
    m_lazy_transaction = transact;
}

/**
 * Put the session in the context of the lazy transaction
 * @param session the session for write a table
 */
template <typename Key, typename Record, template <typename> class Index, typename Interface>
inline void data_set<Key, Record, Index, Interface>::store_session(session_write& session)
{
    if (m_lazy_transaction != NULL)
    {
        m_lazy_transaction->push(session);
    }
}

}   //namespace ouroboros


#endif	/* OUROBOROS_DATASET_H */

