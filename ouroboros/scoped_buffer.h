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
#include "ouroboros/error.h"

namespace ouroboros
{

/**
 * The scoped pointer
 */
template <typename T>
class scoped_ptr
{
public:
    typedef T value_type;
    typedef value_type* pointer_type;
    scoped_ptr() :
        m_pvalue(NULL)
    {}
    explicit scoped_ptr(pointer_type pvalue) :
        m_pvalue(pvalue)
    {}
    ~scoped_ptr()
    {
        if (m_pvalue != NULL)
        {
            delete m_pvalue;
        }
    }
    void reset(pointer_type pvalue = NULL)
    {
        OUROBOROS_ASSERT(NULL == pvalue || pvalue != m_pvalue);
        if (m_pvalue != NULL)
        {
            delete m_pvalue;
        }
        m_pvalue = pvalue;
    }
    value_type& operator* () const
    {
        OUROBOROS_ASSERT(m_pvalue != 0);
        return *m_pvalue;
    }
    pointer_type operator-> () const
    {
        OUROBOROS_ASSERT(m_pvalue != 0);
        return m_pvalue;
    }
    pointer_type get() const
    {
        return m_pvalue;
    }
private:
    scoped_ptr(const scoped_ptr& );
    scoped_ptr& operator= (const scoped_ptr& );
private:
    pointer_type m_pvalue;
};

/**
 * The scoped buffer
 */
template <typename T>
class scoped_buffer
{
public:
    typedef T value_type;
    typedef value_type* pointer_type;
    inline explicit scoped_buffer(const size_t size = sizeof(T))
    {
        OUROBOROS_ASSERT(size > 0);
        m_buffer = malloc(size);
        assert(m_buffer != NULL);
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
    bool operator== (const scoped_buffer& ) const;
    bool operator!= (const scoped_buffer& ) const;
private:
    void *m_buffer;
};

}   //namespace ouroboros

#endif	/* OUROBOROS_SCOPED_BUFFER_H */

