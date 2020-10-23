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
#include "ouroboros/page.h"
#include "ouroboros/gateway.h"

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
    typedef file_page<1> file_page_type;
    typedef stub_locker locker_type;
    typedef gateway<locker_type> gateway_type;
};

/**
 * The base interface for working with a table in local memory
 * (without separation between processes)
 */
template <int pageCount = OUROBOROS_PAGE_COUNT>
struct base_table_local_interface : public base_table_memory_interface
{
    typedef file_page<OUROBOROS_PAGE_SIZE, sizeof(journal_status_type)> file_page_type;
    typedef journal_file<file_page_type, pageCount> file_type;
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
 */
#define OUROBOROS_TABLE_TYPE(N, T) template < typename Record, typename Controlblock > \
    class N : public T< interface_table, Record, Controlblock > \
    { \
        typedef T< interface_table, Record, Controlblock > base_class; \
    public: \
        typedef typename base_class::source_type source_type; \
        typedef typename base_class::controlblock_type controlblock_type; \
        typedef typename base_class::skey_type skey_type; \
        typedef typename base_class::guard_type guard_type; \
        N(source_type& source, skey_type& key) : \
            base_class(source, key) \
        {} \
        N(source_type& source, skey_type& key, const guard_type& guard) : \
            base_class(source, key, guard) \
        {} \
        N(source_type& source, controlblock_type controlblock) : \
            base_class(source, controlblock) \
        {} \
        N(source_type& source, controlblock_type controlblock, const guard_type& guard) : \
            base_class(source, controlblock, guard) \
        {} \
    }

/**
 * The macros for indexed table definition in a interface
 * E - engine
 * N - name
 * T - type
 */
#define __OUROBOROS_INDEX_TABLE_TYPE(E, N, T) template < typename Record, template <typename> class Index, typename Controlblock > \
    class N : public T< E, Record, Index, Controlblock > \
    { \
        typedef T< E, Record, Index, Controlblock > base_class; \
    public: \
        typedef typename base_class::source_type source_type; \
        typedef typename base_class::controlblock_type controlblock_type; \
        typedef typename base_class::skey_type skey_type; \
        typedef typename base_class::guard_type guard_type; \
        N(source_type& source, skey_type& key) : \
            base_class(source, key) \
        {} \
        N(source_type& source, skey_type& key, const guard_type& guard) : \
            base_class(source, key, guard) \
        {} \
        N(source_type& source, controlblock_type controlblock) : \
            base_class(source, controlblock) \
        {} \
        N(source_type& source, controlblock_type controlblock, const guard_type& guard) : \
            base_class(source, controlblock, guard) \
        {} \
    }

/**
 * The macros for indexed table definition in a interface
 * N - name
 * T - type
 */
#define OUROBOROS_INDEX_TABLE_TYPE(N, T) __OUROBOROS_INDEX_TABLE_TYPE(interface_table, N, T)

/**
 * The base interface for working with a dataset
 */
template <typename TableInterface, template <template <typename, typename, typename> class,
        typename, template <typename> class, typename> class Table>
struct base_interface : public TableInterface
{
    OUROBOROS_INDEX_TABLE_TYPE(table_type, Table);
    OUROBOROS_TABLE_TYPE(key_table_type, data_table);
    OUROBOROS_TABLE_TYPE(info_table_type, data_table);
};

/**
 * The base interface for working with a dataset in local memory
 * (without separation between processes)
 */
template <template <template <typename, typename, typename> class, typename,
        template <typename> class, typename> class Table, int pageCount = OUROBOROS_PAGE_COUNT>
struct base_local_interface : public base_interface<base_table_local_interface<pageCount>, Table>,
    public base_dataset_local_interface {};

/**
 * The base interface for working with a dataset stored in local memory
 * (without separation between processes)
 */
template <template <template <typename, typename, typename> class, typename,
        template <typename> class, typename> class Table>
struct base_memory_interface : public base_interface<base_table_memory_interface, Table>,
    public base_dataset_local_interface {};

/**
 * The interface for working with a dataset stored in local memory
 * (without separation between processes)
 */
typedef base_local_interface<indexed_table> local_interface;

}   //namespace ouroboros


#endif	/* OUROBOROS_INTERFACE_H */

