/**
 * @file   log.h
 * The functions and the macros for outputting messages to the log
 */

#ifndef OUROBOROS_LOG_H
#define	OUROBOROS_LOG_H

#define PR(x) #x << " = " << x << ", "
#define PE(x) #x << " = " << x

#ifndef OUROBOROS_LOG
#include <string.h>
#include <iostream>
#define OUROBOROS_SHORT_FILE  ouroboros::get_short_filename(__FILE__)
/** macros for outputting the messages to the log */
#define OUROBOROS_LOG(msg) do { std::cout << msg << std::endl; } while(0)
/** macros for outputting the messages with additional information to the log */
#define OUROBOROS_MESSAGE(prefix, msg) OUROBOROS_LOG(prefix << "\t" << OUROBOROS_SHORT_FILE << "::" << __FUNCTION__ << "(" << __LINE__ << "):\t" << msg)
/** macros for outputting the messages by categories */
#define OUROBOROS_INFO(msg)   OUROBOROS_MESSAGE("INFO:", msg)
#define OUROBOROS_ERROR(msg)  OUROBOROS_MESSAGE("ERROR:", msg)
#define OUROBOROS_DEBUG(msg)  //OUROBOROS_MESSAGE("DEBUG:", msg)

namespace ouroboros
{

/**
 * Get the short file name
 * @param name the file name
 * @return the short file name
 */
inline const char* get_short_filename(const char *name)
{
    const char *p = strrchr(name, '/');
    return (p != NULL) ? p + 1 : name;
}

}   //namespace ouroboros

#endif

#endif	/* OUROBOROS_LOG_H */
