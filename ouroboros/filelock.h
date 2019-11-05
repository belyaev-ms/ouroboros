/**
 * @file   filelock.h
 * The file with support of lock
 */

#ifndef OUROBOROS_FILELOCK_H
#define	OUROBOROS_FILELOCK_H

#include <stdio.h>
#include <fcntl.h>
#include "ouroboros/file.h"

namespace ouroboros
{

/**
 * The file with support of locking
 */
template <typename FilePage>
class file_lock : public file<FilePage>
{
    typedef file<FilePage> base_class;
public:
    typedef typename base_class::file_region_type file_region_type;
    explicit file_lock(const std::string& name);
    file_lock(const std::string& name, const file_region_type& region);

    const bool lock(const pos_type pos, const size_type len) const; ///< set the exclusive lock
    const bool unlock(const pos_type pos, const size_type len) const; ///< remove the exclusive lock
    const bool lock_sharable(const pos_type pos, const size_type len) const; ///< set the shared lock
    const bool unlock_sharable(const pos_type pos, const size_type len) const; ///< remove the shared lock
};

//==============================================================================
//  file_lock
//==============================================================================
/**
 * Constructor
 * @param name the name of the file
 */
template <typename FilePage>
file_lock<FilePage>::file_lock(const std::string& name) :
    base_class(name)
{
}

/**
 * Constructor
 * @param name the name of a file
 * @param region the region of a file
 */
template <typename FilePage>
file_lock<FilePage>::file_lock(const std::string& name, const file_region_type& region) :
    base_class(name, region)
{
}

/**
 * Set the exclusive lock
 * @param pos the position of a block for lock
 * @param len the size of block for lock
 * @return the result
 */
template <typename FilePage>
const bool file_lock<FilePage>::lock(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_WRLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(base_class::fd(), F_SETLKW, &lock));
}

/**
 * Remove the exclusive lock
 * @param pos the position of a block for unlock
 * @param len the size of block for unlock
 * @return the result
 */
template <typename FilePage>
const bool file_lock<FilePage>::unlock(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_UNLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(base_class::fd(), F_SETLK, &lock));
}

/**
 * Set the shared lock
 * @param pos the position of a block for lock
 * @param len the size of block for lock
 * @return the result
 */
template <typename FilePage>
const bool file_lock<FilePage>::lock_sharable(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_RDLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(base_class::fd(), F_SETLKW, &lock));
}

/**
 * Remove the shared lock
 * @param pos the position of a block for unlock
 * @param len the size of block for unlock
 * @return the result
 */
template <typename FilePage>
const bool file_lock<FilePage>::unlock_sharable(const pos_type pos, const size_type len) const
{
    return unlock(pos, len);
}

}   //namespace ouroboros

#endif	/* OUROBOROS_FILE_H */

