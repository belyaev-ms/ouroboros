#include <stdlib.h>
#include "ouroboros/memoryfile.h"

namespace ouroboros
{

//==============================================================================
//  memory_file
//==============================================================================
/**
 * Remove a file by the name
 * @param name the name of the file
 */
//static
void memory_file::remove(const std::string& name)
{
    OUROBOROS_UNUSED(name);
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Constructor
 * @param name the name of the file
 */
memory_file::memory_file(const std::string& name) :
    m_name(name),
    m_data(NULL),
    m_size(0)
{
}

/**
 * Destructor
 */
//virtual
memory_file::~memory_file()
{
    if (m_data != NULL)
    {
        free(m_data);
    }
}

/**
 * Get the name of the file
 * @return the name of the file
 */
const std::string& memory_file::name() const
{
    return m_name;
}

/**
 * Initialize
 * @return result of the initialization
 */
bool memory_file::init()
{
    return true;
}

/**
 * Read data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
void memory_file::read(void *buffer, size_type size, const pos_type pos) const
{
    OUROBOROS_ASSERT(pos != NIL && (pos + size) <= m_size);
    const char *data = static_cast<const char *>(m_data) + pos;
    memcpy(buffer, data, size);
}

/**
 * Write data
 * @param buffer the buffer of the data
 * @param size the size of the data
 * @param pos the position of the data
 */
void memory_file::write(const void *buffer, size_type size, const pos_type pos)
{
    OUROBOROS_ASSERT(pos != NIL && (pos + size) <= m_size);
    char *data = static_cast<char *>(m_data) + pos;
    memcpy(data, buffer, size);
}

/**
 * Change the size of the file
 * @param size the size of the file
 */
size_type memory_file::resize(const size_type size)
{
    OUROBOROS_ASSERT(size > 0);
    if (size != m_size)
    {
        void *data = realloc(m_data, size);
        OUROBOROS_ASSERT(data != NULL);
        m_data = data;
        if (size > m_size)
        {
            memset(static_cast<char *>(m_data) + m_size, 0, size - m_size);
        }
        m_size = size;
    }
    return m_size;
}

/**
 * Get the size of the file
 * @return the size of the file
 */
size_type memory_file::size() const
{
    return m_size;
}

/**
 * Refresh data
 * @param size the size of the data
 * @param pos the position of the data
 */
void memory_file::refresh(size_type size, const pos_type pos)
{
    OUROBOROS_UNUSED(size);
    OUROBOROS_UNUSED(pos);
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Forced synchronization data of the file
 */
void memory_file::sync() const
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Start the transaction
 */
void memory_file::start()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Stop the transaction
 */
void memory_file::stop()
{
//    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Cancel the transaction
 */
void memory_file::cancel()
{
    OUROBOROS_THROW_BUG("method not supported");
}

/**
 * Get the state of the transaction
 * @return the state of the transaction
 */
transaction_state memory_file::state() const
{
    return TR_UNKNOWN;
}

}   //namespace ouroboros

