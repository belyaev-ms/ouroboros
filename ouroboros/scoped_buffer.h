/**
 * @file   scoped_buffer.h
 * The class scoped buffer
 */

#ifndef OUROBOROS_SCOPED_BUFFER_H
#define	OUROBOROS_SCOPED_BUFFER_H

#if __APPLE__
#include "TargetConditionals.h"
#else
#include <malloc.h>
#endif

namespace ouroboros
{

/**
 * The base scoped buffer
 */
template <typename T>
class base_scoped_buffer
{
public:
    typedef T value_type;
    typedef value_type* pointer_type;
    inline explicit base_scoped_buffer(const size_t size)
    {
        OUROBOROS_ASSERT(size > 0);
        m_buffer = malloc(size);
        assert(m_buffer != NULL);
    }
    inline ~base_scoped_buffer()
    {
        free(m_buffer);
    }
    inline pointer_type get() const
    {
        return static_cast<pointer_type>(m_buffer);
    }
private:
    base_scoped_buffer();
    base_scoped_buffer(const base_scoped_buffer& );
    base_scoped_buffer& operator= (const base_scoped_buffer& );
    bool operator== (const base_scoped_buffer& ) const;
    bool operator!= (const base_scoped_buffer& ) const;
private:
    void *m_buffer;
};

/**
 * The scoped buffer
 */
template <typename T>
class scoped_buffer : public base_scoped_buffer<T>
{
public:
    inline explicit scoped_buffer(const size_t size = sizeof(T)) : 
        base_scoped_buffer<T>(size)
    {}
};

template < >
class scoped_buffer<void> : public base_scoped_buffer<void>
{
public:
    inline explicit scoped_buffer(const size_t size) :
        base_scoped_buffer<void>(size)
    {}
};

}   //namespace ouroboros

#endif	/* OUROBOROS_SCOPED_BUFFER_H */

