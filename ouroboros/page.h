/**
 * @file    page.h
 * The page is a part fo a data file
 */

#ifndef OUROBOROS_PAGE_H
#define OUROBOROS_PAGE_H

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The base file page that provide access to useful data and to service data
 */
template <int pageSize>
class file_page
{
public:
    enum
    {
        FILE_PAGE_SIZE = pageSize
    };

    file_page();
    explicit file_page(const pos_type pos);
    file_page(const file_page& page);
    const pos_type pos() const;
    const pos_type index() const;
    void assign(void *ptr);
    void *read(void *buffer) const;
    void *read(void *buffer, const size_type size) const;
    void *read_rest(void *buffer) const;
    const void *write(const void *buffer);
    const void *write(const void *buffer, const size_type size);
    const void *write_rest(const void *buffer);
    const bool valid() const;
    const bool operator== (const file_page& page) const;
    const bool operator< (const file_page& page) const;
    file_page& operator++ ();
    file_page& operator= (const file_page& page);
    static const size_type static_size();
private:
    void *do_read(void *buffer, const pos_type offset, const size_type size) const;
    const void *do_write(const void *buffer, const pos_type offset, const size_type size);
private:
    pos_type m_pos;
    pos_type m_index;
    void *m_ptr;
};

//==============================================================================
//  file_page
//==============================================================================
/**
 * Get the size of the file page
 * @return
 */
template <int pageSize>
//static
const size_type file_page<pageSize>::static_size()
{
    return pageSize;
}

/**
 * Constructor
 */
template <int pageSize>
file_page<pageSize>::file_page() :
    m_pos(NIL),
    m_index(NIL),
    m_ptr(NULL)
{
}

/**
 * Constructor
 * @param pos the position in the data file
 */
template <int pageSize>
file_page<pageSize>::file_page(const pos_type pos) :
    m_pos(pos),
    m_index(pos / pageSize),
    m_ptr(NULL)
{
}

/**
 * Constructor
 * @param page another file page
 */
template <int pageSize>
file_page<pageSize>::file_page(const file_page& page) :
    m_pos(page.m_pos),
    m_index(page.m_index),
    m_ptr(page.m_ptr)
{
}

/**
 * Get the position in the data file
 * @return the position in the data file
 */
template <int pageSize>
const pos_type file_page<pageSize>::pos() const
{
    return m_pos;
}

/**
 * Get the index of the file page
 * @return the index of the file page
 */
template <int pageSize>
const pos_type file_page<pageSize>::index() const
{
    return m_index;
}

/**
 * Assign a memory data page
 * @param ptr the pointer to a memory data page
 */
template <int pageSize>
void file_page<pageSize>::assign(void *ptr)
{
    m_ptr = ptr;
}

/**
 * Read data into the buffer
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
void *file_page<pageSize>::read(void *buffer) const
{
    const pos_type offset = m_pos % pageSize;
    const size_type size = pageSize - offset;
    return do_read(buffer, offset, size);
}

/**
 * Read rest of data into the buffer
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
void *file_page<pageSize>::read_rest(void *buffer) const
{
    const size_type size = m_pos % pageSize + 1;
    return do_read(buffer, 0, size);
}

/**
 * Read rest of data into the buffer
 * @param buffer the pointer to the buffer
 * @param size the size of the useful memory block
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
void *file_page<pageSize>::read(void *buffer, const size_type size) const
{
    const pos_type offset = m_pos % pageSize;
    return do_read(buffer, offset, size);
}

/**
 * Read data into the buffer
 * @param buffer the pointer to the buffer
 * @param offset the offset of the useful data
 * @param size the size of the useful data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
void *file_page<pageSize>::do_read(void *buffer, const pos_type offset, const size_type size) const
{
    OUROBOROS_ASSERT(buffer != NULL && valid() && offset + size <= FILE_PAGE_SIZE);
    memcpy(buffer, static_cast<char *>(m_ptr) + offset, size);
    return static_cast<char *>(buffer) + size;
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
const void *file_page<pageSize>::write(const void *buffer)
{
    const pos_type offset = m_pos % pageSize;
    const size_type size = pageSize - offset;
    return do_write(buffer, offset, size);
}

/**
 * Write the rest of data into the file page
 * @param buffer the pointer to the buffer
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
const void *file_page<pageSize>::write_rest(const void *buffer)
{
    const size_type size = m_pos % pageSize + 1;
    return do_write(buffer, 0, size);
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @param size the size of data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
const void *file_page<pageSize>::write(const void *buffer, const size_type size)
{
    const pos_type offset = m_pos % pageSize;
    return do_write(buffer, offset, size);
}

/**
 * Write data into the file page
 * @param buffer the pointer to the buffer
 * @param offset the offset in the file page
 * @param size the size of data
 * @return the pointer to the next position of the buffer
 */
template <int pageSize>
const void *file_page<pageSize>::do_write(const void *buffer, const pos_type offset, const size_type size)
{
    OUROBOROS_ASSERT(buffer != NULL && valid() && offset + size <= FILE_PAGE_SIZE);
    memcpy(static_cast<char *>(m_ptr) + offset, buffer, size);
    return static_cast<const char *>(buffer) + size;
}

/**
 * Check the file page is valid
 * @return the result of the checking
 */
template <int pageSize>
const bool file_page<pageSize>::valid() const
{
    return m_ptr != NULL && m_pos != NIL && m_index != NIL;
}

/**
 * Operator ==
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize>
const bool file_page<pageSize>::operator== (const file_page& page) const
{
    return m_index == page.m_index;
}

/**
 * Operator <
 * @param page the another file page
 * @return the result of the checking
 */
template <int pageSize>
const bool file_page<pageSize>::operator< (const file_page& page) const
{
    return m_index < page.m_index;
}

/**
 * Operator ++
 * @return the next file page
 */
template <int pageSize>
file_page<pageSize>& file_page<pageSize>::operator++ ()
{
    ++m_index;
    m_pos = m_index * pageSize;
    m_ptr = NULL;
    return *this;
}

/**
 * Operator =
 * @param page the another file page
 * @return reference to yourself
 */
template <int pageSize>
file_page<pageSize>& file_page<pageSize>::operator= (const file_page& page)
{
    m_pos = page.m_pos;
    m_index = page.m_index;
    m_ptr = page.m_ptr;
    return *this;
}

}   //namespace ouroboros

#endif /* OUROBOROS_PAGE_H */

