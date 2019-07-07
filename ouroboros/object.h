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
    const std::string& s = out.str();
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
    inline static pointer allocate(const object_type& obj); ///< allocate the object
    inline static void destruct(pointer ptr); ///< destruct the object
    inline static const char* name(const pointer ptr); ///< get the name of the object
};

/**
 * Interface class adapter for working with an object
 */
template <typename T, template <typename> class TInterface = local_object>
class object
{
    typedef TInterface<T> interface_type;
public:
    typedef T object_type;
    typedef typename interface_type::pointer pointer;

    inline object(const std::string& name);
    inline object(const std::string& name, const object_type& obj);
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

    inline object(const std::string& name);
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
 * Allocate the object
 * @param obj the initial value of the object
 * @return the pointer to the object
 */
//static
template <typename T>
inline typename local_object<T>::pointer local_object<T>::allocate(const object_type& obj)
{
    return new T(obj);
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
template <typename T, template <typename> class TInterface>
inline object<T, TInterface>::object(const std::string& name) :
    m_ptr(interface_type::construct(name))
{
}

/**
 * Constructor
 * @param name the name of the object
 * @param obj the initial value of the object
 */
template <typename T, template <typename> class TInterface>
inline object<T, TInterface>::object(const std::string& name, const object_type& obj) :
    m_ptr(interface_type::construct(name, obj))
{
}

/**
 * Destructor
 */
template <typename T, template <typename> class TInterface>
object<T, TInterface>::~object()
{
    interface_type::destruct(m_ptr);
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T, template <typename> class TInterface>
inline typename object<T, TInterface>::object_type& object<T, TInterface>::operator() ()
{
    return *m_ptr;
}

/**
 * Get the reference to the object
 * @return the reference to the object
 */
template <typename T, template <typename> class TInterface>
inline const typename object<T, TInterface>::object_type& object<T, TInterface>::operator() () const
{
    return *m_ptr;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T, template <typename> class TInterface>
inline typename object<T, TInterface>::pointer object<T, TInterface>::operator-> ()
{
    return m_ptr;
}

/**
 * Get the pointer to the object
 * @return the pointer to the object
 */
template <typename T, template <typename> class TInterface>
inline const typename object<T, TInterface>::pointer object<T, TInterface>::operator-> () const
{
    return m_ptr;
}

/**
 * Get the name of the object
 * @return the name of the object
 */
template <typename T, template <typename> class TInterface>
inline const char* object<T, TInterface>::name() const
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

}   //namespace ouroboros

#endif	/* OUROBOROS_OBJECT_H */

