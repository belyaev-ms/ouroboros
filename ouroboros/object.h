/**
 * @file   object.h
 */

#ifndef OUROBOROS_OBJECT_H
#define	OUROBOROS_OBJECT_H

#include <string>
#include <sstream>
#include <algorithm>

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * Make a name of the object
 * @param s1 the first argument
 * @param s2 the second argument
 * @return the name of the object
 */
template <typename T1, typename T2>
const std::string make_object_name(T1 s1, T2 s2)
{
    std::ostringstream out;
    out << s1 << "." << s2;
    const std::string s = out.str();
    return std::string(std::find(s.rbegin(), s.rend(), '/').base(), s.end());
}

/**
 * Make a name of the object
 * @param s1 the first argument
 * @param s2 the second argument
 * @param s3 the third argument
 * @return the name of the object
 */
template <typename T1, typename T2, typename T3>
const std::string make_object_name(T1 s1, T2 s2, T3 s3)
{
    std::ostringstream out;
    out << make_object_name(s1, s2) << "." << s3;
    return out.str();
}

/**
 * Interface class for an object allocated in the local memory
 */
template <typename T>
class local_object
{
public:
    typedef T object_type;
    typedef T* pointer;

    inline static pointer construct(const std::string& name, const object_type& obj); ///< construct the object
    inline static pointer construct(const std::string& name); ///< construct the object
    inline static pointer construct_array(const std::string& name, const size_t size); ///< construct the array of the objects
    inline static void destruct(pointer ptr); ///< destruct the object
    inline static void destruct_array(pointer ptr); ///< destruct the array of the objects
    inline static const char* name(const pointer ptr); ///< get the name of the object
};

/**
 * The tag of an adopt operation for an object
 */
struct object_adoption_tag {};

/**
 * Interface class adapter for working with an object
 */
template <typename T, template <typename> class Interface = local_object>
class object
{
    typedef Interface<T> interface_type;
public:
    typedef T object_type;
    typedef typename interface_type::pointer pointer;

    explicit inline object(const std::string& name);
    inline object(const std::string& name, const object_type& obj);
    inline object(object_adoption_tag , object_type& obj);
    ~object();
    inline object_type& operator() ();
    inline const object_type& operator() () const;
    inline pointer operator-> ();
    inline const pointer operator-> () const;
    inline const char* name() const;
private:
    object();
private:
    pointer m_ptr;
    bool m_owner;
};

/**
 * Interface class adapter for working with an object allocated in the local memory
 */
template <typename T>
class object<T, local_object>
{
public:
    typedef T object_type;
    typedef T* pointer;

    explicit inline object(const std::string& name);
    inline object(const std::string& name, const object_type& obj);
    inline object_type& operator() ();
    inline const object_type& operator() () const;
    inline pointer operator-> ();
    inline const pointer operator-> () const;
    inline const char* name() const;
private:
    object();
private:
    object_type m_obj;
};

/**
 * Object for which space is reserved
 */
template <typename T>
class reserved_object
{
    typedef reserved_object<T> self_type;
public:
    typedef T value_type;
    reserved_object();
    ~reserved_object();
    value_type& get();
    const value_type& get() const;
    value_type* operator-> ();
    const value_type* operator->() const;
    void reset();
protected:
    void construct() const;
    void destruct() const;
    value_type* get_ptr() const;
private:
    mutable char m_data[sizeof(value_type)];
    mutable bool m_initialized;
};

/**
 * Interface class adapter for working with an array of objects
 */
template <typename T, template <typename> class Interface = local_object>
class object_array
{
protected:
    typedef Interface<T> interface_type;
public:
    typedef T object_type;
    typedef typename interface_type::pointer pointer;

    object_array(const std::string& name, const size_t size);
    ~object_array();
    object_type& get(const size_t index);
    const object_type& get(const size_t index) const;
    object_type& operator[] (const size_t index);
    const object_type& operator[] (const size_t index) const;
    inline size_t size() const;
    inline const char* name() const;
private:
    object_array();
private:
    const size_t m_size;
    pointer m_ptr;
};

/**
 * Interface class adapter for working with a pool of objects
 */
template <typename T, template <typename> class Interface>
class object_pool : public object_array<reserved_object<T>, Interface>
{
    typedef object_array<reserved_object<T>, Interface> base_class;
    typedef typename base_class::interface_type interface_type;
    typedef typename base_class::object_type object_type;
public:
    typedef typename object_type::value_type value_type;
    object_pool(const std::string& name, const size_t size);
    inline value_type& get(const size_t index);
    inline const value_type& get(const size_t index) const;
    inline value_type& operator [](const size_t index);
    inline const value_type& operator [](const size_t index) const;
};

//==============================================================================
//  local_object
//==============================================================================
/**
 * Construct the object
 * @param name the name of the object
 * @param obj the initial value of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename local_object<T>::pointer local_object<T>::construct(const std::string& name, const object_type& obj)
{
    return new object_type(obj);
}

/**
 * Construct the object
 * @param name the name of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename local_object<T>::pointer local_object<T>::construct(const std::string& name)
{
    return new object_type();
}

/**
 * Construct the array of the objects
 * @param name the name of the array
 * @param size the size of the array
 * @return the pointer to the array
 */
//static
template <typename T>
inline typename local_object<T>::pointer local_object<T>::construct_array(const std::string& name, const size_t size)
{
    return new object_type[size];
}

/**
 * Destruct the object
 * @param ptr the pointer to the object
 */
//static
template <typename T>
inline void local_object<T>::destruct(pointer ptr)
{
    delete ptr;
}

/**
 * Destruct the array of the objects
 * @param ptr the pointer to the array
 */
//static
template <typename T>
inline void local_object<T>::destruct_array(pointer ptr)
{
    delete[] ptr;
}

/**
 * Get the name of the object
 * @param ptr the pointer to the object
 * @return the name of the object
 */
//static
template <typename T>
inline const char* local_object<T>::name(const pointer ptr)
{
    return NULL;
}

//==============================================================================
//  object
//==============================================================================
/**
 * Constructor
 * @param name the name of the object
 */
template <typename T, template <typename> class Interface>
inline object<T, Interface>::object(const std::string& name) :
    m_ptr(interface_type::construct(name)),
    m_owner(true)
{
}

/**
 * Constructor
 * @param name the name of the object
 * @param obj the initial value of the object
 */
template <typename T, template <typename> class Interface>
inline object<T, Interface>::object(const std::string& name, const object_type& obj) :
    m_ptr(interface_type::construct(name, obj)),
    m_owner(true)
{
}

/**
 * Constructor
 * @param obj the initial value of the object
 */
template <typename T, template <typename> class Interface>
inline object<T, Interface>::object(object_adoption_tag , object_type& obj) :
    m_ptr(&obj),
    m_owner(false)
{
}

/**
 * Destructor
 */
template <typename T, template <typename> class Interface>
object<T, Interface>::~object()
{
    if (m_owner)
    {
        interface_type::destruct(m_ptr);
    }
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T, template <typename> class Interface>
inline typename object<T, Interface>::object_type& object<T, Interface>::operator() ()
{
    return *m_ptr;
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T, template <typename> class Interface>
inline const typename object<T, Interface>::object_type& object<T, Interface>::operator() () const
{
    return *m_ptr;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T, template <typename> class Interface>
inline typename object<T, Interface>::pointer object<T, Interface>::operator-> ()
{
    return m_ptr;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T, template <typename> class Interface>
inline const typename object<T, Interface>::pointer object<T, Interface>::operator-> () const
{
    return m_ptr;
}

/**
 * Get the name of the object
 * @return the name of the object
 */
template <typename T, template <typename> class Interface>
inline const char* object<T, Interface>::name() const
{
    return interface_type::name(m_ptr);
}

//==============================================================================
//  object<T, local_object>
//==============================================================================
/**
 * Constructor
 * @param name the name of the object
 */
template <typename T>
inline object<T, local_object>::object(const std::string& name)
{
}

/**
 * Constructor
 * @param name the name of the object
 * @param obj the initial value of the object
 */
template <typename T>
inline object<T, local_object>::object(const std::string& name, const object_type& obj) :
    m_obj(obj)
{
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T>
inline typename object<T, local_object>::object_type& object<T, local_object>::operator() ()
{
    return m_obj;
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T>
inline const typename object<T, local_object>::object_type& object<T, local_object>::operator() () const
{
    return m_obj;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T>
inline typename object<T, local_object>::pointer object<T, local_object>::operator-> ()
{
    return &m_obj;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T>
inline const typename object<T, local_object>::pointer object<T, local_object>::operator-> () const
{
    return &m_obj;
}

/**
 * Get the name of the object
 * @return the name of the object
 */
template <typename T>
inline const char* object<T, local_object>::name() const
{
    return NULL;
}

//==============================================================================
//  reserved_object
//==============================================================================
/**
 * Constructor
 */
template <typename T>
reserved_object<T>::reserved_object() :
    m_initialized(false)
{
}

/**
 * Destructor
 */
template <typename T>
reserved_object<T>::~reserved_object()
{
    destruct();
}

/**
 * Get the reference of the object
 * @attention if this object has not yet been built, then the first step is to
 * build this object
 * @return the reference of the object
 */
template <typename T>
typename reserved_object<T>::value_type& reserved_object<T>::get()
{
    construct();
    return *get_ptr();
}

/**
 * Get the reference of the object
 * @attention if this object has not yet been built, then the first step is to
 * build this object
 * @return the reference of the object
 */
template <typename T>
const typename reserved_object<T>::value_type& reserved_object<T>::get() const
{
    construct();
    return *get_ptr();
}

/**
 * Destruct this object
 */
template <typename T>
void reserved_object<T>::reset()
{
    destruct();
}

/**
 * Get the pointer to the object
 * @attention if this object has not yet been built, then the first step is to
 * build this object
 * @return the pointer to the object
 */
template <typename T>
typename reserved_object<T>::value_type* reserved_object<T>::operator->()
{
    return &get();
}

/**
 * Get the pointer to the object
 * @attention if this object has not yet been built, then the first step is to
 * build this object
 * @return the pointer to the object
 */
template <typename T>
const typename reserved_object<T>::value_type* reserved_object<T>::operator->() const
{
    return &get();
}

/**
 * Contruct the object
 */
template <typename T>
void reserved_object<T>::construct() const
{
    if (!m_initialized)
    {
        new(m_data) value_type();
        m_initialized = true;
    }
}

/**
 * Destruct the object
 */
template <typename T>
void reserved_object<T>::destruct() const
{
    if (m_initialized)
    {
        get_ptr()->~value_type();
        m_initialized = false;
    }
}

/**
 * Get the pointer to the reserved place
 * @return the pointer to the reserved place
 */
template <typename T>
typename reserved_object<T>::value_type* reserved_object<T>::get_ptr() const
{
    return reinterpret_cast<value_type*>(m_data);
}

//==============================================================================
//  object_array
//==============================================================================
/**
 * Constructor
 * @param name the name of the array of objects
 * @param size the size of the array
 */
template <typename T, template <typename> class Interface>
object_array<T, Interface>::object_array(const std::string& name, const size_t size) :
    m_size(size),
    m_ptr(interface_type::construct_array(name, size))
{
}

/**
 * Destructor
 */
template <typename T, template <typename> class Interface>
object_array<T, Interface>::~object_array()
{
    interface_type::destruct_array(m_ptr);
}

/**
 * Get an object by the index
 * @param index the index of an object
 * @return an object by the index
 */
template <typename T, template <typename> class Interface>
typename object_array<T, Interface>::object_type&
    object_array<T, Interface>::get(const size_t index)
{
    OUROBOROS_RANGE_ASSERT(index < m_size);
    return m_ptr[index];
}

/**
 * Get an object by the index
 * @param index the index of an object
 * @return an object by the index
 */
template <typename T, template <typename> class Interface>
const typename object_array<T, Interface>::object_type&
    object_array<T, Interface>::get(const size_t index) const
{
    OUROBOROS_RANGE_ASSERT(index < m_size);
    return m_ptr[index];
}

/**
 * Get an object by the index
 * @param index the index of an object
 * @return an object by the index
 */
template <typename T, template <typename> class Interface>
typename object_array<T, Interface>::object_type&
    object_array<T, Interface>::operator[] (const size_t index)
{
    return get(index);
}

/**
 * Get an object by the index
 * @param index the index of an object
 * @return an object by the index
 */
template <typename T, template <typename> class Interface>
const typename object_array<T, Interface>::object_type&
    object_array<T, Interface>::operator[] (const size_t index) const
{
    return get(index);
}

/**
 * Get the size of the array
 * @return the size of the array
 */
template <typename T, template <typename> class Interface>
inline size_t object_array<T, Interface>::size() const
{
    return m_size;
}

/**
 * Get the name of the array
 * @return the name of the array
 */
template <typename T, template <typename> class Interface>
inline const char* object_array<T, Interface>::name() const
{
    return interface_type::name(m_ptr);
}

//==============================================================================
//  object_pool
//==============================================================================
template <typename T, template <typename> class Interface>
object_pool<T, Interface>::object_pool(const std::string& name, const size_t size) :
    base_class(name, size)
{
}

template <typename T, template <typename> class Interface>
inline typename object_pool<T, Interface>::value_type&
    object_pool<T, Interface>::get(const size_t index)
{
    return base_class::operator [](index % base_class::size()).get();
}

template <typename T, template <typename> class Interface>
inline const typename object_pool<T, Interface>::value_type&
    object_pool<T, Interface>::get(const size_t index) const
{
    return base_class::operator [](index % base_class::size()).get();
}

template <typename T, template <typename> class Interface>
inline typename object_pool<T, Interface>::value_type&
    object_pool<T, Interface>::operator [](const size_t index)
{
    return base_class::operator [](index % base_class::size()).get();
}

template <typename T, template <typename> class Interface>
inline const typename object_pool<T, Interface>::value_type&
    object_pool<T, Interface>::operator [](const size_t index) const
{
    return base_class::operator [](index % base_class::size()).get();
}

}   //namespace ouroboros

#endif	/* OUROBOROS_OBJECT_H */

