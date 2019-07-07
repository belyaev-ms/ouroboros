/** 
 * @file   memory.h
 * The shared memory for shared objects
 */

#ifndef OUROBOROS_MEMORY_H
#define	OUROBOROS_MEMORY_H

#ifndef OUROBOROS_BOOST_ENABLED
#error Can not use stg_memory (OUROBOROS_BOOST_ENABLED)
#endif

#include <map>
#include <string>
#include <iostream>
#include <assert.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The shared memory
 */
class shared_memory
{
public:
    enum { DEFAULT_SIZE = 4 * 1024 * 1024 };
public:
    static shared_memory& instance();
    static void remove();

    boost::interprocess::managed_shared_memory& mem();

    static const std::string& get_name();
    static void set_name(const std::string& name);
    static const size_type get_size();
    static void set_size(const size_type size);
private:
    shared_memory();
    ~shared_memory();
    shared_memory(const shared_memory&);
    shared_memory& operator=(const shared_memory&);

    void attach();
    void detach();
private:
    boost::interprocess::managed_shared_memory m_mem;
    static bool s_init;
    static size_type s_size;
    static std::string s_name;
};

#ifdef OUROBOROS_SHOW_MEMORY_INFO
/**
 * Show the information about current state of shared memory
 * @param name the name of the shared object
 */
inline void show_sharedmem_info(const char *name = NULL)
{
    shared_memory& self = shared_memory::instance();
    static const size_type fullmem = self.mem().get_free_memory();
    static size_type freemem = fullmem;
    const size_type objmem = freemem - self.mem().get_free_memory();
    freemem = self.mem().get_free_memory();
    const size_type allocmem = fullmem - freemem;
    std::cout
        << (NULL == name ? "?" : name)
        << "\t[ " << objmem
        << "\t/ " << freemem
        << "\t/ " << allocmem << "\t]"
        << std::endl;
}
#endif

}   //namespace ouroboros

#endif	/* OUROBOROS_MEMORY_H */

