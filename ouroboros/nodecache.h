/**
 * @file   nodecache.h
 * The cache of nodes
 */

#ifndef STG_NODECACHE_H
#define STG_NODECACHE_H

#include "ouroboros/global.h"
#include "ouroboros/cache.h"

namespace ouroboros
{

/**
 * The cache of nodes
 */
template <typename Node, typename Table>
class node_cache
{
    friend cache <node_cache, sizeof(Node), 64>;
public:
    typedef Node node_type;
    typedef Table table_type;
    typedef typename table_type::record_type record_type;

    static void static_begin(table_type& table);
    static void static_end();
    static void static_cancel();
    static bool static_read(const pos_type pos, node_type& node);
    static bool static_write(const pos_type pos, const node_type& node);
    static void static_keep(const pos_type pos, const node_type& node);
protected:
    typedef cache<node_cache, sizeof(node_type), 64> cache_type;

    static node_cache& instance();

    void begin(table_type& table);
    void end();
    void cancel();
    bool read(const pos_type pos, node_type& node) const;
    bool write(const pos_type pos, const node_type& node);
    void keep(const pos_type pos, const node_type& node) const;
    void save_page(const pos_type index, void *data);
private:
    node_cache();
private:
    table_type *m_table;
    cache_type m_cache;
};

/**
 * The guard of a cache
 */
template <typename Cache>
struct cache_guard
{
    typedef Cache cache_type;
    typedef typename cache_type::table_type table_type;
    cache_guard(table_type& table)
    {
        cache_type::static_begin(table);
    }
    ~cache_guard()
    {
        cache_type::static_end();
    }
};

//==============================================================================
//  node_cache
//==============================================================================
/**
 * Get the instance of the cache
 * @return the instance of the cache
 */
//static
template <typename Node, typename Table>
node_cache<Node, Table>& node_cache<Node, Table>::instance()
{
    static node_cache self;
    return self;
}

/**
 * Start caching
 * @param table the pointer to the table
 */
//static
template <typename Node, typename Table>
void node_cache<Node, Table>::static_begin(table_type& table)
{
    instance().begin(table);
}

/**
 * Stop caching
 */
//static
template <typename Node, typename Table>
void node_cache<Node, Table>::static_end()
{
    instance().end();
}

/**
 * Cancel caching
 */
//static
template <typename Node, typename Table>
void node_cache<Node, Table>::static_cancel()
{
    instance().cancel();
}

/**
 * Read a node from the cache
 * @param pos the position of the node
 * @param node the node
 * @return the result of the reading
 */
//static
template <typename Node, typename Table>
bool node_cache<Node, Table>::static_read(const pos_type pos, node_type& node)
{
    return instance().read(pos, node);
}

/**
 * Write a node to the cache
 * @param pos the position of the node
 * @param node the node
 * @return if the result is true then need to write the node to the table
 */
//static
template <typename Node, typename Table>
bool node_cache<Node, Table>::static_write(const pos_type pos, const node_type& node)
{
    return instance().write(pos, node);
}

/**
 * keep the node in the cache
 * @param pos the position of the cache
 * @param node the node
 */
//static
template <typename Node, typename Table>
void node_cache<Node, Table>::static_keep(const pos_type pos, const node_type& node)
{
    instance().keep(pos, node);
}

/**
 * Constructor
 */
template <typename Node, typename Table>
node_cache<Node, Table>::node_cache() :
    m_table(NULL),
    m_cache(*this)
{
}

/**
 * Start caching
 * @param table the table of cached nodes
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::begin(table_type& table)
{
    m_cache.free();
    m_table = &table;
}

/**
 * Stop caching
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::end()
{
    if (m_table != NULL)
    {
        m_cache.free();
    }
    m_table = NULL;
}

/**
 * Cancel caching
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::cancel()
{
    if (m_table != NULL)
    {
        m_table = NULL;
        m_cache.free();
    }
}

/**
 * Save a node
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::save_page(const pos_type index, void *data)
{
    if (m_table != NULL)
    {
        const record_type record(*reinterpret_cast<node_type*>(data));
        m_table->unsafe_write(record, index);
    }
}

/**
 * Read a node from the cache
 * @param pos the position of the node
 * @param node the node
 * @return the result of reading
 */
template <typename Node, typename Table>
bool node_cache<Node, Table>::read(const pos_type pos, node_type& node) const
{
    if (m_table != NULL)
    {
        const typename cache_type::page_status_type status = m_cache.page_exists(pos);
        if (status.state() != PG_DETACHED)
        {
            node = *reinterpret_cast<node_type*>(m_cache.get_page(status));
            return true;
        }
    }
    return false;
}

/**
 * Keep a node in the cache
 * @param pos the position of the node
 * @param node the node
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::keep(const pos_type pos, const node_type& node) const
{
    if (m_table != NULL)
    {
        *reinterpret_cast<node_type*>(m_cache.get_page(pos)) = node;
    }
}

/**
 * Write a node to the cache
 * @param pos the position of the node
 * @param node the node
 * @return if the result is true then need to write the node to the table
 */
template <typename Node, typename Table>
bool node_cache<Node, Table>::write(const pos_type pos, const node_type& node)
{
    if (m_table != NULL)
    {
        *reinterpret_cast<node_type*>(m_cache.get_page(pos)) = node;
        return false;
    }
    return true;
}

}   //namespace ouroboros

#endif /* STG_NODECACHE_H */

