/**
 * @file   sharedcontainer.h
 * The kinds of containers that is placed in shared memory
 */

#ifndef OUROBOROS_SHAREDCONTAINER_H
#define	OUROBOROS_SHAREDCONTAINER_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use sharedcontainer (OUROBOROS_BOOST_ENABLED)
#endif

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/filesystem/operations.hpp>
#include <functional>
#include <utility>

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The interface class for a shared map container
 */
template <typename Key, typename Field>
class shared_map : public base_map<Key, Field>
{
    typedef std::pair<const Key, Field> value_type;
    typedef boost::interprocess::allocator<value_type, boost::interprocess::managed_shared_memory::segment_manager> allocator;
public:
    typedef boost::interprocess::map<Key, Field, std::less<Key>, allocator> container_type;

    inline static container_type* construct(const std::string& name, const size_type size); ///< construct the container
    inline static void destruct(container_type *ptr); ///< destruct the container
    inline static container_type* get(const std::string& name, const size_type size); ///< get the pointer to the container
};

/**
 * The interface class for a shared multimap container
 */
template <typename Key, typename Field>
class shared_multimap : public base_map<Key, Field>
{
    typedef std::pair<const Key, Field> value_type;
    typedef boost::interprocess::allocator<value_type, boost::interprocess::managed_shared_memory::segment_manager> allocator;
public:
    typedef boost::interprocess::multimap<Key, Field, std::less<Key>, allocator> container_type;

    inline static container_type* construct(const std::string& name, const size_type size); ///< construct the container
    inline static void destruct(container_type *ptr); ///< destruct the container
    inline static container_type* get(const std::string& name, const size_type size); ///< get the pointer to the container
};

//==============================================================================
//  shared_map
//==============================================================================
/**
 * Construct the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename shared_map<Key, Field>::container_type*
    shared_map<Key, Field>::construct(const std::string& name, const size_type size)
{
    boost::interprocess::managed_shared_memory& mem = shared_memory::instance().mem();
    static allocator alloc(mem.get_segment_manager());
    return mem.find_or_construct<container_type>(name.c_str())(std::less<Key>(), alloc);
}

/**
 * Destruct the container
 * @param ptr the pointer to the container
 */
//static
template <typename Key, typename Field>
inline void shared_map<Key, Field>::destruct(container_type *ptr)
{
}

/**
 * Get the pointer to the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename shared_map<Key, Field>::container_type*
    shared_map<Key, Field>::get(const std::string& name, const size_type size)
{
    OUROBOROS_THROW_BUG("method not supported");
    return NULL;
}

//==============================================================================
//  shared_multimap
//==============================================================================
/**
 * Construct the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename shared_multimap<Key, Field>::container_type*
    shared_multimap<Key, Field>::construct(const std::string& name, const size_type size)
{
    boost::interprocess::managed_shared_memory& mem = shared_memory::instance().mem();
    static allocator alloc(mem.get_segment_manager());
    return mem.find_or_construct<container_type>(name.c_str())(std::less<Key>(), alloc);
}

/**
 * Destruct the container
 * @param ptr the pointer to the container
 */
//static
template <typename Key, typename Field>
inline void shared_multimap<Key, Field>::destruct(container_type *ptr)
{
}

/**
 * Get the pointer to the container
 * @param name the name of the container
 * @param size the size of the container
 * @return the pointer to the container
 */
//static
template <typename Key, typename Field>
inline typename shared_multimap<Key, Field>::container_type*
    shared_multimap<Key, Field>::get(const std::string& name, const size_type size)
{
    OUROBOROS_THROW_BUG("method not supported");
    return NULL;
}

}   // namespace ouroboros

#endif	/* OUROBOROS_SHAREDCONTAINER_H */

