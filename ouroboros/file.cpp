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
//  file
//==============================================================================
/**
 * Remove a file by the name
 * @param name the name of the file
 */
//static
void file::remove(const std::string& name)
{
    ::remove(name.c_str());
}

/**
 * Constructor
 * @param name the name of the file
 */
file::file(const std::string& name) :
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
file::~file()
{
    if (close(m_fd) == -1)
    {
        OUROBOROS_ERROR("error of closing: " << PR(m_fd) << PR(m_name) << PE(errno));
    }
}

/**
 * Get the name of the file
 * @return the name of the file
 */
const std::string& file::name() const
{
    return m_name;
}

/**
 * Initialize
 * @return result of the initialization
 */
const bool file::init()
{
    return true;
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
void file::read(void *buffer, size_type size, const pos_type pos) const
{
    size_t count = 0;
    ssize_t result = pread(m_fd, buffer, size, pos);
    while (result != static_cast<ssize_t>(size))
    {
        if (result != -1)
        {
            if (++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of reading: " << PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PE(result));
            }
        }
        else
        {
            const int err = errno;
            if (err != EINTR && ++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of reading: " << PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PR(result) << PE(err));
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
void file::write(const void *buffer, size_type size, const pos_type pos)
{
    size_t count = 0;
    ssize_t result = pwrite(m_fd, buffer, size, pos);
    while (result != static_cast<ssize_t>(size))
    {
        if (result != -1)
        {
            if (++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of writing: " << PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PE(result));
            }
        }
        else
        {
            const int err = errno;
            if (err != EINTR && ++count >= OUROBOROS_IO_ERROR_MAX)
            {
                OUROBOROS_THROW_ERROR(io_error, "error of writing: " << PR(m_fd) << PR(m_name) << PR(size) << PR(pos) << PR(result) << PE(err));
            }
        }
        usleep(OUROBOROS_IO_ERROR_DELAY);
        result = pwrite(m_fd, buffer, size, pos);
    }
}

/**
 * Change the size of the file
 * @param size the size of the file
 */
const size_type file::resize(const size_type size)
{
    if (ftruncate(m_fd, size) == -1)
    {
        OUROBOROS_THROW_ERROR(io_error, "error of changing size: " << PR(m_fd) << PR(m_name) << PR(size) << PE(errno));
    }
    return size;
}

/**
 * Get the size of the file
 * @return the size of the file
 */
const size_type file::size() const
{
    const off_t result = lseek(m_fd, 0, SEEK_END);
    if (-1 == result)
    {
        OUROBOROS_THROW_ERROR(io_error, "error of getting size: " << PR(m_fd) << PR(m_name) << PE(errno));
    }
    return result;
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
void file::refresh(size_type size, const pos_type pos)
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Forced synchronization data of the file
 */
void file::flush() const
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
 * Get the file descriptor
 * @return the file descriptor
 */
const int file::fd() const
{
    return m_fd;
}

/**
 * Start the transaction
 */
void file::start()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Stop the transaction
 */
void file::stop()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Cancel the transaction
 */
void file::cancel()
{
    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
const transaction_state file::state() const
{
    return TR_UNKNOWN;
}

}   //namespace ouroboros