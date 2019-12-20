/**
 * @file   memoryfile.h
 * The file stored in memory for a dataset
 */

#ifndef OUROBOROS_MEMORYFILE_H
#define	OUROBOROS_MEMORYFILE_H

#include <string.h>
#include "ouroboros/file.h"

namespace ouroboros
{

/**
 * The file stored in local memory
 */
class memory_file
{
public:
    enum
    {
        /** WITHOUT CACHE */
        CACHE_PAGE_SIZE = 1,
        CACHE_PAGE_COUNT = 0
    };

    explicit memory_file(const std::string& name);
    virtual ~memory_file();

    const std::string& name() const; ///< get the name of the file
    bool init(); ///< ititialize
    void read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    void write(const void *buffer, size_type size, const pos_type pos); ///< write data
    size_type resize(const size_type size); ///< change the size of the file
    size_type size() const; ///< get the size of the file
    void refresh(size_type size, const pos_type pos); ///< refresh data
    void flush() const; ///< forced synchronization data of the file

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    transaction_state state() const; ///< get the state of the transaction

    static void remove(const std::string& name); ///< remove a file by the name
private:
    const std::string m_name; ///< the file name
    void *m_data; ///< the data of the file
    size_type m_size; ///< the size of the data
};

}   //namespace ouroboros

#endif	/* OUROBOROS_MEMORYFILE_H */

