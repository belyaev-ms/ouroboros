/**
 * @file   file.h
 * The base file for a dataset
 */

#ifndef OUROBOROS_FILE_H
#define	OUROBOROS_FILE_H

#include <string>

#include "ouroboros/global.h"
#include "page.h"

namespace ouroboros
{

/** The state of a transaction */
enum transaction_state
{
    TR_STARTED,     ///< the transaction is in progress
    TR_STOPPED,     ///< the transaction terminated
    TR_CANCELED,    ///< the transaction canceled
    TR_UNKNOWN      ///< the transaction has unknown state
};

/**
 * The base file
 */
class base_file
{
public:
    enum
    {
        /** WITHOUT CACHE */
        CACHE_PAGE_SIZE = 1,
        CACHE_PAGE_COUNT = 0
    };

    explicit base_file(const std::string& name);
    virtual ~base_file();

    const std::string& name() const; ///< get the name of the file
    bool init(); ///< ititialize
    void read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    void write(const void *buffer, size_type size, const pos_type pos); ///< write data
    size_type resize(const size_type size); ///< change the size of the file
    size_type sizeup(const size_type size); ///< size up the file
    size_type size() const; ///< get the size of the file
    void refresh(size_type size, const pos_type pos); ///< refresh data
    void flush() const; ///< forced synchronization data of the file

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    transaction_state state() const; ///< get the state of the transaction

    static void remove(const std::string& name); ///< remove a file by the name
protected:
    virtual void do_read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    virtual void do_write(const void *buffer, size_type size, const pos_type pos); ///< write data
    virtual size_type do_resize(const size_type size); ///< change the size of the file
    virtual size_type do_sizeup(const size_type size); ///< size up the file
    virtual size_type do_size() const; ///< get the size of the file
    virtual void do_refresh(size_type size, const pos_type pos); ///< refresh data
    int fd() const; ///< get the file descriptor
private:
    const std::string m_name; ///< the file name
    const int m_fd; ///< the file descriptor
};

/**
 * The simple file
 */
template <typename FilePage = file_page<1> >
class file : public base_file
{
public:
    typedef base_file simple_file;
    typedef FilePage file_page_type;
    typedef file_region<file_page_type> file_region_type;
    explicit file(const std::string& name);
    file(const std::string& name, const file_region_type& region);
    void set_region(const file_region_type& region); ///< the region of the file
    void read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    void write(const void *buffer, size_type size, const pos_type pos); ///< write data
    size_type resize(const size_type size); ///< change the size of the file
    size_type sizeup(const size_type size); ///< size up the file
    void refresh(size_type size, const pos_type pos); ///< refresh data
protected:
    inline offset_type convert_offset(const offset_type raw_offset) const; ///< convert the raw offset to the real offset in the file
    inline size_type convert_size(const offset_type raw_size) const; ///< convert the raw size to the real size in the file
private:
    const file_region_type *m_region; ///< the file region
};

/**
 * Constructor
 * @param name the name of a file
 */
template <typename FilePage>
file<FilePage>::file(const std::string& name) :
    base_file(name),
    m_region(NULL)
{
}

/**
 * Constructor
 * @param name the name of a file
 * @param region the region of a file
 */
template <typename FilePage>
file<FilePage>::file(const std::string& name, const file_region_type& region) :
    base_file(name),
    m_region(&region)
{

}

/**
 * Set the region
 * @param region the region of the file
 */
template <typename FilePage>
void file<FilePage>::set_region(const file_region_type& region)
{
    m_region = &region;
}

/**
 * Convert the raw offset to the real offset in the file
 * @param raw_offset the raw offset
 * @return the real offset in the file
 */
template <typename FilePage>
inline offset_type file<FilePage>::convert_offset(const offset_type raw_offset) const
{
    return NULL == m_region ? raw_offset : m_region->convert_offset(raw_offset);
}

/**
 * Convert the raw size to the real size in the file
 * @param raw_size the raw size
 * @return the real size in the file
 */
template <typename FilePage>
inline size_type file<FilePage>::convert_size(const size_type raw_size) const
{
    return NULL == m_region ? raw_size : m_region->convert_size(raw_size);
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <typename FilePage>
void file<FilePage>::read(void *buffer, size_type size, const pos_type pos) const
{
    base_file::read(buffer, size, convert_offset(pos));
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <typename FilePage>
void file<FilePage>::write(const void *buffer, size_type size, const pos_type pos)
{
    base_file::write(buffer, size, convert_offset(pos));
}

/**
 * Change the size of the base_file
 * @param size the size of the base_file
 */
template <typename FilePage>
size_type file<FilePage>::resize(const size_type size)
{
    return do_resize(convert_size(size));
}

/**
 * Size up the file
 * @param size the new size
 * @return the size of the file
 */
template <typename FilePage>
size_type file<FilePage>::sizeup(const size_type size)
{
    return do_sizeup(convert_size(size));
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
template <typename FilePage>
void file<FilePage>::refresh(size_type size, const pos_type pos)
{
    const offset_type offset = convert_offset(pos);
    const offset_type real_size = convert_offset(pos + size) - offset;
    do_refresh(real_size, convert_offset(pos));
}

}   //namespace ouroboros

#endif	/* OUROBOROS_FILE_H */

