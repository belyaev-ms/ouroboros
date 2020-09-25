/**
 * @file   global.h
 * Global settings for the ouroboros
 */

#ifndef OUROBOROS_GLOBAL_H
#define	OUROBOROS_GLOBAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef OUROBOROS_PORT
#include "ouroboros_port.h"
#endif

#include "ouroboros/log.h"
#include "ouroboros/error.h"

#ifndef OUROBOROS_SETTINGS
#define OUROBOROS_LOCK_TIMEOUT 5000 ///< timeout for unlocking (ms)
#define OUROBOROS_OPTIMIZATION_NODE_RW ///< optimize i/o opeartions of rbtree
#define OUROBOROS_FASTRBTREE_ENABLED ///< use fastrbtree
#define OUROBOROS_NODECACHE_ENABLED ///< use cache of nodes for rbtree
#define OUROBOROS_FILE_REGION_CACHE_TYPE 1 ///< use the first type of file region cache
//#define OUROBOROS_SYNC_ENABLED ///< use sync operation for fixation of file data
//#define OUROBOROS_SHOW_MEMORY_INFO ///< show information about the status of shared memory
//#define OUROBOROS_STRICT_ASSERT_ENABLED ///< use strict assert
#endif

namespace ouroboros
{

typedef uint32_t pos_type;      ///< position
typedef int32_t  spos_type;     ///< signed position
typedef uint32_t offset_type;   ///< offset
typedef uint32_t revision_type; ///< revision
typedef uint32_t count_type;    ///< count
typedef uint32_t size_type;     ///< size

const pos_type NIL = -1; ///< pointer of position NULL

/** kind of table */
enum
{
    TABLE_SIMPLE,   ///< simple table
    TABLE_INDEX,    ///< indexed table
    TABLE_TREE      ///< table based on rb-tree
};

#ifndef OUROBOROS_SETTINGS
/** default settings of cache */
enum
{
    OUROBOROS_PAGE_SIZE = 512,    ///< size of cache page
    OUROBOROS_PAGE_COUNT = 16     ///< count of cache pages
};
#endif

}   // namespace ouroboros

#endif	/* OUROBOROS_GLOBAL_H */

