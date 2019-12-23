/**
 * @file   key.h
 * The simple key of a table
 */

#ifndef OUROBOROS_KEY_H
#define	OUROBOROS_KEY_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The simple key of a table
 */
class simple_key
{
public:
    typedef uint64_t key_type;
    inline simple_key();
    inline simple_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev);

    inline bool operator == (const simple_key& o) const;
    inline bool operator != (const simple_key& o) const;

    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline bool valid() const;

    static size_type static_size()
    {
        return sizeof(key_type) + sizeof(spos_type) + sizeof(pos_type) + sizeof(pos_type) +
            sizeof(count_type) + sizeof(revision_type);
    }

public:
    enum { COUNT = 6 };    ///< the count of the key fields
    key_type   key;        ///< the value of the key
    spos_type  pos;        ///< the position of the table
    pos_type   beg;        ///< the position of the begin reacords
    pos_type   end;        ///< the position of the end reacords
    count_type count;      ///< the count of the records
    revision_type   rev;        ///< the revision of modifying
};

//==============================================================================
//  simple_key
//==============================================================================
/**
 * Constructor
 */
inline simple_key::simple_key() :
    key(0),
    pos(0),
    beg(0),
    end(0),
    count(0),
    rev(0)
{
}

/**
 * Constructor
 * @param akey the value of the key
 * @param apos the position of the table
 * @param abeg the position of the begin records
 * @param aend the position of the end records
 * @param acount the count of the records
 * @param arev the revision of the modifying
 */
inline simple_key::simple_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev) :
    key(akey),
    pos(apos),
    beg(abeg),
    end(aend),
    count(acount),
    rev(arev)
{
}

/**
 * The operator ==
 * @param o the another key
 * @return the result of the checking
 */
inline bool simple_key::operator == (const simple_key& o) const
{
    return key == o.key
        && pos == o.pos
        && beg == o.beg
        && end == o.end
        && count == o.count
        && rev == o.rev;
}

/**
 * The operator !=
 * @param o the another key
 * @return the result of the checking
 */
inline bool simple_key::operator != (const simple_key& o) const
{
    return !(*this == o);
}

/**
 * Pack the key to the buffer
 * @param out the pointer to data of the key
 * @return the pointer to data of the next key
 */
inline void* simple_key::pack(void *out) const
{
    char *buf = static_cast<char *>(out);
    memcpy(buf, &key, sizeof(key));
    buf += sizeof(key);
    memcpy(buf, &pos, sizeof(pos));
    buf += sizeof(pos);
    memcpy(buf, &beg, sizeof(beg));
    buf += sizeof(beg);
    memcpy(buf, &end, sizeof(end));
    buf += sizeof(end);
    memcpy(buf, &count, sizeof(count));
    buf += sizeof(count);
    memcpy(buf, &rev, sizeof(rev));
    buf += sizeof(rev);
    return buf;
}

/**
 * Unpack the key from the buffer
 * @param in the pointer to data of the key
 * @return the pointer to data of the next key
 */
inline const void* simple_key::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(in);
    memcpy(&key, buf, sizeof(key));
    buf += sizeof(key);
    memcpy(&pos, buf, sizeof(pos));
    buf += sizeof(pos);
    memcpy(&beg, buf, sizeof(beg));
    buf += sizeof(beg);
    memcpy(&end, buf, sizeof(end));
    buf += sizeof(end);
    memcpy(&count, buf, sizeof(count));
    buf += sizeof(count);
    memcpy(&rev, buf, sizeof(rev));
    buf += sizeof(rev);
    return buf;
}

/**
 * Get the size of the packed key
 * @return the size of the packed key
 */
inline size_type simple_key::size() const
{
    return static_size();
}

/**
 * Check the key is valid
 * @return the result of the checking
 */
inline bool simple_key::valid() const
{
    return !(0 == count && beg != end);
}

/**
 * Output the key
 * @param s the stream
 * @param data the key
 * @return the stream
 */
inline std::ostream& operator << (std::ostream& s, const simple_key& data)
{
    s   << "key = " << data.key
        << ", pos = " << data.pos
        << ", beg = " << data.beg
        << ", end = " << data.end
        << ", count = " << data.count
        << ", rev = " << data.rev;
    return s;
}

}   //namespace ouroboros


#endif	/* OUROBOROS_KEY_H */

