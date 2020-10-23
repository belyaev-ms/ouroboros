/**
 * @file   sharedobject.h
 * Shared objects
 */

#ifndef OUROBOROS_SHAREDOBJECT_H
#define	OUROBOROS_SHAREDOBJECT_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use sharedobject (OUROBOROS_BOOST_ENABLED)
#endif

#include "ouroboros/memory.h"

namespace ouroboros
{

/**
 * Interface class for an object allocated in the shared memory
 */
template <typename T>
class shared_object
{
public:
    typedef T object_type;
    typedef boost::interprocess::offset_ptr<object_type> pointer;

    inline static object_type* construct(const std::string& name, const object_type& obj); ///< construct the object
    inline static object_type* construct(const std::string& name); ///< construct the object
    inline static object_type* construct_array(const std::string& name, const size_t size); ///< construct the array of the objects
    inline static void destruct(object_type* ptr); ///< destruct the object
    inline static void destruct(pointer& ptr); ///< destruct the object
    inline static void destruct_array(object_type* ptr); ///< destruct the array of the objects
    inline static void destruct_array(pointer& ptr); ///< destruct the array of the objects
    inline static const char* name(const pointer& ptr); ///< get the name of the object
};

//==============================================================================
//  shared_object
//==============================================================================
/**
 * Construct the object
 * @param name the name of the object
 * @param obj the initial value of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename shared_object<T>::object_type* shared_object<T>::construct(const std::string& name, const object_type& obj)
{
    object_type* ptr = shared_memory::instance().mem().find_or_construct<object_type>(name.c_str())(obj);
#ifdef OUROBOROS_SHOW_MEMORY_INFO
    show_sharedmem_info(name);
#endif
    return ptr;
}

/**
 * Construct the object
 * @param name the name of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename shared_object<T>::object_type* shared_object<T>::construct(const std::string& name)
{
    object_type* ptr = shared_memory::instance().mem().find_or_construct<object_type>(name.c_str())();
#ifdef OUROBOROS_SHOW_MEMORY_INFO
    show_sharedmem_info(name);
#endif
    return ptr;
}

/**
 * Construct the array of the objects
 * @param name the name of the array
 * @param size the size of the array
 * @return the pointer to the array
 */
//static
template <typename T>
inline typename shared_object<T>::object_type* shared_object<T>::construct_array(const std::string& name, const size_t size)
{
    object_type* ptr = shared_memory::instance().mem().find_or_construct<object_type>(name.c_str())[size]();
#ifdef OUROBOROS_SHOW_MEMORY_INFO
    show_sharedmem_info(name);
#endif
    return ptr;
}

/**
 * Destruct the object
 * @param ptr the pointer to the object
 */
//static
template <typename T>
inline void shared_object<T>::destruct(object_type* ptr)
{
}

/**
 * Destruct the object
 * @param ptr the pointer to the object
 */
//static
template <typename T>
inline void shared_object<T>::destruct(pointer& ptr)
{
}

/**
 * Destruct the array of the objects
 * @param ptr the pointer to the array
 */
//static
template <typename T>
inline void shared_object<T>::destruct_array(object_type* ptr)
{
}

/**
 * Destruct the array of the objects
 * @param ptr the pointer to the array
 */
//static
template <typename T>
inline void shared_object<T>::destruct_array(pointer& ptr)
{
}

/**
 * Get the name of the object
 * @param ptr the pointer to the object
 * @return the name of the object
 */
//static
template <typename T>
inline const char* shared_object<T>::name(const pointer& ptr)
{
    return shared_memory::instance().mem().get_instance_name(ptr.get());
}

}   // namespace ouroboros

#endif	/* OUROBOROS_SHAREDOBJECT_H */

