/**
 * @file   hashmap.h
 * The class hash map
 */

#ifndef OUROBOROS_HASHMAP_H
#define	OUROBOROS_HASHMAP_H

#include <assert.h>
#include <utility>
#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The item of a hash_map container
 */
template <typename Key, typename Value>
class hash_item
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    explicit inline hash_item(const value_type& val);
    value_type value;
    hash_item *prev;
    hash_item *next;
private:
    hash_item();
};

template <typename Key, typename Value, int slotCount>
class hash_map;

/**
 * The item iterator of a hash_map container
 */
template <typename Map>
class hash_iterator
{
    template <typename Key, typename Value, int slotCount> friend class hash_map;
    typedef hash_iterator<Map> self_type;
    typedef Map map_type;
    typedef typename map_type::item_type item_type;
public:
    typedef typename map_type::key_type key_type;
    typedef typename map_type::mapped_type mapped_type;
    typedef typename map_type::value_type value_type;
    hash_iterator();
    explicit hash_iterator(const map_type *map);
    hash_iterator(const map_type *map, item_type *item);
    hash_iterator(const self_type& iter);
    self_type operator++ ();
    self_type operator++ (int );

    const self_type operator++ () const;
    const self_type operator++ (int ) const;

    const bool operator== (const self_type& iter) const;
    const bool operator!= (const self_type& iter) const;

    value_type* operator-> ();
    value_type& operator* ();
    const value_type* operator-> () const;
    const value_type& operator* () const;
private:
    const map_type *m_map;
    mutable item_type *m_item;
};

/**
 * The hash_map container
 */
template <typename Key, typename Value, int slotCount>
class hash_map
{
    typedef hash_map<Key, Value, slotCount> self_type;
    friend class hash_iterator<self_type>;
    typedef hash_item<Key, Value> item_type;
public:
    enum
    {
        SLOT_COUNT = slotCount
    };
    typedef hash_iterator<self_type> iterator;
    typedef const iterator const_iterator;
    typedef typename item_type::key_type key_type;
    typedef typename item_type::mapped_type mapped_type;
    typedef typename item_type::value_type value_type;

    hash_map();
    ~hash_map();

    iterator find(const key_type& key) const;
    void insert(const value_type& value);
    void erase(const key_type& key);
    void erase(const_iterator& it);
    void clear();

    const size_t size() const;
    const bool empty() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
private:
    mutable item_type *m_items[SLOT_COUNT];
    size_t m_count;
};

//==============================================================================
//  hash_item
//==============================================================================
/**
 * Constructor
 * @param val the value of the item
 */
template <typename Key, typename Value>
inline hash_item<Key, Value>::hash_item(const value_type& val) :
    value(val),
    prev(NULL),
    next(NULL)
{
}

//==============================================================================
//  hash_iterator
//==============================================================================
/**
 * Constructor
 */
template <typename Map>
hash_iterator<Map>::hash_iterator() :
    m_map(NULL),
    m_item(NULL)
{
}

/**
 * Constructor
 * @param map the hash_map container
 */
template <typename Map>
hash_iterator<Map>::hash_iterator(const map_type *map) :
    m_map(map),
    m_item(NULL)
{
}

/**
 * Constructor
 * @param map the hash_map container
 * @param item the item of the container
 */
template <typename Map>
hash_iterator<Map>::hash_iterator(const map_type *map, item_type *item) :
    m_map(map),
    m_item(item)
{
}

/**
 * Constructor
 * @param iter the copied iterator
 */
template <typename Map>
hash_iterator<Map>::hash_iterator(const self_type& iter) :
    m_map(iter.m_map),
    m_item(iter.m_item)
{
}

/**
 * Increment operator
 * @return the iterator on the current item
 */
template <typename Map>
typename hash_iterator<Map>::self_type hash_iterator<Map>::operator++ ()
{
    if (m_item->next != NULL)
    {
        m_item = m_item->next;
    }
    else
    {
        size_t i = m_item->value.first % map_type::SLOT_COUNT + 1;
        if (i >= map_type::SLOT_COUNT)
        {
            m_item = NULL;
        }
        else
        {
            while (i < map_type::SLOT_COUNT)
            {
                m_item = m_map->m_items[i++];
                if (m_item != NULL)
                {
                    break;
                }
            }
        }
    }
    return *this;
}

/**
 * Increment operator
 * @return the iterator on the previous item
 */
template <typename Map>
typename hash_iterator<Map>::self_type hash_iterator<Map>::operator++ (int )
{
    self_type temp(*this);
    operator++();
    return self_type(temp);
}

/**
 * Increment operator
 * @return the iterator on the current item
 */
template <typename Map>
const typename hash_iterator<Map>::self_type hash_iterator<Map>::operator++ () const
{
    if (m_item->next != NULL)
    {
        m_item = m_item->next;
    }
    else
    {
        size_t i = m_item->value.first % map_type::SLOT_COUNT + 1;
        if (i >= map_type::SLOT_COUNT)
        {
            m_item = NULL;
        }
        else
        {
            while (i < map_type::SLOT_COUNT)
            {
                m_item = m_map->m_items[i++];
                if (m_item != NULL)
                {
                    break;
                }
            }
        }
    }
    return *this;
}

/**
 * Increment operator
 * @return the iterator on the previous item
 */
template <typename Map>
const typename hash_iterator<Map>::self_type hash_iterator<Map>::operator++ (int ) const
{
    self_type temp(*this);
    operator++();
    return self_type(temp);
}

/**
 * Operator ==
 * @param iter the iterator for comparing
 * @return the result of the comparing
 */
template <typename Map>
const bool hash_iterator<Map>::operator== (const self_type& iter) const
{
    return (m_map == iter.m_map) && (m_item == iter.m_item);
}

/**
 * Operator !=
 * @param iter the iterator for comparing
 * @return the result of the comparing
 */
template <typename Map>
const bool hash_iterator<Map>::operator!= (const self_type& iter) const
{
    return (m_map != iter.m_map) || (m_item != iter.m_item);
}

/**
 * Operator->
 * @return pointer to yourself
 */
template <typename Map>
typename hash_iterator<Map>::value_type* hash_iterator<Map>::operator-> ()
{
    return &(m_item->value);
}

/**
 * Operator*
 * @return reference to yourself
 */
template <typename Map>
typename hash_iterator<Map>::value_type& hash_iterator<Map>::operator* ()
{
    return m_item->value;
}

/**
 * Operator->
 * @return pointer to yourself
 */
template <typename Map>
const typename hash_iterator<Map>::value_type* hash_iterator<Map>::operator-> () const
{
    return &(m_item->value);
}

/**
 * Operator*
 * @return reference to yourself
 */
template <typename Map>
const typename hash_iterator<Map>::value_type& hash_iterator<Map>::operator* () const
{
    return m_item->value;
}

//==============================================================================
//  hash_map
//==============================================================================
/**
 * Constructor
 */
template <typename Key, typename Value, int slotCount>
hash_map<Key, Value, slotCount>::hash_map() :
    m_count(0)
{
    for (size_t i = 0; i < SLOT_COUNT; ++i)
    {
        m_items[i] = NULL;
    }
}

/**
 * Destructor
 */
template <typename Key, typename Value, int slotCount>
hash_map<Key, Value, slotCount>::~hash_map()
{
    clear();
}

/**
 * Find an item of the container by the key
 * @param key the key
 * @return the iterator on the found item
 */
template <typename Key, typename Value, int slotCount>
typename hash_map<Key, Value, slotCount>::iterator
    hash_map<Key, Value, slotCount>::find(const key_type& key) const
{
    const key_type id = key % SLOT_COUNT;
    item_type *item = m_items[id];
    while (item != NULL)
    {
        if (key == item->value.first)
        {
            break;
        }
        item = item->next;
    }
    return iterator(this, item);
}

/**
 * Insert the item to the container
 * @param value the key and the value
 */
template <typename Key, typename Value, int slotCount>
void hash_map<Key, Value, slotCount>::insert(const value_type& value)
{
    const key_type id = value.first % SLOT_COUNT;
    item_type *item = m_items[id];
    item_type *prev = NULL;
    while (item != NULL)
    {
        if (value.first == item->value.first)
        {
            return;
        }
        prev = item;
        item = item->next;
    }
    item = new item_type(value);
    item->prev = prev;
    if (prev != NULL)
    {
        prev->next = item;
    }
    else
    {
        m_items[id] = item;
    }
    ++m_count;
}

/**
 * Erase an item by the key
 * @param key the key
 */
template <typename Key, typename Value, int slotCount>
void hash_map<Key, Value, slotCount>::erase(const key_type& key)
{
    erase(find(key));
}

/**
 * Erase an item
 * @param it the iterator on the item
 */
template <typename Key, typename Value, int slotCount>
void hash_map<Key, Value, slotCount>::erase(const_iterator& it)
{
    if (it.m_item != NULL)
    {
        if (it.m_item->prev != NULL)
        {
            it.m_item->prev->next = it.m_item->next;
        }
        else
        {
            m_items[it->first % SLOT_COUNT] = it.m_item->next;
        }
        if (it.m_item->next != NULL)
        {
            it.m_item->next->prev = it.m_item->prev;
        }
        delete it.m_item;
        assert(m_count > 0);
        --m_count;
    }
}

/**
 * Clear the container
 */
template <typename Key, typename Value, int slotCount>
void hash_map<Key, Value, slotCount>::clear()
{
    for (size_t i = 0; i < SLOT_COUNT; ++i)
    {
        const item_type *item = m_items[i];
        while (item != NULL)
        {
            const item_type *next = item->next;
            delete item;
            item = next;
        }
        m_items[i] = NULL;
    }
    m_count = 0;
}

/**
 * Get the count of items in the container
 * @return the count of items
 */
template <typename Key, typename Value, int slotCount>
const size_t hash_map<Key, Value, slotCount>::size() const
{
    return m_count;
}

/**
 * Check the table is empty
 * @return the result of the checking
 */
template <typename Key, typename Value, int slotCount>
const bool hash_map<Key, Value, slotCount>::empty() const
{
    return 0 == m_count;
}

/**
 * Get the iterator on the first item
 * @return the iterator on the first item
 */
template <typename Key, typename Value, int slotCount>
typename hash_map<Key, Value, slotCount>::iterator
    hash_map<Key, Value, slotCount>::begin()
{
    for (size_t i = 0; i < SLOT_COUNT; ++i)
    {
        item_type *item = m_items[i];
        if (item != NULL)
        {
            return iterator(this, item);
        }
    }

    return end();
}

/**
 * Get the iterator on the end of the container
 * @return the iterator on the end of the container
 */
template <typename Key, typename Value, int slotCount>
typename hash_map<Key, Value, slotCount>::iterator
    hash_map<Key, Value, slotCount>::end()
{
    return iterator(this);
}

/**
 * Get the iterator on the first item
 * @return the iterator on the first item
 */
template <typename Key, typename Value, int slotCount>
typename hash_map<Key, Value, slotCount>::const_iterator
    hash_map<Key, Value, slotCount>::begin() const
{
    for (size_t i = 0; i < SLOT_COUNT; ++i)
    {
        item_type *item = m_items[i];
        if (item != NULL)
        {
            return iterator(this, item);
        }
    }

    return end();
}

/**
 * Get the iterator on the end of the container
 * @return the iterator on the end of the container
 */
template <typename Key, typename Value, int slotCount>
typename hash_map<Key, Value, slotCount>::const_iterator
    hash_map<Key, Value, slotCount>::end() const
{
    return const_iterator(this);
}

}   //namespace ouroboros

#endif	/* OUROBOROS_HASHMAP_H */

