/**
 * @file   treekey.h
 * The key of a tree table
 */

#ifndef OUROBOROS_TREEKEY_H
#define	OUROBOROS_TREEKEY_H

#include "ouroboros/key.h"

namespace ouroboros
{

/**
 * The key of a tree table
 */
class tree_key : public simple_key
{
    friend inline std::ostream& operator << (std::ostream& s, const tree_key& data);
public:
    typedef simple_key::key_type key_type;
    inline tree_key();
    inline tree_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev);
    inline tree_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev, const pos_type aroot);

    inline bool operator == (const tree_key& o) const;
    inline bool operator != (const tree_key& o) const;

    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline bool valid() const;

    static size_type static_size()
    {
        return simple_key::static_size() + sizeof(pos_type);
    }

public:
    enum {COUNT = simple_key::COUNT + 1};
    pos_type root;
};

//==============================================================================
//  tree_key
//==============================================================================
/**
 * Constructor
 */
inline tree_key::tree_key() :
    simple_key(),
    root(NIL)
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
inline tree_key::tree_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev) :
    simple_key(akey, apos, abeg, aend, acount, arev),
    root(NIL)
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
 * @param aroot the position of the root
 */
inline tree_key::tree_key(const key_type akey, const spos_type apos, const pos_type abeg,
        const pos_type aend, const count_type acount, const revision_type arev, const pos_type aroot) :
    simple_key(akey, apos, abeg, aend, acount, arev),
    root(aroot)
{
}

/**
 * The operator ==
 * @param o the another key
 * @return the result of the checking
 */
inline bool tree_key::operator == (const tree_key& o) const
{
    return simple_key::operator==(o) && root == o.root;
}

/**
 * The operator !=
 * @param o the another key
 * @return the result of the checking
 */
inline bool tree_key::operator != (const tree_key& o) const
{
    return !(*this == o);
}

/**
 * Pack the key to the buffer
 * @param out the pointer to data of the key
 * @return the pointer to data of the next key
 */
inline void* tree_key::pack(void *out) const
{
    char *buf = static_cast<char *>(simple_key::pack(out));
    memcpy(buf, &root, sizeof(root));
    buf += sizeof(root);
    return buf;
}

/**
 * Unpack the key from the buffer
 * @param in the pointer to data of the key
 * @return the pointer to data of the next key
 */
inline const void* tree_key::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(simple_key::unpack(in));
    memcpy(&root, buf, sizeof(root));
    buf += sizeof(root);
    return buf;
}

/**
 * Get the size of the packed key
 * @return the size of the packed key
 */
inline size_type tree_key::size() const
{
    return static_size();
}

/**
 * Check the key is valid
 * @return the result of the checking
 */
inline bool tree_key::valid() const
{
    return simple_key::valid() && ((count > 0 && root != NIL) || (0 == count && NIL == root));
}

/**
 * Output the key
 * @param s the stream
 * @param data the key
 * @return the stream
 */
inline std::ostream& operator << (std::ostream& s, const tree_key& data)
{
    s << dynamic_cast<const tree_key::simple_key&>(data) << ", root = " << data.root;
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_TREEKEY_H */

