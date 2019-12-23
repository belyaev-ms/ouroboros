/**
 * @file   container.h
 * The kinds of containers
 */

#ifndef OUROBOROS_CONTAINER_H
#define	OUROBOROS_CONTAINER_H

#include <map>
#include <string>
#include <vector>
#include "ouroboros/error.h"

namespace ouroboros
{

/** the array is kind of vector */
template <typename T> struct vector { typedef std::vector<T> vector_type; };

/**
 * The base interface class for a map container
 */
template <typename Key, typename Field>
class base_map
{
public:
    inline static void update(void *ptr); ///< update data of the container
    inline static size_type size(const std::string& name, const count_type count); ///< get the size of the container
};

/**
 * The interface class for a local map container
 */
template <typename Key, typename Field>
class local_map : public base_map<Key, Field>
{
public:
    typedef std::map<Key, Field> container_type;

    inline static container_type* construct(const std::string& name, const size_type size); ///< construct the container
    inline static void destruct(container_type *ptr); ///< destruct the container
    inline static container_type* get(const std::string& name, const size_type size); ///< get the pointer to the container
};

/**
 * The interface class for a local multimap container
 */
template <typename Key, typename Field>
class local_multimap : public base_map<Key, Field>
{
public:
    typedef std::multimap<Key, Field> container_type;

    inline static container_type* construct(const std::string& name, const size_type size); ///< construct the container
    inline static void destruct(container_type *ptr); ///< destruct the container
    inline static container_type* get(const std::string& name, const size_type size ); ///< get the pointer to the container
};

/**
 * The interface class adapter for a map container
 */
template <typename Key, typename Field, template <typename, typename> class Interface = local_map>
class map
{
    typedef Interface<Key, Field> interface_type;
public:
    typedef typename interface_type::container_type container_type;
    typedef container_type* pointer;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::value_type value_type;

    inline map(const std::string& name);
    inline map(const std::string& name, const count_type count);
    inline ~map();
    inline container_type& operator() ();
    inline const container_type& operator() () const;
    inline pointer operator-> ();
    inline const pointer operator-> () const;
    inline void update() const;
private:
    const std::string m_name; ///< the name of the container
    const size_type m_size; ///< the limit of size of the container
    pointer m_pcontainer; ///< the pointer to the container
};

/**
 * The interface class adapter for a local map container
 */
template <typename Key, typename Field>
class map<Key, Field, local_map>
{
    typedef local_map<Key, Field> interface_type;
public:
    typedef typename interface_type::container_type container_type;
    typedef container_type* pointer;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::value_type value_type;

    inline map(const std::string& name);
    inline map(const std::string& name, const count_type count);
    inline container_type& operator() ();
    inline const container_type& operator() () const;
    inline pointer operator-> ();
    inline const pointer operator-> () const;
    inline void update() const;
private:
    container_type m_container;
};

/**
 * The interface class adapter for a local multimap container
 */
template <typename Key, typename Field>
class map<Key, Field, local_multimap>
{
    typedef local_map<Key, Field> interface_type;
public:
    typedef typename interface_type::container_type container_type;
    typedef container_type* pointer;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::value_type value_type;

    inline map(const std::string& name);
    inline map(const std::string& name, const count_type count);
    inline container_type& operator() ();
    inline const container_type& operator() () const;
    inline pointer operator-> ();
    inline const pointer operator-> () const;
    inline void update() const;
private:
    container_type m_container;
};

//==============================================================================
//  base_map
//==============================================================================
/**
 * Get the size of the container
 * @param name the name of the container
 * @param count maximum value of the number of items in the container
 * @return the size of the container
 */
//static
template <typename Key, typename Field>
inline size_type base_map<Key, Field>::size(const std::string& name, const count_type count)
{
    return 0;
}

/**
 * Update data of the container
 * @param ptr the pointer to the container
 */
//static
template <typename Key, typename Field>
inline void base_map<Key, Field>::update(void *ptr)
{
}

//==============================================================================
//  local_map
//==============================================================================
/**
 * Construct the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename local_map<Key, Field>::container_type*
    local_map<Key, Field>::construct(const std::string& name, const size_type size)
{
    return new container_type();
}

/**
 * Destruct the container
 * @param ptr the pointer to the container
 */
//static
template <typename Key, typename Field>
inline void local_map<Key, Field>::destruct(container_type *ptr)
{
    delete ptr;
}

/**
 * Get the pointer to the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename local_map<Key, Field>::container_type*
    local_map<Key, Field>::get(const std::string& name, const size_type size)
{
    OUROBOROS_THROW_BUG("method not supported");
    return NULL;
}

//==============================================================================
//  local_multimap
//==============================================================================
/**
 * Construct the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename local_multimap<Key, Field>::container_type*
    local_multimap<Key, Field>::construct(const std::string& name, const size_type size)
{
    return new container_type();
}

/**
 * Destruct the container
 * @param ptr the pointer to the container
 */
//static
template <typename Key, typename Field>
inline void local_multimap<Key, Field>::destruct(container_type *ptr)
{
    delete ptr;
}

/**
 * Get the pointer to the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename local_multimap<Key, Field>::container_type*
    local_multimap<Key, Field>::get(const std::string& name, const size_type size)
{
    OUROBOROS_THROW_BUG("method not supported");
    return NULL;
}

//==============================================================================
//  map
//==============================================================================
/**
 * Constructor
 * @param name the name of the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline map<Key, Field, Interface>::map(const std::string& name) :
    m_name(name),
    m_size(1024 * 1024),
    m_pcontainer(interface_type::construct(name, m_size))
{
}

/**
 * Constructor
 * @param name the name of the container
 * @param count the maximum value of the number of items in the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline map<Key, Field, Interface>::map(const std::string& name, const count_type count) :
    m_name(name),
    m_size(interface_type::size(name, count)),
    m_pcontainer(interface_type::construct(name, m_size))
{
}

/**
 * Destructor
 */
//virtual
template <typename Key, typename Field, template <typename, typename> class Interface>
map<Key, Field, Interface>::~map()
{
    interface_type::destruct(m_pcontainer);
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline typename map<Key, Field, Interface>::container_type&
    map<Key, Field, Interface>::operator() ()
{
    return m_pcontainer != NULL ? *m_pcontainer : *interface_type::get(m_name.c_str(), m_size);
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline const typename map<Key, Field, Interface>::container_type&
    map<Key, Field, Interface>::operator() () const
{
    return m_pcontainer != NULL ? *m_pcontainer : *interface_type::get(m_name.c_str(), m_size);
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline typename map<Key, Field, Interface>::pointer
    map<Key, Field, Interface>::operator-> ()
{
    return m_pcontainer != NULL ? m_pcontainer : interface_type::get(m_name.c_str(), m_size);
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline const typename map<Key, Field, Interface>::pointer
    map<Key, Field, Interface>::operator-> () const
{
    return m_pcontainer != NULL ? m_pcontainer : interface_type::get(m_name.c_str(), m_size);
}

/**
 * Update data of the container
 */
template <typename Key, typename Field, template <typename, typename> class Interface>
inline void map<Key, Field, Interface>::update() const
{
    interface_type::update(m_pcontainer != NULL ? m_pcontainer : interface_type::get(m_name.c_str(), m_size));
}

//==============================================================================
//  map<Key, Field, local_map>
//==============================================================================
/**
 * Constructor
 * @param name the name of the container
 */
template <typename Key, typename Field>
inline map<Key, Field, local_map>::map(const std::string& name)
{
}

/**
 * Constructor
 * @param name the name of the container
 * @param count the maximum value of the number of items in the container
 */
template <typename Key, typename Field>
inline map<Key, Field, local_map>::map(const std::string& name, const count_type count)
{
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field>
inline typename map<Key, Field, local_map>::container_type&
    map<Key, Field, local_map>::operator() ()
{
    return m_container;
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field>
inline const typename map<Key, Field, local_map>::container_type&
    map<Key, Field, local_map>::operator() () const
{
    return m_container;
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field>
inline typename map<Key, Field, local_map>::pointer
    map<Key, Field, local_map>::operator-> ()
{
    return &m_container;
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field>
inline const typename map<Key, Field, local_map>::pointer
    map<Key, Field, local_map>::operator-> () const
{
    return &m_container;
}

/**
 * Update data of the container
 */
template <typename Key, typename Field>
inline void map<Key, Field, local_map>::update() const
{
}

//==============================================================================
//  map<Key, Field, local_multimap>
//==============================================================================
/**
 * Constructor
 * @param name the name of the container
 */
template <typename Key, typename Field>
inline map<Key, Field, local_multimap>::map(const std::string& name)
{
}

/**
 * Constructor
 * @param name the name of the container
 * @param count the maximum value of the number of items in the container
 */
template <typename Key, typename Field>
inline map<Key, Field, local_multimap>::map(const std::string& name, const count_type count)
{
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field>
inline typename map<Key, Field, local_multimap>::container_type&
    map<Key, Field, local_multimap>::operator() ()
{
    return m_container;
}

/**
 * Get the reference to the container
 * @return the reference to the container
 */
template <typename Key, typename Field>
inline const typename map<Key, Field, local_multimap>::container_type&
    map<Key, Field, local_multimap>::operator() () const
{
    return m_container;
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field>
inline typename map<Key, Field, local_multimap>::pointer
    map<Key, Field, local_multimap>::operator-> ()
{
    return &m_container;
}

/**
 * Get the pointer to the container
 * @return the pointer to the container
 */
template <typename Key, typename Field>
inline const typename map<Key, Field, local_multimap>::pointer
    map<Key, Field, local_multimap>::operator-> () const
{
    return &m_container;
}

/**
 * Update data of the container
 */
template <typename Key, typename Field>
inline void map<Key, Field, local_multimap>::update() const
{
}

}   //namespace ouroboros

#endif	/* OUROBOROS_CONTAINER_H */

