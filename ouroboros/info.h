/**
 * @file   info.h
 * The information of a dataset
 */

#ifndef OUROBOROS_INFO_H
#define	OUROBOROS_INFO_H

#include <iostream>
#include <iomanip>
#include <string.h>

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The information of a dataset
 */
class info
{
    friend std::ostream& operator << (std::ostream& s, const info& info);
public:
    inline info();
    inline info(const count_type tc, const count_type rc, const count_type kc = 0,
        const count_type ver = 0, const void *user_data = NULL, const size_type user_size = 0);

    inline bool operator == (const info& o) const;
    inline bool operator != (const info& o) const;

    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline size_type get_data(void *buffer, const size_type size) const;
    inline size_type set_data(const void *buffer, const size_type size);
    inline bool compare_data(const info& o) const;

    static size_type static_size()
    {
        return sizeof(count_type) + sizeof(count_type) + sizeof(count_type) +
            sizeof(count_type) + RESERVE_SIZE + DATA_SIZE;
    }
public:
    enum
    {
        COUNT           = 6,    ///< the count of fields in the informations
        RESERVE_SIZE    = 512,  ///< the size of reserved region
        DATA_SIZE       = 256   ///< the size of user data region
    };
    count_type version;        ///< the version of a dataset
    count_type tbl_count;      ///< the count of tables in a dataset
    count_type rec_count;      ///< the count of records in a table
    count_type key_count;      ///< the count of keys
private:
    char reserve[RESERVE_SIZE]; ///< the reserved region
    char data[DATA_SIZE];       ///< the user data region
};

/**
 * Convert a memory dump to a hex string
 * @param buffer the pointer to memory buffer
 * @param size the size of the buffer
 * @return a hex string
 */
inline std::string dump_memory(const void *buffer, const size_type size)
{
    std::ostringstream s;
    const char *p = reinterpret_cast<const char*>(buffer);
    for (size_type i = 0; i < size; ++i)
    {
        s << std::hex << std::setw(2) << ((const unsigned int)(p[i]) & 0xFF) << " ";
    }
    return s.str();
}

//==============================================================================
//  info
//==============================================================================
/**
 * Default constructor
 */
inline info::info() :
    version(0), tbl_count(0), rec_count(0), key_count(0)
{
    memset(reserve, 0, sizeof(reserve));
    memset(data, 0, sizeof(data));
}

/**
 * Constructor
 * @param tc the count of the tables
 * @param rc the count of the records
 * @param kc the count of the keys
 * @param ver the version of the dataset
 * @param user_data the pointer to buffer of user data
 * @param user_size the size of the user data
 */
inline info::info(const count_type tc, const count_type rc, const count_type kc,
    const count_type ver, const void *user_data, const size_type user_size) :
    version(ver), tbl_count(tc), rec_count(rc), key_count(kc)
{
    memset(reserve, 0, sizeof(reserve));
    if (NULL == user_data || 0 == user_size)
    {
        memset(data, 0, sizeof(data));
    }
    else
    {
        set_data(user_data, user_size);
    }
}

/**
 * The operator ==
 * @param o another information
 * @return the result of the checking
 */
inline bool info::operator == (const info& o) const
{
    return version == o.version &&
        tbl_count == o.tbl_count &&
        rec_count == o.rec_count &&
        key_count == o.key_count &&
        0 == memcmp(reserve, o.reserve, sizeof(reserve)) &&
        0 == memcmp(data, o.data, sizeof(data));
}

/**
 * The operator !=
 * @param o another information
 * @return the result of the checking
 */
inline bool info::operator != (const info& o) const
{
    return !(*this == o);
}

/**
 * Pack the information to the buffer
 * @param out the pointer to data of the information
 * @return the pointer to data of the next information
 */
inline void * info::pack(void *out) const
{
    char *buf = static_cast<char *>(out);
    memcpy(buf, &version, sizeof(version));
    buf += sizeof(version);
    memcpy(buf, &tbl_count, sizeof(tbl_count));
    buf += sizeof(tbl_count);
    memcpy(buf, &rec_count, sizeof(rec_count));
    buf += sizeof(rec_count);
    memcpy(buf, &key_count, sizeof(key_count));
    buf += sizeof(key_count);
    memcpy(buf, reserve, sizeof(reserve));
    buf += sizeof(reserve);
    memcpy(buf, data, sizeof(data));
    buf += sizeof(data);
    return buf;
}

/**
 * Unpack the information from the buffer
 * @param in the pointer to data of the information
 * @return the pointer to data of the next information
 */
inline const void* info::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(in);
    memcpy(&version, buf, sizeof(version));
    buf += sizeof(version);
    memcpy(&tbl_count, buf, sizeof(tbl_count));
    buf += sizeof(tbl_count);
    memcpy(&rec_count, buf, sizeof(rec_count));
    buf += sizeof(rec_count);
    memcpy(&key_count, buf, sizeof(key_count));
    buf += sizeof(key_count);
    memcpy(reserve, buf, sizeof(reserve));
    buf += sizeof(reserve);
    memcpy(data, buf, sizeof(data));
    buf += sizeof(data);
    return buf;
}

/**
 * Get the size of the information
 * @return the size of the information
 */
inline size_type info::size() const
{
    return sizeof(version) + sizeof(tbl_count) + sizeof(rec_count) + sizeof(key_count) +
        sizeof(reserve) + sizeof(data);
}

/**
 * Get the region of the user data
 * @param buffer the buffer for user data
 * @param size the size of the buffer
 * @return the size of getting data
 */
inline size_type info::get_data(void *buffer, const size_type size) const
{
    const size_type count = size < sizeof(data) ? size : sizeof(data);
    memcpy(buffer, data, count);
    return count;
}

/**
 * Set the region of the user data
 * @param buffer the buffer for user data
 * @param size the size of the buffer
 * @return the size of setting data
 */
inline size_type info::set_data(const void *buffer, const size_type size)
{
    const size_type count = size < sizeof(data) ? size : sizeof(data);
    memcpy(data, buffer, count);
    return count;
}

/**
 * Compare the user data
 * @param o the another information
 * @return the result of the comparing (true - the user data is equal)
 */
inline bool info::compare_data(const info& o) const
{
    return 0 == memcmp(data, o.data, sizeof(data));
}

/**
 * Output the information
 * @param s the stream
 * @param info the information
 * @return the stream
 */
inline std::ostream& operator << (std::ostream& s, const info& info)
{
    const std::string dump = dump_memory(info.data, sizeof(info.data));
    s << "ver = " << info.version
      << ", tbl_count = " << info.tbl_count
      << ", rec_count = " << info.rec_count
      << ", key_count = " << info.key_count
      << ", data = [ " << dump << " ]";
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_INFO_H */

