/**
 * @file   rbtree.h
 * Red-black tree integrated into a table
 */

#ifndef OUROBOROS_RBTREE_H
#define	OUROBOROS_RBTREE_H

#include "ouroboros/node.h"

namespace ouroboros
{

/**
 * The helper for rbtree
 */
class rbtree_helper
{
public:
    typedef void (*callback_func)(void*, const pos_type, const pos_type);
    rbtree_helper() :
        m_object(NULL),
        m_callback(NULL)
    {}
    rbtree_helper(void *object, callback_func callback) :
        m_object(object),
        m_callback(callback)
    {}
    void operator() (const pos_type source, const pos_type dest) const
    {
        if (m_callback != NULL)
        {
            m_callback(m_object, source, dest);
        }
    }
private:
    void *m_object;
    callback_func m_callback;
};

/**
 * The interface class table adapter for a rbtree
 */
template <typename Table>
class rbtree_table_adapter : public Table
{
    typedef Table base_class;
public:
    typedef typename base_class::source_type source_type;
    typedef typename base_class::skey_type skey_type;
    typedef typename base_class::guard_type guard_type;
    typedef void (*callback_before_del_func)(const pos_type pos);
    typedef void (*callback_before_move_func)(const pos_type source, const pos_type dest);
    rbtree_table_adapter(source_type& source, skey_type& key) :
        base_class(source, key)
    {
    }
    rbtree_table_adapter(source_type& source, skey_type& key, const guard_type& guard) :
        base_class(source, key, guard)
    {
    }
    void set_helper(const rbtree_helper *helper)
    {
        m_helper = helper != NULL ? *helper : rbtree_helper();
    }
protected:
    virtual void do_before_move(const pos_type source, const pos_type dest)
    {
        m_helper(source, dest);
    }
private:
    rbtree_helper m_helper;
};

/**
 * Red-black tree
 */
template <typename PNode>
class rbtree
{
    template <template <typename, typename, typename> class, typename, typename, typename>
    friend class tree_data_table;
public:
    typedef PNode pnode_type;
    typedef typename pnode_type::key_type key_type;
    typedef typename pnode_type::body_type body_type;
    typedef typename pnode_type::table_type table_type;
    typedef typename pnode_type::node_type node_type;
    typedef typename node_type::converter converter;
    typedef typename pnode_type::static_pnode static_pnode;
    typedef node_iterator<static_pnode> iterator;
    typedef const iterator const_iterator;
    typedef node_reverseiterator<iterator> reverse_iterator;
    typedef const reverse_iterator const_reverse_iterator;

    rbtree(table_type& table, const pos_type root);
    virtual ~rbtree();

    iterator begin() const; ///< get the iterator to the begin of the tree
    iterator end() const; ///< get the iterator to the end of the tree
    reverse_iterator rbegin() const; ///< get the reverse iterator to the begin of the tree
    reverse_iterator rend() const; ///< get the reverse iterator to the end of the tree
    iterator root() const; ///< get the iterator to the root of the tree

    bool empty() const; ///< check the tree is empty
    size_type size() const; ///< get the size of the tree

    virtual iterator maximum() const; ///< get the iterator to the node that has a maximum key
    virtual iterator minimum() const; ///< get the iterator to the node that has a minimum key
    iterator lower_bound(const key_type& key) const; ///< get the iterator to first node that has a key is not less the key
    iterator lower_bound(const_iterator& xi, const_iterator& yi, const key_type& key) const; ///< get the iterator in the range [xi, yi) to first node that has a key is not less the key
    iterator upper_bound(const key_type& key) const; ///< get the iterator to first node that has a key is not greater the key
    iterator upper_bound(const_iterator& xi, const_iterator& yi, const key_type& key) const; ///< get the iterator in the range [xi, yi) to first node that has a key is not greater the key
    virtual iterator find(const key_type& key) const; ///< find a node by the key
    virtual void clear(); ///< clear the tree
    iterator insert(const body_type& value); ///< insert new node into the tree
    void erase(const key_type key); ///< erase a node by the key
    void erase(iterator& iter); ///< erase a node by the iterator
    void replace(iterator& iter, const body_type& value); ///< replace the node
#ifdef OUROBOROS_TEST_TOOLS_ENABLED
    void test() const; ///< test the structure of the tree
#endif
protected:
    inline table_type& table() const; ///< get the table of nodes
    inline typename table_type::unsafe_table& raw_table() const; ///< get the table of nodes (for the operations without lock)
    inline pnode_type construct(const node_type& node); ///< make the pointer to the node
    virtual pnode_type do_insert(pnode_type z); ///< insert new node
    pnode_type do_insert(pnode_type z, pnode_type y); ///< insert new node
    virtual pnode_type remove(iterator& iter); ///< remove the node
    inline void destruct(const pnode_type& pnode); ///< remove the pointer to the node
    virtual void move(const pnode_type& node, const pos_type pos); ///< move node to the new position
    inline void set_root(const pos_type root); ///< set position of the root of the tree
    inline pos_type get_root() const; ///< get position of the root of the tree
private:
    rbtree& operator= (const rbtree& o);

    void left_rotate(pnode_type x); ///< rotate the node to the left
    void right_rotate(pnode_type x); ///< rotate the node to the right
    void insert_fixup(pnode_type x); ///< fix up the balance of the tree after inserting the node
    void remove_fixup(pnode_type x); ///< fix up the balance of the tree after removing the node
    inline node_color get_node_color(pnode_type pnode) const;
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
    void verify() const;
    void verify_colors_for_each_node(pnode_type pnode, count_type& count) const;
    void verify_colors_for_relationship(pnode_type pnode) const;
    void verify_path(pnode_type pnode, count_type black_count, count_type& path_black_count) const;
    void verify_path(pnode_type pnode) const;
#endif
public:
    static void on_before_move(void *object, const pos_type source, const pos_type dest); ///< callback before moving a record
private:
    static_pnode m_root; ///< the root of the tree
};

/**
 * Fast red-black tree
 */
template <typename PNode>
class fast_rbtree : public rbtree<PNode>
{
    template <template <typename, typename, typename> class, typename, typename, typename>
    friend class tree_data_table;
    typedef rbtree<PNode> base_class;
public:
    typedef typename base_class::pnode_type pnode_type;
    typedef typename base_class::key_type key_type;
    typedef typename base_class::body_type body_type;
    typedef typename base_class::table_type table_type;
    typedef typename base_class::node_type node_type;
    typedef typename base_class::converter converter;
    typedef typename base_class::static_pnode static_pnode;
    typedef typename base_class::iterator iterator;
    typedef typename base_class::const_iterator const_iterator;
    typedef typename base_class::reverse_iterator reverse_iterator;
    typedef typename base_class::const_reverse_iterator const_reverse_iterator;

    fast_rbtree(table_type& table, const pos_type root);
    virtual ~fast_rbtree();

    virtual iterator maximum() const; ///< get the iterator to the node that has a maximum key
    virtual iterator minimum() const; ///< get the iterator to the node that has a minimum key
    virtual iterator find(const key_type& key) const; ///< find a node by the key
    virtual void clear(); ///< clear the tree
protected:
    virtual pnode_type do_insert(pnode_type z); ///< insert new node
    virtual pnode_type remove(iterator& iter); ///< remove the node
    virtual void move(const pnode_type& node, const pos_type pos); ///< move node to the new position
    inline void set_root(const pos_type root); ///< set the position of the root of the tree
private:
    static_pnode m_min; ///< the node that has a minimum key
    static_pnode m_max; ///< the node that has a maximum key
};

//==============================================================================
//  rbtree
//==============================================================================
/**
 * Callback before moving a record
 * @param object pointer to the tree
 * @param source the position of record to be moved
 * @param dest the position of record to be deleted
 */
//static
template <typename PNode>
void rbtree<PNode>::on_before_move(void *object, const pos_type source, const pos_type dest)
{
    rbtree<PNode> *tree = reinterpret_cast<rbtree<PNode>*>(object);
    pnode_type pnode(tree->table(), source);
    tree->move(pnode, dest);
}

/**
 * Constructor
 * @param table the table of nodes
 * @param root the position of the root
 */
template <typename PNode>
rbtree<PNode>::rbtree(table_type& table, const pos_type root) :
    m_root(table, root)
{
    const rbtree_helper helper(this, &on_before_move);
    table.set_helper(&helper);
}

/**
 * Destructor
 */
//virtual
template <typename PNode>
rbtree<PNode>::~rbtree()
{

}

/**
 * Get the iterator to the begin of the tree
 * @return the iterator to the begin of the tree
 */
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::begin() const
{
    return minimum();
}

/**
 * Get the iterator to the end of the tree
 * @return the iterator to the end of the tree
 */
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::end() const
{
    return iterator(table());
}

/**
 * Get the reverse iterator to the begin of the tree
 * @return the reverse iterator to the begin of the tree
 */
template <typename PNode>
typename rbtree<PNode>::reverse_iterator rbtree<PNode>::rbegin() const
{
    return end();
}

/**
 * Get the reverse iterator to the end of the tree
 * @return the reverse iterator to the end of the tree
 */
template <typename PNode>
typename rbtree<PNode>::reverse_iterator rbtree<PNode>::rend() const
{
    return reverse_iterator(table());
}

/**
 * Get the iterator to the root of the tree
 * @return the iterator to the root of the tree
 */
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::root() const
{
    return iterator(m_root);
}

/**
 * Check the tree is empty
 * @return the result of the checking
 */
template <typename PNode>
bool rbtree<PNode>::empty() const
{
    return m_root.pos() == NIL;
}

/**
 * Get the size of the tree
 * @return the size of the tree
 */
template <typename PNode>
size_type rbtree<PNode>::size() const
{
    return raw_table().count();
}

/**
 * Get the iterator to the node that has a maximum key
 * @return the iterator to the node that has a maximum key
 */
//virtual
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::maximum() const
{
    return empty() ? end() : iterator(m_root.maximum());
}

/**
 * Get the iterator to the node that has a minimum key
 * @return the iterator to the node that has a minimum key
 */
//virtual
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::minimum() const
{
    return empty() ? end() : iterator(m_root.minimum());
}

/**
 * Get the iterator to first node that has a key is not less the key
 * @param key the key
 * @return the iterator to first node that has a key is not less the key
 */
template <typename PNode>
typename rbtree<PNode>::iterator
    rbtree<PNode>::lower_bound(const key_type& key) const
{
    return lower_bound(root(), end(), key);
}

/**
 * Get the iterator in the range [xi, yi) to first node that has a key is not less the key
 * @param xi the iterator to the begin of the range
 * @param yi the iterator to the end of the range
 * @param key the key
 * @return the iterator in range [xi, yi) to first node that has a key is not less the key
 */
template <typename PNode>
typename rbtree<PNode>::iterator
    rbtree<PNode>::lower_bound(const_iterator& xi, const_iterator& yi, const key_type& key) const
{
    while (xi != end())
    {
        if (xi->key() < key)
        {
            xi = const_iterator(xi->right());
        }
        else
        {
            yi = xi;
            xi = const_iterator(xi->left());
        }
    }
    return yi;
}

/**
 * Get the iterator to first node that has a key is not greater the key
 * @param key the key
 * @return the iterator to first node that has a key is not greater the key
 */
template <typename PNode>
typename rbtree<PNode>::iterator
    rbtree<PNode>::upper_bound(const key_type& key) const
{
    return upper_bound(root(), end(), key);
}

/**
 * Get the iterator in the range [xi, yi) to first node that has a key is not greater the key
 * @param xi the iterator to the begin of the range
 * @param yi the iterator to the end of the range
 * @param key the key
 * @return the iterator in range [xi, yi) to first node that has a key is not greater the key
 */
template <typename PNode>
typename rbtree<PNode>::iterator
    rbtree<PNode>::upper_bound(const_iterator& xi, const_iterator& yi, const key_type& key) const
{
    while (xi != end())
    {
        if (xi->key() <= key)
        {
            xi = const_iterator(xi->right());
        }
        else
        {
            yi = xi;
            xi = const_iterator(xi->left());
        }
    }
    return yi;
}

/**
 * Find a node by the key
 * @param key the key
 * @return the iterator to the found key
 */
//virtual
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::find(const key_type& key) const
{
    pnode_type pnode = m_root;
    while (pnode.pos() != NIL)
    {
        key_type nkey = pnode.get().key();
        if (nkey == key)
        {
            return iterator(pnode);
        }
        pnode = key < nkey ? pnode.left() : pnode.right();
    }
    return end();
}

/**
 * Clear the tree
 */
//virtual
template <typename PNode>
void rbtree<PNode>::clear()
{
    raw_table().clear();
    m_root.pos(NIL);
}

/**
 * Insert new node into the tree
 * @param value the value of the node
 * @return the iterator to the node
 */
template <typename PNode>
typename rbtree<PNode>::iterator rbtree<PNode>::insert(const body_type& value)
{
    const node_type node(value, NIL, RED);
    pnode_type pnode(construct(node));
    return iterator(do_insert(pnode));
}

/**
 * Erase the node
 * @param key the key
 */
template <typename PNode>
void rbtree<PNode>::erase(const key_type key)
{
    iterator it = find(key);
    if (it != end())
    {
        erase(it);
    }
}

/**
 * Get the table of nodes
 * @return the table of nodes
 */
template <typename PNode>
inline typename rbtree<PNode>::table_type& rbtree<PNode>::table() const
{
    return m_root.table();
}

/**
 * Get the table of nodes (for the operations without lock)
 * @return the table of nodes (for the operations without lock)
 */
template <typename PNode>
inline typename rbtree<PNode>::table_type::unsafe_table& rbtree<PNode>::raw_table() const
{
    return static_cast<typename table_type::unsafe_table&>(table());
}

/**
 * Rotate the node to the left
 * @param x the pointer to the node
 */
template <typename PNode>
void rbtree<PNode>::left_rotate(pnode_type x)
{
    pnode_type y = x.right();

    x.right(y.left());
    if (y.pleft() != NIL)
    {
        y.left().parent(x);
    }

    if (y.pos() != NIL)
    {
        y.parent(x.parent());
    }
    if (x.pparent() == NIL)
    {
        m_root = y;
    }
    else
    {
        if (x.is_left_son())
        {
            x.parent().left(y);
        }
        else
        {
            x.parent().right(y);
        }
    }

    y.left(x);
    if (x.pos() != NIL)
    {
        x.parent(y);
    }
}

/**
 * Rotate the node to the right
 * @param x the pointer to the node
 */
template <typename PNode>
void rbtree<PNode>::right_rotate(pnode_type x)
{
    pnode_type y = x.left();

    x.left(y.right());
    if (y.pright() != NIL)
    {
        y.right().parent(x);
    }

    if (y.pos() != NIL)
    {
        y.parent(x.parent());
    }
    if (x.pparent() == NIL)
    {
        m_root = y;
    }
    else
    {
        if (x.is_right_son())
        {
            x.parent().right(y);
        }
        else
        {
            x.parent().left(y);
        }
    }

    y.right(x);
    if (x.pos() != NIL)
    {
        x.parent(y);
    }
}

/**
 * Insert new node
 * @param z the pointer to the node
 * @return the pointer to the node
 */
//virtual
template <typename PNode>
typename rbtree<PNode>::pnode_type rbtree<PNode>::do_insert(pnode_type z)
{
    pnode_type x = m_root;
    pnode_type y(table());
    while (x.pos() != NIL)
    {
        y = x;
        x = (z < x) ? x.left() : x.right();
    }
    return do_insert(z, y);
}

/**
 * Insert new node
 * @param z the pointer to the node
 * @param y the pointer to the parent node
 * @return the pointer to the node
 */
template <typename PNode>
inline typename rbtree<PNode>::pnode_type rbtree<PNode>::do_insert(pnode_type z, pnode_type y)
{
    z.parent(y);
    if (y.pos() == NIL)
    {
        m_root = z;
    }
    else
    {
        if (z < y)
        {
            y.left(z);
        }
        else
        {
            y.right(z);
        }
    }
    z.color(RED);
    insert_fixup(z);
#ifdef OUROBOROS_TEST_ENABLED
    verify();
#endif
    return z;
}

/**
 * Fix up the balance of the tree after inserting the node
 * @param x the pointer to the node
 */
template <typename PNode>
void rbtree<PNode>::insert_fixup(pnode_type x)
{
    while (x.pos() != m_root.pos() && x.parent().color() == RED)
    {
        if (x.parent().is_left_son())
        {
            pnode_type y = x.parent().parent().right();
            if (y.color() == RED)
            {
                x.parent().color(BLACK);
                y.color(BLACK);
                x.parent().parent().color(RED);
                x = x.parent().parent();
            }
            else
            {
                if (x.is_right_son())
                {
                    x = x.parent();
                    left_rotate(x);
                }
                x.parent().color(BLACK);
                x.parent().parent().color(RED);
                right_rotate(x.parent().parent());
            }
        }
        else
        {
            pnode_type y = x.parent().parent().left();
            if (y.color() == RED)
            {
                x.parent().color(BLACK);
                y.color(BLACK);
                x.parent().parent().color(RED);
                x = x.parent().parent();
            }
            else
            {
                if (x.is_left_son())
                {
                    x = x.parent();
                    right_rotate(x);
                }
                x.parent().color(BLACK);
                x.parent().parent().color(RED);
                left_rotate(x.parent().parent());
            }
        }
    }
    m_root.color(BLACK);
}

/**
 * Remove the node
 * @param iter the iterator to the node
 * @return the pointer to the node
 */
//virtual
template <typename PNode>
typename rbtree<PNode>::pnode_type rbtree<PNode>::remove(iterator& iter)
{
    pnode_type y(table());
    pnode_type z(*iter);

    y = (z.pleft() == NIL || z.pright() == NIL) ? z : z.successor();

    pnode_type x = (y.pleft() != NIL) ? y.left() : y.right();

    x.parent(y.parent());

    if (y.pparent() == NIL)
    {
        m_root = x;
    }
    else
    {
        if (y.is_left_son())
        {
            y.parent().left(x);
        }
        else
        {
            y.parent().right(x);
        }
    }

    z.reset();

    if (y.color() == BLACK)
    {
        remove_fixup(x);
    }

    if (y.pos() != z.pos())
    {
        if (m_root.pos() == z.pos())
        {
            m_root = y;
        }
        y.substitute(z);
    }

#ifdef OUROBOROS_TEST_ENABLED
    verify();
#endif
    return z;
}

/**
 * Erase the node
 * @param iter the iterator to the node
 */
template <typename PNode>
void rbtree<PNode>::erase(iterator& iter)
{
    destruct(remove(iter));
#ifdef OUROBOROS_TEST_ENABLED
    verify();
#endif
}

/**
 * Fix up the balance of the tree after removing the node
 * @param x the pointer to the node
 */
template <typename PNode>
void rbtree<PNode>::remove_fixup(pnode_type x)
{
    while (x.pos() != m_root.pos() && x.color() == BLACK)
    {
        if (x.is_left_son())
        {
            pnode_type w = x.parent().right();
            if (w.color() == RED)
            {
                w.color(BLACK);
                x.parent().color(RED);
                left_rotate(x.parent());
                w = x.parent().right();
            }
            if (w.left().color() == BLACK && w.right().color() == BLACK)
            {
                w.color(RED);
                x = x.parent();
            }
            else
            {
                if (w.right().color() == BLACK)
                {
                    w.left().color(BLACK);
                    w.color(RED);
                    right_rotate(w);
                    w = x.parent().right();
                }
                w.color(x.parent().color());
                x.parent().color(BLACK);
                w.right().color(BLACK);
                left_rotate(x.parent());
                x = m_root;
            }
        }
        else
        {
            pnode_type w = x.parent().left();
            if (w.color() == RED)
            {
                w.color(BLACK);
                x.parent().color(RED);
                right_rotate(x.parent());
                w = x.parent().left();
            }
            if (w.right().color() == BLACK && w.left().color() == BLACK)
            {
                w.color(RED);
                x = x.parent();
            }
            else
            {
                if (w.left().color() == BLACK)
                {
                    w.right().color(BLACK);
                    w.color(RED);
                    left_rotate(w);
                    w = x.parent().left();
                }
                w.color(x.parent().color());
                x.parent().color(BLACK);
                w.left().color(BLACK);
                right_rotate(x.parent());
                x = m_root;
            }
        }
    }
    x.color(BLACK);
}

/**
 * Get color of the node
 * @param pnode the pointer to the node
 * @return color of the node
 */
template <typename PNode>
inline node_color rbtree<PNode>::get_node_color(pnode_type pnode) const
{
    return pnode.pos() == NIL ? BLACK : pnode.color();
}

/**
 * Make the pointer to the node
 * @param node the node
 * @return the pointer to the node
 */
template <typename PNode>
inline typename rbtree<PNode>::pnode_type
    rbtree<PNode>::construct(const node_type& node)
{
    const typename table_type::record_type record(node);
    const pos_type beg = raw_table().beg_pos();
    const pos_type end = raw_table().end_pos();
    if (!raw_table().empty() && end == beg)
    {
        pnode_type pnode(table(), beg);
        iterator it(pnode);
        erase(it);
    }
    table().unsafe_add(record);
    return pnode_type(table(), end);
}

/**
 * Remove the pointer to the node
 * @param pnode the pointer to the node
 */
template <typename PNode>
inline void rbtree<PNode>::destruct(const pnode_type& pnode)
{
    raw_table().remove(pnode.pos());
}

/**
 * Move the node to the new position
 * @param pnode the pointer to the node
 * @param pos the new position
 */
//virtual
template <typename PNode>
void rbtree<PNode>::move(const pnode_type& pnode, const pos_type pos)
{
    if (pnode.pleft() != NIL)
    {
        pnode.left().pparent(pos);
    }
    if (pnode.pright() != NIL)
    {
        pnode.right().pparent(pos);
    }
    if (NIL == pnode.pparent())
    {
        m_root.pos(pos);
    }
    else if (pnode.is_left_son())
    {
        pnode.parent().pleft(pos);
    }
    else
    {
        pnode.parent().pright(pos);
    }
}

/**
 * Replace the node
 * @param iter the iterator to the node
 * @param value new value of the node
 */
template <typename PNode>
void rbtree<PNode>::replace(iterator& iter, const body_type& value)
{
    pnode_type pnode(remove(iter));
    const node_type node(value, NIL, RED);
    pnode.set(node);
    do_insert(pnode);
}

#ifdef OUROBOROS_TEST_TOOLS_ENABLED
/**
 * Test the structure of the tree
 */
template <typename PNode>
void rbtree<PNode>::test() const
{
    verify();
}
#endif

#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED)
/**
 * Verify the the tree
 */
template <typename PNode>
void rbtree<PNode>::verify() const
{
    OUROBOROS_ASSERT(get_node_color(m_root) == BLACK);
    count_type count = 0;
    verify_colors_for_each_node(m_root, count);
    verify_colors_for_relationship(m_root);
    verify_path(m_root);
}

/**
 * Verify colors for each node
 * @param pnode the root for the verification
 * @param count counter of nodes
 */
template <typename PNode>
void rbtree<PNode>::verify_colors_for_each_node(pnode_type pnode, count_type& count) const
{
    OUROBOROS_ASSERT(get_node_color(pnode) == RED || get_node_color(pnode) == BLACK);
    if (pnode.pos() != NIL)
    {
        ++count;
        OUROBOROS_ASSERT(count <= size());
        verify_colors_for_each_node(pnode.left(), count);
        verify_colors_for_each_node(pnode.right(), count);
        --count;
    }
}

/**
 * Verify colors for relationship
 * @param pnode the root for the verification
 */
template <typename PNode>
void rbtree<PNode>::verify_colors_for_relationship(pnode_type pnode) const
{
    if (get_node_color(pnode) == RED)
    {
        OUROBOROS_ASSERT(get_node_color(pnode.left()) == BLACK);
        OUROBOROS_ASSERT(get_node_color(pnode.right()) == BLACK);
        OUROBOROS_ASSERT(get_node_color(pnode.parent()) == BLACK);
    }
    if (pnode.pos() != NIL)
    {
        verify_colors_for_relationship(pnode.left());
        verify_colors_for_relationship(pnode.right());
    }
}

/**
 * Verify path for each node
 * @param pnode the root for the verification
 * @param black_count the count of black nodes in the path
 * @param path_black_count the count of black nodes that each path must have
 */
template <typename PNode>
void rbtree<PNode>::verify_path(pnode_type pnode, count_type black_count, count_type& path_black_count) const
{
    if (get_node_color(pnode) == BLACK)
    {
        ++black_count;
    }
    if (pnode.pos() != NIL)
    {
        verify_path(pnode.left(), black_count, path_black_count);
        verify_path(pnode.right(), black_count, path_black_count);
    }
    else
    {
        if (NIL == path_black_count)
        {
            path_black_count = black_count;
        }
        else
        {
            OUROBOROS_ASSERT(black_count == path_black_count);
        }
    }
}

/**
 * Verify path for each node
 * @param pnode the root for the verification
 */
template <typename PNode>
void rbtree<PNode>::verify_path(pnode_type pnode) const
{
    count_type path_black_count = NIL;
    verify_path(pnode, 0, path_black_count);
}
#endif

/**
 * Set position of the root of the tree
 * @param root the position of the root of the tree
 */
template <typename PNode>
inline void rbtree<PNode>::set_root(const pos_type root)
{
    m_root.pos(root);
}

/**
 * Get position of the root of the tree
 * @return the position of the root of the tree
 */
template <typename PNode>
inline pos_type rbtree<PNode>::get_root() const
{
    return m_root.pos();
}

//==============================================================================
//  fast_rbtree
//==============================================================================
/**
 * Constructor
 * @param table the table of nodes
 * @param root the position of the root
 */
template <typename PNode>
fast_rbtree<PNode>::fast_rbtree(table_type& table, const pos_type root) :
    base_class(table, root),
    m_min(table, NIL == root ? NIL : base_class::minimum()->pos()),
    m_max(table, NIL == root ? NIL : base_class::maximum()->pos())
{

}

/**
 * Destructor
 */
//virtual
template <typename PNode>
fast_rbtree<PNode>::~fast_rbtree()
{

}

/**
 * Get the iterator to the node that has a maximum key
 * @return the iterator to the node that has a maximum key
 */
//virtual
template <typename PNode>
typename fast_rbtree<PNode>::iterator fast_rbtree<PNode>::maximum() const
{
    return iterator(m_max);
}

/**
 * Get the iterator to the node that has a minimum key
 * @return the iterator to the node that has a minimum key
 */
//virtual
template <typename PNode>
typename fast_rbtree<PNode>::iterator fast_rbtree<PNode>::minimum() const
{
    return iterator(m_min);
}

/**
 * Find a node by the key
 * @param key the key
 * @return the iterator to the found key
 */
//virtual
template <typename PNode>
typename fast_rbtree<PNode>::iterator fast_rbtree<PNode>::find(const key_type& key) const
{
    return (base_class::empty() || key < m_min.get().key() || key > m_max.get().key()) ?
        base_class::end() :
        base_class::find(key);
}

/**
 * Clear the tree
 */
//virtual
template <typename PNode>
void fast_rbtree<PNode>::clear()
{
    base_class::clear();
    m_min.pos(NIL);
    m_max.pos(NIL);
}

/**
 * Insert new node
 * @param z the pointer to the node
 * @return the pointer to the node
 */
//virtual
template <typename PNode>
typename fast_rbtree<PNode>::pnode_type fast_rbtree<PNode>::do_insert(pnode_type z)
{
    const key_type key = z.get().key();
    if (base_class::empty())
    {
        m_min = z;
        m_max = z;
    }
    else if (m_max.pos() != NIL && key >= m_max.get().key())
    {
        m_max.pos(base_class::do_insert(z, m_max).pos());
        return m_max;
    }
    else if (m_min.pos() != NIL && key < m_min.get().key())
    {
        m_min.pos(base_class::do_insert(z, m_min).pos());
        return m_min;
    }
    return base_class::do_insert(z);
}

/**
 * Remove the node
 * @param iter the iterator to the node
 * @return the pointer to the node
 */
//virtual
template <typename PNode>
typename fast_rbtree<PNode>::pnode_type fast_rbtree<PNode>::remove(iterator& iter)
{
    if (m_min.pos() != NIL)
    {
        if (m_min.pos() == iter->pos())
        {
            m_min.pos(m_min.pright() != NIL ? m_min.pright() : m_min.pparent());
        }
    }
    if (m_max.pos() != NIL)
    {
        if (m_max.pos() == iter->pos())
        {
            m_max.pos(m_max.pleft() != NIL ? m_max.pleft() : m_max.pparent());
        }
    }
    return base_class::remove(iter);
}

/**
 * Move the node to the new position
 * @param pnode the pointer to the node
 * @param pos the new position
 */
//virtual
template <typename PNode>
void fast_rbtree<PNode>::move(const pnode_type& pnode, const pos_type pos)
{
    if (m_min.pos() == pnode.pos())
    {
        m_min.pos(pos);
    }
    if (m_max.pos() == pnode.pos())
    {
        m_max.pos(pos);
    }
    base_class::move(pnode, pos);
}

/**
 * Set position of the root of the tree
 * @param root the position of the root of the tree
 */
template <typename PNode>
inline void fast_rbtree<PNode>::set_root(const pos_type root)
{
    base_class::set_root(root);
    m_min.pos(NIL == root ? NIL : base_class::minimum()->pos());
    m_max.pos(NIL == root ? NIL : base_class::maximum()->pos());
}

}   //namespace ouroboros

#endif	/* OUROBOROS_RBTREE_H */

