/**
 * @file   file.h
 * The base file for a dataset
 */

#ifndef OUROBOROS_FILE_H
#define	OUROBOROS_FILE_H

#include <string>

#include "ouroboros/global.h"

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
class file
{
public:
    enum
    {
        /** WITHOUT CACHE */
        CACHE_PAGE_SIZE = 1,
        CACHE_PAGE_COUNT = 0
    };

    explicit file(const std::string& name);
    virtual ~file();

    const std::string& name() const; ///< get the name of the file
    const bool init(); ///< ititialize
    void read(void *buffer, size_type size, const pos_type pos) const; ///< read data
    void write(const void *buffer, size_type size, const pos_type pos); ///< write data
    const size_type resize(const size_type size); ///< change the size of the file
    const size_type size() const; ///< get the size of the file
    void refresh(size_type size, const pos_type pos); ///< refresh data
    void flush() const; ///< forced synchronization data of the file

    void start();  ///< start the transaction
    void stop();   ///< stop the transaction
    void cancel(); ///< cancel the transaction
    const transaction_state state() const; ///< get the state of the transaction

    static void remove(const std::string& name); ///< remove a file by the name
protected:
    const int fd() const; ///< get the file descriptor
private:
    const std::string m_name; ///< the file name
    const int m_fd; ///< the file descriptor
};

}   //namespace ouroboros

#endif	/* OUROBOROS_FILE_H */

