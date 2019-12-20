/**
 * @file   nodecache.h
 * The cache of nodes
 */

#ifndef STG_NODECACHE_H
#define STG_NODECACHE_H

#include "ouroboros/global.h"
#include "ouroboros/hashmap.h"

namespace ouroboros
{

/**
 * The cache of nodes
 */
template <typename Node, typename Table>
class node_cache
{
public:
    typedef Node node_type;
    typedef Table table_type;
    typedef typename table_type::record_type record_type;

    static void static_begin(table_type *table = NULL);
    static void static_end();
    static bool static_read(const pos_type pos, node_type& node);
    static bool static_write(const pos_type pos, const node_type& node);
    static void static_keep(const pos_type pos, const node_type& node);
protected:
    enum state_type
    {
        ST_RD,
        ST_WR
    };
    struct item
    {
        state_type state;
        node_type node;
        item(const state_type s, const node_type& n) : state(s), node(n) {}
    };
    typedef hash_map<pos_type, item, 32> cache_type;

    static node_cache& instance();

    void begin(table_type *table = NULL);
    void end();
    bool read(const pos_type pos, node_type& node) const;
    bool write(const pos_type pos, const node_type& node);
    void keep(const pos_type pos, const node_type& node);
private:
    node_cache();
private:
    bool m_enabled;
    cache_type m_cache;
    table_type *m_table;
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
void node_cache<Node, Table>::static_begin(table_type *table)
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
    m_enabled(false),
    m_table(NULL)
{
}

/**
 * Start caching
 * @param table the table of cached nodes
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::begin(table_type *table)
{
    m_table = table;
    m_cache.clear();
    m_enabled = true;
}

/**
 * Stop caching
 */
template <typename Node, typename Table>
void node_cache<Node, Table>::end()
{
    if (m_enabled && m_table != NULL)
    {
        const typename cache_type::const_iterator itend = m_cache.end();
        for (typename cache_type::const_iterator it = m_cache.begin(); it != itend; ++it)
        {
            if (ST_WR == it->second.state)
            {
//                std::cout << "+";
                const record_type record(it->second.node);
                m_table->unsafe_write(record, it->first);
            }
        }
    }
    m_table = NULL;
    m_cache.clear();
    m_enabled = false;
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
    if (m_enabled)
    {
        const typename cache_type::const_iterator it = m_cache.find(pos);
        if (it != m_cache.end())
        {
            node = it->second.node;
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
void node_cache<Node, Table>::keep(const pos_type pos, const node_type& node)
{
    if (m_enabled)
    {
        m_cache.insert(typename cache_type::value_type(pos, item(ST_RD, node)));
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
    if (m_enabled)
    {
        typename cache_type::iterator it = m_cache.find(pos);
        if (m_cache.end() == it)
        {
            m_cache.insert(typename cache_type::value_type(pos, item(ST_WR, node)));
        }
        else
        {
            it->second.state = ST_WR;
            it->second.node = node;
        }
        return NULL == m_table;
    }
    return true;
}

}   //namespace ouroboros

#endif /* STG_NODECACHE_H */

