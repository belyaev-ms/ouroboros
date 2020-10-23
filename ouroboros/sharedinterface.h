/**
 * @file   sharedinterface.h
 * The shared interfaces
 */

#ifndef OUROBOROS_SHAREDINTERFACE_H
#define	OUROBOROS_SHAREDINTERFACE_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use sharedinterface (OUROBOROS_BOOST_ENABLED)
#endif

#include "ouroboros/memory.h"
#include "ouroboros/sharedobject.h"
#include "ouroboros/sharedcontainer.h"
#include "ouroboros/sharedlocker.h"
#include "ouroboros/interface.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>

namespace ouroboros
{

/**
 * The base interface for working with a table stored in shared memory
 */
template <int pageCount = OUROBOROS_PAGE_COUNT>
struct base_table_shared_interface
{
    template <typename T> struct object_type : public shared_object<T> {};
    typedef file_page<OUROBOROS_PAGE_SIZE, sizeof(journal_status_type)> file_page_type;
    typedef journal_file<file_page_type, pageCount> file_type;
    struct locker_type : public locker<mutex_locker>
    {
        typedef typename locker<mutex_locker>::lock_type lock_type;
        locker_type(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_locker>(name, scoped_count, sharable_count)
        {}
        locker_type(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_locker>(lock, scoped_count, sharable_count)
        {}
    };
    typedef gateway<boost::interprocess::interprocess_mutex> gateway_type;
};

/**
 * The base interface for working with a dataset in shared memory
 * (with separation between processes)
 */
struct base_dataset_shared_interface
{
    template <typename Key, typename Field>
    struct skey_list : public shared_map<Key, Field> {};
};

/**
 * The base interface for working with a dataset in shared memory
 * (with separation between processes)
 */
template <template <template <typename, typename, typename> class, typename,
        template <typename> class, typename> class Table, int pageCount = OUROBOROS_PAGE_COUNT>
struct base_shared_interface : public base_interface<base_table_shared_interface<pageCount>, Table>,
    public base_dataset_shared_interface {};

/**
 * The interface for working with a dataset in shared memory
 * (with separation between processes)
 */
typedef base_shared_interface<indexed_table> shared_interface;

}   //namespace ouroboros

#endif	/* OUROBOROS_SHAREDINTERFACE_H */

