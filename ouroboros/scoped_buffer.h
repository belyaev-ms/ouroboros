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
 * The scoped buffer
 */
template <typename T>
class scoped_buffer
{
public:
    typedef T value_type;
    typedef value_type* pointer_type;
    inline explicit scoped_buffer(const size_t size = sizeof(T)) :
        m_buffer(malloc(size))
    {
    }
    inline ~scoped_buffer()
    {
        free(m_buffer);
    }
    inline pointer_type get() const
    {
        return static_cast<pointer_type>(m_buffer);
    }
private:
    scoped_buffer();
    scoped_buffer(const scoped_buffer& );
    scoped_buffer& operator= (const scoped_buffer& );
    void operator== (const scoped_buffer& ) const;
    void operator!= (const scoped_buffer& ) const;
private:
    void *m_buffer;
};

}   //namespace ouroboros

#endif	/* OUROBOROS_SCOPED_BUFFER_H */

