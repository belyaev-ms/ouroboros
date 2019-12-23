/**
 * @file   node.h
 * The nodes of the rb-tree that is placed in a table
 */

#ifndef OUROBOROS_NODE_H
#define	OUROBOROS_NODE_H

#include <stddef.h>

#include "ouroboros/global.h"
#include "ouroboros/nodecache.h"

namespace ouroboros
{

enum node_color { RED, BLACK };

/**
 * The simple converter from a body to a key of a node
 */
template <typename Key, typename Body>
struct simple_convertor
{
    typedef Key key_type;
    typedef Body body_type;
    inline const key_type key(const body_type& body) const
    {
        return body;
    }
};

/**
 * The simple node of a data container
 */
template <typename Key, typename Body, typename Converter = simple_convertor<Key, Body> >
class data_node
{
public:
    typedef data_node<Key, Body, Converter> self_type;
    typedef Key key_type;
    typedef Body body_type;
    typedef Converter converter;

    data_node(const body_type& in_body, const pos_type in_parent, const node_color in_color);
    data_node(const body_type& in_body, const pos_type in_parent);
    data_node(const body_type& in_body);
    data_node();
    data_node(const self_type& node);

    inline pos_type parent() const;
    inline pos_type left() const;
    inline pos_type right() const;
    inline node_color color() const;
    inline bool parent(const pos_type pos);
    inline bool left(const pos_type pos);
    inline bool right(const pos_type pos);
    inline bool color(const node_color in_color);

    inline const key_type key() const;
    body_type& body();
    const body_type& body() const;
    bool body(const body_type& body);

    self_type& operator= (const self_type& node);

    inline bool operator== (const self_type& node) const;
    inline bool operator!= (const self_type& node) const;
    inline bool operator< (const self_type& node) const;
    inline bool operator<= (const self_type& node) const;
    inline bool operator> (const self_type& node) const;
    inline bool operator>= (const self_type& node) const;
private:
    pos_type   m_parent;
    pos_type   m_left;
    pos_type   m_right;
    node_color m_color;
    body_type m_body;
};

/**
 * The simple extractor a node from a table record
 */
///@todo may be to add index
template <typename Node, typename Record>
struct simple_extractor
{
    typedef Node node_type;
    typedef Record record_type;
    inline const node_type node(const record_type& record) const
    {
        return record.field1();
    }
};

/**
 * The pointer to a node that is located in a table
 */
template <typename Node, typename Table, typename Extractor = simple_extractor<Node, typename Table::record_type> >
class table_pnode
{
    typedef node_cache<Node, Table> cache_type;
    typedef table_pnode<Node, Table, Extractor> self_type;
public:
    typedef Node node_type;
    typedef Table table_type;
    typedef typename table_type::record_type record_type;
    typedef Extractor extractor;
    typedef typename node_type::key_type key_type;
    typedef typename node_type::body_type body_type;
    typedef self_type static_pnode;
public:
    table_pnode(table_type& table);
    table_pnode(table_type& table, const pos_type pos);
    table_pnode(const self_type& pnode);
    virtual ~table_pnode();

    self_type parent() const;
    self_type left() const;
    self_type right() const;
    void parent(const self_type& pnode);
    void left(const self_type& pnode);
    void right(const self_type& pnode);

    const node_type get() const;
    void set(const node_type& node);
    const node_type get_parent() const;
    const node_type get_left() const;
    const node_type get_right() const;
    void set_parent(const node_type& node);
    void set_left(const node_type& node);
    void set_right(const node_type& node);
    pos_type pparent() const;
    pos_type pleft() const;
    pos_type pright() const;
    void pparent(const pos_type pos);
    void pleft(const pos_type pos);
    void pright(const pos_type pos);
    node_color color() const;
    void color(const node_color in_color);
    const key_type key() const;
    inline pos_type pos() const;
    virtual void pos(const pos_type in_pos);
    inline table_type& table() const;
    const body_type operator() () const;
    void operator() (const body_type& body);
    inline void reset() const {}; ///< todo
    self_type substitute(const self_type& pnode);
    self_type& operator= (const self_type& pnode);

    const self_type root() const;
    bool is_left_son() const;
    bool is_right_son() const;
    const self_type maximum() const;
    const self_type minimum() const;
    const self_type successor() const;
    const self_type predecessor() const;

    bool operator== (const self_type& pnode) const;
    bool operator!= (const self_type& pnode) const;
    bool operator< (const self_type& pnode) const;
    bool operator<= (const self_type& pnode) const;
    bool operator> (const self_type& pnode) const;
    bool operator>= (const self_type& pnode) const;
protected:
    inline const node_type read() const; ///< read data of the node
    inline void write(const node_type& node); ///< write data the node
    virtual const node_type read(const pos_type pos) const; ///< read data of a node
    virtual void write(const node_type& node, const pos_type pos); ///< write data of a node
private:
    table_pnode();
private:
    table_type *m_table;
    pos_type m_pos;
    static node_type s_sentinel;
};

/**
 * The smart pointer to a node that is located in a table
 * @details the pointer caches its data
 */
template <typename Node, typename Table, typename Extractor = simple_extractor<Node, typename Table::record_type> >
class table_smart_pnode : public table_pnode<Node, Table, Extractor>
{
    typedef table_pnode<Node, Table, Extractor> base_class;
    typedef table_smart_pnode<Node, Table, Extractor> self_type;
public:
    typedef Node node_type;
    typedef Table table_type;
    typedef typename table_type::record_type record_type;
    typedef Extractor extractor;
    typedef typename node_type::key_type key_type;
    typedef typename node_type::body_type body_type;
    typedef base_class static_pnode;
public:
    table_smart_pnode(table_type& table);
    table_smart_pnode(table_type& table, const pos_type pos);
    table_smart_pnode(const self_type& pnode);
    table_smart_pnode(const base_class& pnode);
    virtual ~table_smart_pnode();

    base_class& operator= (const self_type& pnode);
    base_class& operator= (const base_class& pnode);
    inline pos_type pos() const { return base_class::pos(); }; ///@todo
    virtual void pos(const pos_type in_pos);
    inline void reset() const; ///< reset the cache of data
protected:
    virtual const node_type read(const pos_type pos) const; ///< read data of a node
    virtual void write(const node_type& node, const pos_type pos); ///< write data the node
    inline void assign(const self_type& pnode) const; ///< assign the cache of data
private:
    mutable bool m_cached;
    mutable node_type m_self;
};

/**
 * The iterator of a node
 */
template <typename PNode>
class node_iterator
{
    typedef node_iterator<PNode> self_type;
public:
    typedef PNode pnode_type;
    typedef typename pnode_type::table_type table_type;

    node_iterator(table_type& table);
    node_iterator(const pnode_type& pnode);
    node_iterator(const self_type& iter);

    self_type operator++ ();
    self_type operator++ (int );
    self_type operator-- ();
    self_type operator-- (int );

    const self_type operator++ () const;
    const self_type operator++ (int ) const;
    const self_type operator-- () const;
    const self_type operator-- (int ) const;

    bool operator== (const self_type& iter) const;
    bool operator!= (const self_type& iter) const;

    pnode_type* operator-> ();
    pnode_type& operator* ();
    const pnode_type* operator-> () const;
    const pnode_type& operator* () const;
    self_type& operator= (const self_type& iter);
    const self_type& operator= (const self_type& iter) const;

    inline table_type& table() const;
private:
    mutable pnode_type m_iter;
};

/**
 * The reverse iterator of a node
 */
template <typename Iterator>
class node_reverseiterator
{
    typedef node_reverseiterator<Iterator> self_type;
public:
    typedef Iterator iterator;
    typedef typename iterator::pnode_type pnode_type;
    typedef typename iterator::table_type table_type;

    node_reverseiterator(table_type& table);
    node_reverseiterator(const pnode_type& pnode);
    node_reverseiterator(const self_type& iter);
    node_reverseiterator(const iterator& iter);

    iterator base() const;

    self_type operator++ ();
    self_type operator++ (int );
    self_type operator-- ();
    self_type operator-- (int );

    const self_type operator++ () const;
    const self_type operator++ (int ) const;
    const self_type operator-- () const;
    const self_type operator-- (int ) const;

    bool operator== (const self_type& iter) const;
    bool operator!= (const self_type& iter) const;

    pnode_type* operator-> ();
    pnode_type& operator* ();
    const pnode_type* operator-> () const;
    const pnode_type& operator* () const;
    self_type& operator= (const self_type& iter);
    const self_type& operator= (const self_type& iter) const;
    self_type& operator= (const iterator& iter);
    const self_type& operator= (const iterator& iter) const;

    inline table_type& table() const;
private:
    static iterator pred_node(const iterator& iter);
private:
    iterator m_current;
};

//==============================================================================
//  data_node
//==============================================================================
/**
 * Constructor
 * @param in_body body of the node
 * @param in_parent the position of the parent node
 * @param in_color the color of the node
 */
template <typename Key, typename Body, typename Converter>
data_node<Key, Body, Converter>::data_node(const body_type& in_body, const pos_type in_parent, const node_color in_color) :
    m_parent(in_parent),
    m_left(NIL),
    m_right(NIL),
    m_color(in_color),
    m_body(in_body)
{
}

/**
 * Constructor
 * @param in_body body of the node
 * @param in_parent the position of the parent node
 */
template <typename Key, typename Body, typename Converter>
data_node<Key, Body, Converter>::data_node(const body_type& in_body, const pos_type in_parent) :
    m_parent(in_parent),
    m_left(NIL),
    m_right(NIL),
    m_color(BLACK),
    m_body(in_body)
{
}

/**
 * Constructor
 * @param in_body body of the node
 */
template <typename Key, typename Body, typename Converter>
data_node<Key, Body, Converter>::data_node(const body_type& in_body) :
    m_parent(NIL),
    m_left(NIL),
    m_right(NIL),
    m_color(BLACK),
    m_body(in_body)
{
}

/**
 * Constructor
 * @param node the another node
 */
template <typename Key, typename Body, typename Converter>
data_node<Key, Body, Converter>::data_node(const self_type& node) :
    m_parent(node.m_parent),
    m_left(node.m_left),
    m_right(node.m_right),
    m_color(node.m_color),
    m_body(node.m_body)
{
}

/**
 * Constructor
 */
template <typename Key, typename Body, typename Converter>
data_node<Key, Body, Converter>::data_node() :
    m_parent(NIL),
    m_left(NIL),
    m_right(NIL),
    m_color(BLACK)
{
}

/**
 * Get the position of the parent node
 * @return the position of the parent node
 */
template <typename Key, typename Body, typename Converter>
inline pos_type data_node<Key, Body, Converter>::parent() const
{
    return m_parent;
}

/**
 * Get the position of the left node
 * @return the position of the left node
 */
template <typename Key, typename Body, typename Converter>
inline pos_type data_node<Key, Body, Converter>::left() const
{
    return m_left;
}

/**
 * Get the position of the right node
 * @return the position of the right node
 */
template <typename Key, typename Body, typename Converter>
inline pos_type data_node<Key, Body, Converter>::right() const
{
    return m_right;
}

/**
 * Get the color of the node
 * @return the color of the node
 */
template <typename Key, typename Body, typename Converter>
inline node_color data_node<Key, Body, Converter>::color() const
{
    return m_color;
}

/**
 * Set the position of the parent node
 * @param pos the position of the parent node
 * @return there was a change
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::parent(const pos_type pos)
{
    if (m_parent != pos)
    {
        m_parent = pos;
        return true;
    }
    return false;
}

/**
 * Set the position of the left node
 * @param pos the position of the left node
 * @return there was a change
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::left(const pos_type pos)
{
    if (m_left != pos)
    {
        m_left = pos;
        return true;
    }
    return false;
}

/**
 * Set the position of the right node
 * @param pos the position of the right node
 * @return there was a change
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::right(const pos_type pos)
{
    if (m_right != pos)
    {
        m_right = pos;
        return true;
    }
    return false;
}

/**
 * Set the color of the node
 * @param in_color the color of the node
 * @return there was a change
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::color(const node_color in_color)
{
    if (m_color != in_color)
    {
        m_color = in_color;
        return true;
    }
    return false;
}

/**
 * Get the key of the node
 * @return the key of the node
 */
template <typename Key, typename Body, typename Converter>
inline const typename data_node<Key, Body, Converter>::key_type
    data_node<Key, Body, Converter>::key() const
{
    return converter().key(body());
}

/**
 * Get the body of the node
 * @return the body of the node
 */
template <typename Key, typename Body, typename Converter>
inline typename data_node<Key, Body, Converter>::body_type&
    data_node<Key, Body, Converter>::body()
{
    return m_body;
}

/**
 * Get the body of the node
 * @return the body of the node
 */
template <typename Key, typename Body, typename Converter>
inline const typename data_node<Key, Body, Converter>::body_type&
    data_node<Key, Body, Converter>::body() const
{
    return m_body;
}

/**
 * Set the body of the node
 * @param in_body the body of the node
 * @return there was a change
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::body(const body_type& in_body)
{
    if (m_body != in_body)
    {
        m_body = in_body;
        return true;
    }
    return false;
}

/**
 * Operator =
 * @param node the another node
 * @return the reference to the node
 */
template <typename Key, typename Body, typename Converter>
typename data_node<Key, Body, Converter>::self_type&
    data_node<Key, Body, Converter>::operator= (const self_type& node)
{
    m_parent = node.m_parent;
    m_left   = node.m_left;
    m_right  = node.m_right;
    m_color  = node.m_color;
    m_body   = node.m_body;
    return *this;
}

/**
 * Operator ==
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator== (const self_type& node) const
{
    return (m_parent == node.m_parent) && (m_left == node.m_left) && (m_right == node.m_right)
        && (m_color == node.m_color) && (m_body == node.m_body);
}

/**
 * Operator !=
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator!= (const self_type& node) const
{
    return (m_parent != node.m_parent) || (m_left != node.m_left) || (m_right != node.m_right)
        || (m_color != node.m_color) || (m_body != node.m_body);
}

/**
 * Operator <
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator< (const self_type& node) const
{
    return key() < node.key();
}

/**
 * Operator <=
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator<= (const self_type& node) const
{
    return key() <= node.key();
}

/**
 * Operator >
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator> (const self_type& node) const
{
    return key() > node.key();
}

/**
 * Operator >=
 * @param node the another node
 * @return the result of the checking
 */
template <typename Key, typename Body, typename Converter>
inline bool data_node<Key, Body, Converter>::operator>= (const self_type& node) const
{
    return key() >= node.key();
}

//==============================================================================
//  table_pnode
//==============================================================================
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::s_sentinel = node_type();

/**
 * Constructor
 * @param table the table of nodes
 */
template <typename Node, typename Table, typename Extractor>
table_pnode<Node, Table, Extractor>::table_pnode(table_type& table) :
    m_table(&table),
    m_pos(NIL)
{

}

/**
 * Constructor
 * @param table the table of nodes
 * @param pos the position of the node
 */
template <typename Node, typename Table, typename Extractor>
table_pnode<Node, Table, Extractor>::table_pnode(table_type& table, const pos_type pos) :
    m_table(&table),
    m_pos(pos)
{

}

/**
 * Constructor
 * @param pnode the pointer to the node
 */
template <typename Node, typename Table, typename Extractor>
table_pnode<Node, Table, Extractor>::table_pnode(const self_type& pnode) :
    m_table(pnode.m_table),
    m_pos(pnode.m_pos)
{

}

/**
 * Destructor
 */
//virtual
template <typename Node, typename Table, typename Extractor>
table_pnode<Node, Table, Extractor>::~table_pnode()
{

}

/**
 * Read data of a node
 * @param pos the position of a node
 * @return data of a node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::read(const pos_type pos) const
{
    if (NIL == pos)
    {
        return s_sentinel;
    }
    else
    {
#ifdef OUROBOROS_NODECACHE_ENABLED
        node_type node;
        if (cache_type::static_read(pos, node))
        {
            return node;
        }
        else
#endif
        {
            record_type record;
            m_table->unsafe_read(record, pos);
#ifndef OUROBOROS_NODECACHE_ENABLED
            return extractor().node(record);
#else
            node = extractor().node(record);
            cache_type::static_keep(pos, node);
            return node;
#endif
        }
    }
}

/**
 * Write data of a node
 * @param node data of a node
 * @param pos a position of a node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::write(const node_type& node, const pos_type pos)
{
    if (NIL == pos)
    {
        s_sentinel = node;
    }
    else
    {
#ifdef OUROBOROS_NODECACHE_ENABLED
        if (cache_type::static_write(pos, node))
#endif
        {
            const record_type record(node);
            m_table->unsafe_write(record, pos);
        }
    }
}

/**
 * Read data of the node
 * @return data of the node
 */
template <typename Node, typename Table, typename Extractor>
inline const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::read() const
{
    return read(m_pos);
}

/**
 * Write  data of the node
 * @param node data of the node
 */
template <typename Node, typename Table, typename Extractor>
inline void table_pnode<Node, Table, Extractor>::write(const node_type& node)
{
    write(node, m_pos);
}

/**
 * Get data of the node
 * @return data of the node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::get() const
{
    return read();
}

/**
 * Set data of the node
 * @param node data of the node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::set(const node_type& node)
{
    write(node);
}

/**
 * Get the body of the node
 * @return the body of the node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::body_type
    table_pnode<Node, Table, Extractor>::operator() () const
{
    return read().body;
}

/**
 * Set the body of the node
 * @param body the body of the node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::operator() (const body_type& body)
{
    node_type node = read();
    node.body = body;
    write(node);
}

/**
 * Get the parent node
 * @return the parent node
 */
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::parent() const
{
    return self_type(*m_table, read().parent());
}

/**
 * Get the left node
 * @return the left node
 */
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::left() const
{
    return self_type(*m_table, read().left());
}

/**
 * Get the right node
 * @return the right node
 */
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::right() const
{
    return self_type(*m_table, read().right());
}

/**
 * Get data of the parent node
 * @return data of the parent node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::get_parent() const
{
    return read(read().parent());
}

/**
 * Get data of the left node
 * @return data of the left node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::get_left() const
{
    return read(read().left());
}

/**
 * Get data of the right node
 * @return data of the right node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::node_type
    table_pnode<Node, Table, Extractor>::get_right() const
{
    return read(read().right());
}

/**
 * Set data of the parent node
 * @param node data of the parent node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::set_parent(const node_type& node)
{
    write(node, read().parent());
}

/**
 * Set data of the left node
 * @param node data of the left node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::set_left(const node_type& node)
{
    write(node, read().left());
}

/**
 * Set data of the right node
 * @param node data of the right node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::set_right(const node_type& node)
{
    write(node, read().right());
}

/**
 * Get the color of the node
 * @return the color of the node
 */
template <typename Node, typename Table, typename Extractor>
node_color table_pnode<Node, Table, Extractor>::color() const
{
    return read().color();
}

/**
 * Get the key of the node
 * @return the key of the node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::key_type
    table_pnode<Node, Table, Extractor>::key() const
{
    return read().key();
}

/**
 * Get the position of the node
 * @return the position of the node
 */
template <typename Node, typename Table, typename Extractor>
inline pos_type table_pnode<Node, Table, Extractor>::pos() const
{
    return m_pos;
}

/**
 * Set the position of the node
 * @param in_pos the position of the node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::pos(const pos_type in_pos)
{
    m_pos = in_pos;
}

/**
 * Get the reference to the table of nodes
 * @return the reference to the table of nodes
 */
template <typename Node, typename Table, typename Extractor>
inline typename table_pnode<Node, Table, Extractor>::table_type&
    table_pnode<Node, Table, Extractor>::table() const
{
    return *m_table;
}

/**
 * Get the position of the parent node
 * @return the position of the parent node
 */
template <typename Node, typename Table, typename Extractor>
pos_type table_pnode<Node, Table, Extractor>::pparent() const
{
    return read().parent();
}

/**
 * Get the position of the left node
 * @return the position of the left node
 */
template <typename Node, typename Table, typename Extractor>
pos_type table_pnode<Node, Table, Extractor>::pleft() const
{
    return read().left();
}

/**
 * Get the position of the right node
 * @return the position of the right node
 */
template <typename Node, typename Table, typename Extractor>
pos_type table_pnode<Node, Table, Extractor>::pright() const
{
    return read().right();
}

/**
 * Set the position of the parent node
 * @param pos the position of the parent node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::pparent(const pos_type pos)
{
    node_type node = read();
    if (node.parent(pos))
    {
        write(node);
    }
}

/**
 * Set the position of the left node
 * @param pos the position of the left node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::pleft(const pos_type pos)
{
    node_type node = read();
    if (node.left(pos))
    {
        write(node);
    }
}

/**
 * Set the position of the right node
 * @param pos the position of the right node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::pright(const pos_type pos)
{
    node_type node = read();
    if (node.right(pos))
    {
        write(node);
    }
}

/**
 * Set the parent node
 * @param pnode the pointer to the parent node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::parent(const self_type& pnode)
{
    node_type self = read();
    if (self.parent(pnode.pos()))
    {
        write(self);
    }
}

/**
 * Set the left node
 * @param pnode the pointer to the left node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::left(const self_type& pnode)
{
    node_type self = read();
    if (self.left(pnode.pos()))
    {
        write(self);
    }
}

/**
 * Set the right node
 * @param pnode the pointer to the right node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::right(const self_type& pnode)
{
    node_type self = read();
    if (self.right(pnode.pos()))
    {
        write(self);
    }
}

/**
 * Set the color of the node
 * @param in_color the color of the node
 */
template <typename Node, typename Table, typename Extractor>
void table_pnode<Node, Table, Extractor>::color(const node_color in_color)
{
    node_type node = read();
    if (node.color(in_color))
    {
        write(node);
    }
}

/**
 * Substitute the node
 * @param pnode the pointer to the node to be substituted
 * @return the pointer to the substituted node
 */
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::substitute(const self_type& pnode)
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    pleft(pnode.pleft());
    pright(pnode.pright());
    pparent(pnode.pparent());
    color(pnode.color());
    pnode.left().pparent(m_pos);
    pnode.right().pparent(m_pos);
    if (pnode.is_left_son())
    {
        pnode.parent().pleft(m_pos);
    }
    else
    {
        pnode.parent().pright(m_pos);
    }
    return pnode;
#else
    {
        node_type self = read();
        node_type node = read(pnode.m_pos);
        self.left(node.left());
        self.right(node.right());
        self.parent(node.parent());
        self.color(node.color());
        write(self);
    }
    pnode.left().pparent(m_pos);
    pnode.right().pparent(m_pos);
    if (pnode.is_left_son())
    {
        pnode.parent().pleft(m_pos);
    }
    else
    {
        pnode.parent().pright(m_pos);
    }
    return pnode;
#endif
}

/**
 * Get the root node
 * @return the root node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::root() const
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    return (NIL == pparent()) ? *this : parent().root();
#else
    self_type pnode(*this);
    while (pnode.pparent() != NIL)
    {
        pnode = pnode.parent();
    }
    return pnode;
#endif
}

/**
 * Check if the node is left
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::is_left_son() const
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    return (pparent() != NIL) && (parent().pleft() == pos());
#else
    node_type self = read();
    if (self.parent() != NIL)
    {
        node_type parent = read(self.parent());
        return parent.left() == pos();
    }
    return false;
#endif
}

/**
 * Check if the node is right
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::is_right_son() const
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    return (pparent() != NIL) && (parent().pright() == pos());
#else
    node_type self = read();
    if (self.parent() != NIL)
    {
        node_type parent = read(self.parent());
        return parent.right() == pos();
    }
    return false;
#endif
}

/**
 * Get the sub node that has a maximal key
 * @return the sub node that has a maximal key
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::maximum() const
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    return (NIL == pright()) ? *this : right().maximum();
#else
    self_type max(*this);
    while (max.pright() != NIL)
    {
        max = max.right();
    }
    return max;
#endif
}

/**
 * Get the sub node that has a minimal key
 * @return the sub node that has a minimal key
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::minimum() const
{
#ifndef OUROBOROS_OPTIMIZATION_NODE_RW
    return (NIL == pleft()) ? *this : left().minimum();
#else
    self_type min(*this);
    while (min.pleft() != NIL)
    {
        min = min.left();
    }
    return min;
#endif
}

/**
 * Get the successor of the node
 * @return the successor of the node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::successor() const
{
    if (pright() != NIL)
    {
        return right().minimum();
    }

    if (is_left_son())
    {
        return parent();
    }

    self_type succ = *this;
    do
    {
        succ = succ.parent();
    } while (succ.pos() != NIL && succ.is_right_son());

    return (succ.pos() != NIL) ? succ.parent() : self_type(*m_table, NIL);
}

/**
 * Get the predecessor of the node
 * @return the predecessor of the node
 */
template <typename Node, typename Table, typename Extractor>
const typename table_pnode<Node, Table, Extractor>::self_type
    table_pnode<Node, Table, Extractor>::predecessor() const
{
    if (pleft() != NIL)
    {
        return left().maximum();
    }

    if (is_right_son())
    {
        return parent();
    }

    self_type pred = *this;
    do
    {
        pred = pred.parent();
    } while (pred.pos() != NIL && pred.is_left_son());

    return (pred.pos() != NIL) ? pred.parent() : self_type(*m_table, NIL);
}

/**
 * Operator =
 * @param pnode the pointer to another node
 * @return reference to yourself
 */
template <typename Node, typename Table, typename Extractor>
typename table_pnode<Node, Table, Extractor>::self_type&
    table_pnode<Node, Table, Extractor>::operator= (const self_type& pnode)
{
    m_table = pnode.m_table;
    m_pos = pnode.m_pos;
    return *this;
}

/**
 * Operator ==
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator== (const self_type& pnode) const
{
    return key() == pnode.key();
}

/**
 * Operator !=
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator!= (const self_type& pnode) const
{
    return key() != pnode.key();
}

/**
 * Operator <
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator< (const self_type& pnode) const
{
    return get() < pnode.get();
}

/**
 * Operator <=
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator<= (const self_type& pnode) const
{
    return get() <= pnode.get();
}

/**
 * Operator >
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator> (const self_type& pnode) const
{
    return get() > pnode.get();
}

/**
 * Operator >=
 * @param pnode the pointer to another node
 * @return the result of the checking
 */
template <typename Node, typename Table, typename Extractor>
bool table_pnode<Node, Table, Extractor>::operator>= (const self_type& pnode) const
{
    return get() >= pnode.get();
}

//==============================================================================
//  table_smart_pnode
//==============================================================================
/**
 * Constructor
 * @param table the table of nodes
 */
template <typename Node, typename Table, typename Extractor>
table_smart_pnode<Node, Table, Extractor>::table_smart_pnode(table_type& table) :
    base_class(table),
    m_cached(false)
{

}

/**
 * Constructor
 * @param table the table of nodes
 * @param pos the position of the node
 */
template <typename Node, typename Table, typename Extractor>
table_smart_pnode<Node, Table, Extractor>::table_smart_pnode(table_type& table, const pos_type pos) :
    base_class(table, pos),
    m_cached(false)
{

}

/**
 * Constructor
 * @param pnode the pointer to the node
 */
template <typename Node, typename Table, typename Extractor>
table_smart_pnode<Node, Table, Extractor>::table_smart_pnode(const self_type& pnode) :
    base_class(pnode),
    m_cached(false)
{
    assign(pnode);
}

/**
 * Constructor
 * @param pnode the pointer to the node
 */
template <typename Node, typename Table, typename Extractor>
table_smart_pnode<Node, Table, Extractor>::table_smart_pnode(const base_class& pnode) :
    base_class(pnode),
    m_cached(false)
{

}

/**
 * Destructor
 */
//virtual
template <typename Node, typename Table, typename Extractor>
table_smart_pnode<Node, Table, Extractor>::~table_smart_pnode()
{
    reset();
}

/**
 * Operator =
 * @param pnode the pointer to another node
 * @return reference to yourself
 */
template <typename Node, typename Table, typename Extractor>
typename table_smart_pnode<Node, Table, Extractor>::base_class&
    table_smart_pnode<Node, Table, Extractor>::operator= (const self_type& pnode)
{
    reset();
    assign(pnode);
    return base_class::operator =(pnode);
}

/**
 * Operator =
 * @param pnode the pointer to another node
 * @return reference to yourself
 */
template <typename Node, typename Table, typename Extractor>
typename table_smart_pnode<Node, Table, Extractor>::base_class&
    table_smart_pnode<Node, Table, Extractor>::operator= (const base_class& pnode)
{
    reset();
    return base_class::operator =(pnode);
}

/**
 * Set the position of the node
 * @param in_pos the position of the node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
void table_smart_pnode<Node, Table, Extractor>::pos(const pos_type in_pos)
{
    reset();
    base_class::pos(in_pos);
}

/**
 * Read data of a node
 * @param pos the position of a node
 * @return data of a node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
const typename table_smart_pnode<Node, Table, Extractor>::node_type
    table_smart_pnode<Node, Table, Extractor>::read(const pos_type pos) const
{
    if (NIL == pos || base_class::pos() != pos)
    {
        return base_class::read(pos);
    }
    if (!m_cached)
    {
        m_cached = true;
        m_self = base_class::read(pos);
    }
    return m_self;
}

/**
 * Write data of a node
 * @param node data of a node
 * @param pos a position of a node
 */
//virtual
template <typename Node, typename Table, typename Extractor>
void table_smart_pnode<Node, Table, Extractor>::write(const node_type& node, const pos_type pos)
{
    if (m_cached && NIL != pos && base_class::pos() == pos)
    {
        m_self = node;
    }
    base_class::write(node, pos);
}

/**
 * Reset the cache of data
 */
template <typename Node, typename Table, typename Extractor>
inline void table_smart_pnode<Node, Table, Extractor>::reset() const
{
    m_cached = false;
}

/**
 * Assign the cache of data
 * @param pnode the pointer to another node
 */
template <typename Node, typename Table, typename Extractor>
inline void table_smart_pnode<Node, Table, Extractor>::assign(const self_type& pnode) const
{
    m_cached = pnode.m_cached;
    m_self = pnode.m_self;
    pnode.m_cached = false;
}

//==============================================================================
//  node_iterator
//==============================================================================
/**
 * Constructor
 * @param table the table of nodes
 */
template <typename PNode>
node_iterator<PNode>::node_iterator(table_type& table) :
    m_iter(table)
{

}

/**
 * Constructor
 * @param pnode the pointer to the node
 */
template <typename PNode>
node_iterator<PNode>::node_iterator(const pnode_type& pnode) :
    m_iter(pnode)
{

}

/**
 * Constructor
 * @param iter another iterator
 */
template <typename PNode>
node_iterator<PNode>::node_iterator(const self_type& iter) :
    m_iter(iter.m_iter)
{

}

/**
 * Operator ++
 * @return the iterator to next node
 */
template <typename PNode>
typename node_iterator<PNode>::self_type node_iterator<PNode>::operator++ ()
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    m_iter = m_iter.successor();
    return *this;
}

/**
 * Operator ++
 * @return the iterator to current node
 */
template <typename PNode>
typename node_iterator<PNode>::self_type node_iterator<PNode>::operator++ (int )
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    pnode_type temp = m_iter;
    operator++();
    return self_type(temp);
}

/**
 * Operator --
 * @return the iterator to previous node
 */
template <typename PNode>
typename node_iterator<PNode>::self_type node_iterator<PNode>::operator-- ()
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    m_iter = m_iter.predecessor();
    return *this;
}

/**
 * Operator --
 * @return the iterator to previous node
 */
template <typename PNode>
typename node_iterator<PNode>::self_type node_iterator<PNode>::operator-- (int )
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    pnode_type temp = m_iter;
    operator--();
    return self_type(temp);
}

/**
 * Operator ++
 * @return the iterator to next node
 */
template <typename PNode>
const typename node_iterator<PNode>::self_type node_iterator<PNode>::operator++ () const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    m_iter = m_iter.successor();
    return *this;
}

/**
 * Operator ++
 * @return the iterator to current node
 */
template <typename PNode>
const typename node_iterator<PNode>::self_type node_iterator<PNode>::operator++ (int ) const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    pnode_type temp = m_iter;
    operator++();
    return self_type(temp);
}

/**
 * Operator --
 * @return the iterator to previous node
 */
template <typename PNode>
const typename node_iterator<PNode>::self_type node_iterator<PNode>::operator-- () const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    m_iter = m_iter.predecessor();
    return *this;
}

/**
 * Operator --
 * @return the iterator to current node
 */
template <typename PNode>
const typename node_iterator<PNode>::self_type node_iterator<PNode>::operator-- (int ) const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    pnode_type temp = m_iter;
    operator--();
    return self_type(temp);
}

/**
 * Operator ==
 * @param iter another iterator
 * @return the result of the checking
 */
template <typename PNode>
bool node_iterator<PNode>::operator== (const self_type& iter) const
{
    return &(m_iter.table()) == &(iter.m_iter.table())
        && m_iter.pos() == iter.m_iter.pos();
}

/**
 * Operator !=
 * @param iter another iterator
 * @return the result of the checking
 */
template <typename PNode>
bool node_iterator<PNode>::operator!= (const self_type& iter) const
{
    return &(m_iter.table()) != &(iter.m_iter.table())
        || m_iter.pos() != iter.m_iter.pos();
}

/**
 * Operator ->
 * @return the pointer to the current node
 */
template <typename PNode>
typename node_iterator<PNode>::pnode_type* node_iterator<PNode>::operator-> ()
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    return &m_iter;
}

/**
 * Operator *
 * @return the reference to the current node
 */
template <typename PNode>
typename node_iterator<PNode>::pnode_type& node_iterator<PNode>::operator* ()
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    return m_iter;
}

/**
 * Operator ->
 * @return the pointer to the current node
 */
template <typename PNode>
const typename node_iterator<PNode>::pnode_type* node_iterator<PNode>::operator-> () const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    return &m_iter;
}

/**
 * Operator *
 * @return the reference to the current node
 */
template <typename PNode>
const typename node_iterator<PNode>::pnode_type& node_iterator<PNode>::operator* () const
{
    OUROBOROS_ASSERT(m_iter.pos() != NIL);
    return m_iter;
}

/**
 * Operator =
 * @param iter another iterator
 * @return reference to yourself
 */
template <typename PNode>
typename node_iterator<PNode>::self_type&
    node_iterator<PNode>::operator= (const self_type& iter)
{
    m_iter = iter.m_iter;
    return *this;
}

/**
 * Operator =
 * @param iter another iterator
 * @return reference to yourself
 */
template <typename PNode>
const typename node_iterator<PNode>::self_type&
    node_iterator<PNode>::operator= (const self_type& iter) const
{
    m_iter = iter.m_iter;
    return *this;
}

/**
 * Get the table of nodes
 * @return the table of nodes
 */
template <typename PNode>
inline typename node_iterator<PNode>::table_type&
    node_iterator<PNode>::table() const
{
    return m_iter.table();
}

//==============================================================================
//  node_reverseiterator
//==============================================================================
/**
 * Get a iterator to previous node
 * @attention if the iterator to the end then get last node
 * @param iter the iterator
 * @return the iterator to previous node
 */
//static
template <typename Iterator>
typename node_reverseiterator<Iterator>::iterator
    node_reverseiterator<Iterator>::pred_node(const iterator& iter)
{
    if (iter != iterator(pnode_type(iter.table())))
    {
        return --iter;
    }
    else
    {
        const pos_type pos = iter.table().back_pos();
        return NIL == pos ?
            iterator(pnode_type(iter.table())) :
            iterator(pnode_type(iter.table(), pos).root().maximum());
    }
}

/**
 * Constructor
 * @param table the table of nodes
 */
template <typename Iterator>
node_reverseiterator<Iterator>::node_reverseiterator(table_type& table) :
    m_current(table)
{
}

/**
 * Constructor
 * @param pnode the pointer to the node
 */
template <typename Iterator>
node_reverseiterator<Iterator>::node_reverseiterator(const pnode_type& pnode) :
    m_current(pnode)
{
}

/**
 * Constructor
 * @param iter another reverse iterator
 */
template <typename Iterator>
node_reverseiterator<Iterator>::node_reverseiterator(const self_type& iter) :
    m_current(iter.m_current)
{
}

/**
 * Constructor
 * @param iter another iterator
 */
template <typename Iterator>
node_reverseiterator<Iterator>::node_reverseiterator(const iterator& iter) :
    m_current(self_type::pred_node(iter))
{
}

/**
 * Get the normal iterator
 * @return the normal iterator
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::iterator
    node_reverseiterator<Iterator>::base() const
{
    iterator temp = m_current;
    return ++temp;
}

/**
 * Operator ++
 * @return the reverse iterator to next node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator++ ()
{
    --m_current;
    return *this;
}

/**
 * Operator ++
 * @return the reverse iterator to current node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator++ (int )
{
    self_type temp = *this;
    --m_current;
    return temp;
}

/**
 * Operator --
 * @return the reverse iterator to previous node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator-- ()
{
    ++m_current;
    return *this;
}

/**
 * Operator --
 * @return the reverse iterator to current node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator-- (int )
{
    self_type temp = *this;
    ++m_current;
    return temp;
}

/**
 * Operator ++
 * @return the reverse iterator to next node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator++ () const
{
    return iterator::operator--();
}

/**
 * Operator ++
 * @return the reverse iterator to current node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator++ (int ) const
{
    return iterator::operator--(1);
}

/**
 * Operator --
 * @return the reverse iterator to previous node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator-- () const
{
   return iterator::operator++();
}

/**
 * Operator --
 * @return the reverse iterator to current node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type
    node_reverseiterator<Iterator>::operator-- (int ) const
{
    return iterator::operator++(1);
}

/**
 * Operator ==
 * @param iter another reverse iterator
 * @return the result of the checking
 */
template <typename Iterator>
bool node_reverseiterator<Iterator>::operator== (const self_type& iter) const
{
    return m_current == iter.m_current;
}

/**
 * Operator !=
 * @param iter another reverse iterator
 * @return the result of the checking
 */
template <typename Iterator>
bool node_reverseiterator<Iterator>::operator!= (const self_type& iter) const
{
    return m_current != iter.m_current;
}

/**
 * Operator ->
 * @return the pointer to the current node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::pnode_type*
    node_reverseiterator<Iterator>::operator-> ()
{
    return &(operator*());
}

/**
 * Operator *
 * @return the reference to the current node
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::pnode_type&
    node_reverseiterator<Iterator>::operator* ()
{
    return *m_current;
}

/**
 * Operator ->
 * @return the pointer to the current node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::pnode_type*
    node_reverseiterator<Iterator>::operator-> () const
{
    return &(operator*());
}

/**
 * Operator *
 * @return the reference to the current node
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::pnode_type&
    node_reverseiterator<Iterator>::operator* () const
{
    return *m_current;
}

/**
 * Operator =
 * @param iter another reverse iterator
 * @return reference to yourself
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type&
    node_reverseiterator<Iterator>::operator= (const self_type& iter)
{
    m_current = iter.m_current;
    return *this;
}

/**
 * Operator =
 * @param iter another reverse iterator
 * @return reference to yourself
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type&
    node_reverseiterator<Iterator>::operator= (const self_type& iter) const
{
    m_current = iter.m_current;
    return *this;
}

/**
 * Operator =
 * @param iter another iterator
 * @return reference to yourself
 */
template <typename Iterator>
typename node_reverseiterator<Iterator>::self_type&
    node_reverseiterator<Iterator>::operator= (const iterator& iter)
{
    m_current = pred_node(iter);
    return *this;
}

/**
 * Operator =
 * @param iter another iterator
 * @return reference to yourself
 */
template <typename Iterator>
const typename node_reverseiterator<Iterator>::self_type&
    node_reverseiterator<Iterator>::operator= (const iterator& iter) const
{
    m_current = pred_node(iter);
    return *this;
}

/**
 * Get the table of nodes
 * @return the table of nodes
 */
template <typename Iterator>
inline typename node_reverseiterator<Iterator>::table_type&
    node_reverseiterator<Iterator>::table() const
{
    return m_current.table();
}

inline std::ostream& operator << (std::ostream& s, const node_color color)
{
    s << (RED == color ? "r" : "b");
    return s;
}

template <typename Key, typename Body, typename Converter>
std::ostream& operator << (std::ostream& s, const data_node<Key, Body, Converter>& node)
{
    s   << "p=" << node.parent()
        << " l=" << node.left()
        << " r=" << node.right()
        << " c=" << node.color()
        << " b=" << node.body();
    return s;
}

template <typename Node, typename Table, typename Extractor>
std::ostream& operator << (std::ostream& s, const table_pnode<Node, Table, Extractor>& pnode)
{
    s << "pos=" << pnode.pos() << " " << pnode.get();
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_NODE_H */
