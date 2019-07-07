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
class file_lock : public file
{
public:
    inline explicit file_lock(const std::string& name);

    inline const bool lock(const pos_type pos, const size_type len) const; ///< set the exclusive lock
    inline const bool unlock(const pos_type pos, const size_type len) const; ///< remove the exclusive lock
    inline const bool lock_sharable(const pos_type pos, const size_type len) const; ///< set the shared lock
    inline const bool unlock_sharable(const pos_type pos, const size_type len) const; ///< remove the shared lock
};

//==============================================================================
//  file_lock
//==============================================================================
/**
 * Constructor
 * @param name the name of the file
 */
inline file_lock::file_lock(const std::string& name) :
    file(name)
{
}

/**
 * Set the exclusive lock
 * @param pos the position of a block for lock
 * @param len the size of block for lock
 * @return the result
 */
inline const bool file_lock::lock(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_WRLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(fd(), F_SETLKW, &lock));
}

/**
 * Remove the exclusive lock
 * @param pos the position of a block for unlock
 * @param len the size of block for unlock
 * @return the result
 */
inline const bool file_lock::unlock(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_UNLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(fd(), F_SETLK, &lock));
}

/**
 * Set the shared lock
 * @param pos the position of a block for lock
 * @param len the size of block for lock
 * @return the result
 */
inline const bool file_lock::lock_sharable(const pos_type pos, const size_type len) const
{
    struct ::flock lock;
    lock.l_type    = F_RDLCK;
    lock.l_whence  = SEEK_SET;
    lock.l_start   = pos;
    lock.l_len     = len;
    return (-1 != ::fcntl(fd(), F_SETLKW, &lock));
}

/**
 * Remove the shared lock
 * @param pos the position of a block for unlock
 * @param len the size of block for unlock
 * @return the result
 */
inline const bool file_lock::unlock_sharable(const pos_type pos, const size_type len) const
{
    return unlock(pos, len);
}

}   //namespace ouroboros

#endif	/* OUROBOROS_FILE_H */

