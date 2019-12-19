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
    struct locker_type : public locker<mutex_lock>
    {
        locker_type(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_lock>(name, scoped_count, sharable_count)
        {}
    };
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
template <template <template <typename, typename, typename> class, typename, template <typename> class, typename, typename> class Table, int pageCount = OUROBOROS_PAGE_COUNT>
struct base_shared_interface : public base_interface<base_table_shared_interface<pageCount>, Table>,
    public base_dataset_shared_interface {};

typedef base_shared_interface<indexed_table> shared_interface;

}   //namespace ouroboros

#endif	/* OUROBOROS_SHAREDINTERFACE_H */

