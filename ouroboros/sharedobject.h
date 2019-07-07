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
    inline static object_type* allocate(const object_type& obj); ///< allocate the object
    inline static void destruct(object_type* ptr); ///< destruct the object
    inline static void destruct(pointer& ptr); ///< destruct the object
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
 * Allocate the object
 * @param obj the initial value of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename shared_object<T>::object_type* shared_object<T>::allocate(const object_type& obj)
{
    void *ptr = shared_memory::instance().mem().allocate(sizeof(T));
#ifdef OUROBOROS_SHOW_MEMORY_INFO
    show_sharedmem_info();
#endif
    return new(ptr) T(obj);
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

