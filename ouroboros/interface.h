/**
 * @file   interface.h
 * The local interface
 */

#ifndef OUROBOROS_INTERFACE_H
#define	OUROBOROS_INTERFACE_H

#include "ouroboros/global.h"
#include "ouroboros/object.h"
#include "ouroboros/container.h"
#include "ouroboros/memoryfile.h"
#include "ouroboros/journalfile.h"
#include "ouroboros/indexedtable.h"
#include "ouroboros/locker.h"

namespace ouroboros
{

/**
 * The base interface for working with a table stored in local memory
 * (without separation between processes)
 */
struct base_table_memory_interface
{
    template <typename T> struct object_type : public local_object<T> {};
    typedef memory_file file_type;
    typedef stub_locker locker_type;
};

/**
 * The base interface for working with a table in local memory
 * (without separation between processes)
 */
template <int pageCount = OUROBOROS_PAGE_COUNT>
struct base_table_local_interface : public base_table_memory_interface
{
    typedef journal_file<OUROBOROS_PAGE_SIZE, pageCount> file_type;
};

/**
 * The base interface for working with a dataset in local memory
 * (without separation between processes)
 */
struct base_dataset_local_interface
{
    template <typename Key, typename Field>
    struct skey_list : public local_map<Key, Field> {};
};

/**
 * The macros for table definition in a interface
 * N - name
 * T - type
 * I - interface
 */
#define OUROBOROS_TABLE_TYPE(N, T, I) template < typename Record, typename Key > \
    class N : public T< interface_table, Record, Key, I > \
    { \
        typedef T< interface_table, Record, Key, I > base_class; \
    public: \
        typedef typename base_class::source_type source_type; \
        typedef typename base_class::guard_type guard_type; \
        N(source_type& source, Key& key) : \
            base_class(source, key) \
        {} \
        N(source_type& source, Key& key, const guard_type& guard) : \
            base_class(source, key, guard) \
        {} \
    }

/**
 * The macros for indexed table definition in a interface
 * E - engine
 * N - name
 * T - type
 * I - interface
 */
#define __OUROBOROS_INDEX_TABLE_TYPE(E, N, T, I) template < typename Record, template <typename> class Index, typename Key > \
    class N : public T< E, Record, Index, Key, I > \
    { \
        typedef T< E, Record, Index, Key, I > base_class; \
    public: \
        typedef typename base_class::source_type source_type; \
        typedef typename base_class::guard_type guard_type; \
        N(source_type& source, Key& key) : \
            base_class(source, key) \
        {} \
        N(source_type& source, Key& key, const guard_type& guard) : \
            base_class(source, key, guard) \
        {} \
    }

/**
 * The macros for indexed table definition in a interface
 * N - name
 * T - type
 * I - interface
 */
#define OUROBOROS_INDEX_TABLE_TYPE(N, T, I) __OUROBOROS_INDEX_TABLE_TYPE(interface_table, N, T, I)

/**
 * The base interface for working with a dataset
 */
template <typename TableInterface, template <template <typename, typename, typename> class,
        typename, template <typename> class, typename, typename> class Table>
struct base_interface : public TableInterface
{
    OUROBOROS_INDEX_TABLE_TYPE(table_type, Table, TableInterface);
    OUROBOROS_TABLE_TYPE(key_table_type, data_table, TableInterface);
    OUROBOROS_TABLE_TYPE(info_table_type, data_table, TableInterface);
};

/**
 * The base interface for working with a dataset in local memory
 * (without separation between processes)
 */
template <template <template <typename, typename, typename> class, typename, template <typename> class,
        typename, typename> class Table, int pageCount = OUROBOROS_PAGE_COUNT>
struct base_local_interface : public base_interface<base_table_local_interface<pageCount>, Table>,
    public base_dataset_local_interface {};

/**
 * The base interface for working with a dataset stored in local memory
 * (without separation between processes)
 */
template <template <template <typename, typename, typename> class, typename, template <typename> class, typename, typename> class Table>
struct base_memory_interface : public base_interface<base_table_memory_interface, Table>,
    public base_dataset_local_interface {};

typedef base_local_interface<indexed_table> local_interface;

}   //namespace ouroboros


#endif	/* OUROBOROS_INTERFACE_H */

