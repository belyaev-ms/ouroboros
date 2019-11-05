#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "ouroboros/file.h"

#define OUROBOROS_IO_ERROR_MAX 100
#define OUROBOROS_IO_ERROR_DELAY 100

namespace ouroboros
{

//==============================================================================
//  base_file
//==============================================================================
/**
 * Remove a base_file by the name
 * @param name the name of the base_file
 */
//static
void base_file::remove(const std::string& name)
{
    ::remove(name.c_str());
}

/**
 * Constructor
 * @param name the name of the base_file
 */
base_file::base_file(const std::string& name) :
    m_name(name),
    m_fd(open(name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))
{
    if (-1 == m_fd)
    {
        OUROBOROS_THROW_ERROR(io_error, "error of opening: " << PR(m_name) << PE(errno));
    }
}

/**
 * Destructor
 */
//virtual
base_file::~base_file()
{
    if (close(m_fd) == -1)
    {
        OUROBOROS_ERROR("error of closing: " << PR(m_fd) << PR(m_name) << PE(errno));
    }
}

/**
 * Get the name of the base_file
 * @return the name of the base_file
 */
const std::string& base_file::name() const
{
    return m_name;
}

/**
 * Initialize
 * @return result of the initialization
 */
const bool base_file::init()
{
    return true;
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
//virtual
void base_file::do_read(void *buffer, size_type size, const pos_type pos) const
{
    size_t count = 0;
    ssize_t result = pread(m_fd, buffer, size, pos);
    while (result != static_cast<ssize_t>(size))
    {
        if (result != -1)
        {
            if (++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of reading: " << 
                    PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PE(result));
            }
        }
        else
        {
            const int err = errno;
            if (err != EINTR && ++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of reading: " << 
                    PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PR(result) << PE(err));
            }
        }
        usleep(OUROBOROS_IO_ERROR_DELAY);
        result = pread(m_fd, buffer, size, pos);
    }
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
//virtaul
void base_file::do_write(const void *buffer, size_type size, const pos_type pos)
{
    size_t count = 0;
    ssize_t result = pwrite(m_fd, buffer, size, pos);
    while (result != static_cast<ssize_t>(size))
    {
        if (result != -1)
        {
            if (++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of writing: " << 
                    PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PE(result));
            }
        }
        else
        {
            const int err = errno;
            if (err != EINTR && ++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of writing: " << 
                    PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PR(result) << PE(err));
            }
        }
        usleep(OUROBOROS_IO_ERROR_DELAY);
        result = pwrite(m_fd, buffer, size, pos);
    }
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
void base_file::read(void *buffer, size_type size, const pos_type pos) const
{
    do_read(buffer, size, pos);
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
void base_file::write(const void *buffer, size_type size, const offset_type pos)
{
    do_write(buffer, size, pos);
}

/**
 * Change the size of the base_file
 * @param size the size of the base_file
 */
//virtual
const size_type base_file::do_resize(const size_type size)
{
    const size_type current_size = base_file::do_size();
    if (ftruncate(m_fd, size) == -1)
    {
        OUROBOROS_THROW_ERROR(io_error, "error of changing size: " << 
            PR(m_fd) << PR(m_name) << PR(size) << PE(errno));
    }
    if (current_size < size)
    {
        char buffer[OUROBOROS_PAGE_SIZE] = { 0 };
        for (pos_type pos = current_size; pos < size; pos += OUROBOROS_PAGE_SIZE)
        {
            const size_type count = std::min(static_cast<size_type>(OUROBOROS_PAGE_SIZE), size - pos);
            base_file::do_write(buffer, count, pos);
        }
    }
    return size;
}

/**
 * Change the size of the base_file
 * @param size the size of the base_file
 */
const size_type base_file::resize(const size_type size)
{
    return do_resize(size);
}

/**
 * Size up the file
 * @param size the new size
 * @return the size of the file
 */
//virtual
const size_type base_file::do_sizeup(const size_type size)
{
    const size_type current_size = do_size();
    return current_size < size ? do_resize(size) : current_size;
}

/**
 * Size up the file
 * @param size the new size
 * @return the size of the file
 */
const size_type base_file::sizeup(const size_type size)
{
    return do_sizeup(size);
}

/**
 * Get the size of the base_file
 * @return the size of the base_file
 */
//virtual
const size_type base_file::do_size() const
{
    const off_t result = lseek(m_fd, 0, SEEK_END);
    if (-1 == result)
    {
        OUROBOROS_THROW_ERROR(io_error, "error of getting size: " << PR(m_fd) << PR(m_name) << PE(errno));
    }
    return result;
}

/**
 * Get the size of the base_file
 * @return the size of the base_file
 */
const size_type base_file::size() const
{
    return do_size();
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
//virtual
void base_file::do_refresh(size_type size, const pos_type pos)
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
void base_file::refresh(size_type size, const pos_type pos)
{
    do_refresh(size, pos);
}

/**
 * Forced synchronization data of the base_file
 */
void base_file::flush() const
{
    size_t count = 0;
    while (fdatasync(m_fd) != 0)
    {
        const int err = errno;
        if (err != EINTR && ++count >= OUROBOROS_IO_ERROR_MAX)
        {
            OUROBOROS_THROW_ERROR(io_error, "error of synchronization: " << PR(m_fd) << PR(m_name) << PE(err));
        }
        usleep(OUROBOROS_IO_ERROR_DELAY);
    }
}

/**
 * Get the base_file descriptor
 * @return the base_file descriptor
 */
const int base_file::fd() const
{
    return m_fd;
}

/**
 * Start the transaction
 */
void base_file::start()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Stop the transaction
 */
void base_file::stop()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Cancel the transaction
 */
void base_file::cancel()
{
    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
const transaction_state base_file::state() const
{
    return TR_UNKNOWN;
}

}   //namespace ouroboros