#include <stddef.h>

#include "ouroboros/memory.h"

namespace ouroboros
{

using namespace boost::interprocess;

static const char SHMEM_PREFIX[] = "/ouroboros";
static const char SHMEM_COUNTER_NAME[] = "SHMEM_COUNTER";

/**
 * Make a name of a shared memory
 * @param name the source name
 * @return a name of a shared memory
 */
static const std::string make_shmem_name(const std::string& name)
{
    return std::string(SHMEM_PREFIX) + name;
}

//==============================================================================
//  shared_memory
//==============================================================================
bool shared_memory::s_init = false;
size_type shared_memory::s_size = shared_memory::DEFAULT_SIZE;
std::string shared_memory::s_name;

/**
 * Get the name of the shared memory
 * @return the name of the shared memory
 */
//static
const std::string& shared_memory::get_name()
{
    return s_name;
}

/**
 * Set the name of the shared memory
 * @param name the name of the shared memory
 */
//static
void shared_memory::set_name(const std::string& name)
{
    if (s_init)
    {
        OUROBOROS_THROW_BUG("Error renaming shared memory after initialization: " << s_name << " -> " << name);
    }
    else
    {
        s_init = true;
        s_name = name;
    }
}

/**
 * Get the size of the shared memory
 * @return the size of the shared memory
 */
//static
size_type shared_memory::get_size()
{
    return s_size;
}

/**
 * Set the size of the shared memory
 * @param size the size of the shared memory
 */
//static
void shared_memory::set_size(const size_type size)
{
    if (s_init)
    {
        OUROBOROS_THROW_BUG("Error resizing shared memory after initialization: " << s_size << " -> " << size);
    }
    else
    {
        s_size = size;
    }
}

/**
 * Get the instance of the shared memory
 * @return the instance of the shared memory
 */
//static
shared_memory& shared_memory::instance()
{
    static shared_memory self;
    s_init = true;
    return self;
}

/**
 * Remove the shared memory
 */
//static
void shared_memory::remove()
{
    OUROBOROS_DEBUG("remove shared_memory " << get_name());
    shared_memory_object::remove(make_shmem_name(get_name()).c_str());
}

/**
 * Constructor
 */
shared_memory::shared_memory() :
    m_mem(open_or_create, make_shmem_name(get_name()).c_str(), get_size())
{
    attach();
}

/**
 * Destructor
 */
shared_memory::~shared_memory()
{
    detach();
}

/**
 * Get the reference to the manager of the shared memory
 * @return the reference to the manager of the shared memory
 */
managed_shared_memory& shared_memory::mem()
{
    return m_mem;
}

/**
 * Assistant to attach the shared memory
 */
class attach_assistant
{
public:
    explicit attach_assistant(managed_shared_memory& mem) : m_mem(mem) {}
    void operator ()()
    {
        // cppcheck-suppress unreadVariable symbolName=count
        count_type& count = *m_mem.find_or_construct<count_type>(SHMEM_COUNTER_NAME)(0);
        // cppcheck-suppress unreadVariable symbolName=count
        OUROBOROS_DEBUG("attach memory count = " << count);
        ++count;
    }
private:
    managed_shared_memory& m_mem;
};

/**
 * Attach the shared memory
 */
void shared_memory::attach()
{
    attach_assistant func(m_mem);
    m_mem.atomic_func(func);
}

/**
 * Assistant to detach the shared memory
 */
class detach_assistant
{
public:
    explicit detach_assistant(managed_shared_memory& mem) : m_mem(mem) {}
    void operator ()()
    {
        count_type& count = *(m_mem.find<count_type>(SHMEM_COUNTER_NAME).first);
        OUROBOROS_DEBUG("detach memory count = " << count);
        if (count <= 0 || 0 == --count)
        {
            shared_memory::remove();
        }
    }
private:
    managed_shared_memory& m_mem;
};

/**
 * Detach the shared memory
 */
void shared_memory::detach()
{
    detach_assistant func(m_mem);
    m_mem.atomic_func(func);
}

}   //namespace ouroboros