/**
 * @file   indexedrecord.h
 * The indexed record
 */

#ifndef OUROBOROS_INDEXEDRECORD_H
#define	OUROBOROS_INDEXEDRECORD_H

#include <string.h>

#include "ouroboros/node.h"
#include "ouroboros/find.h"
#include "ouroboros/record.h"
#include "ouroboros/index.h"

namespace ouroboros
{

/**
 * The indexed record
 */
template <typename Record, template <typename> class Index>
class indexed_record : public Record
{
    typedef indexed_record<Record, Index> self_type;
    typedef typename Index<Record>::field_type key_type;
    struct converter
    {
        inline const key_type key(const Record& record) const
        {
            return Index<Record>::value(record);
        }
    };
public:
    typedef Record record_type;
    typedef typename record_type::field1_type field1_type;
    typedef Index<record_type> index_type;
    typedef data_node<key_type, record_type, converter> node_type;

    inline indexed_record();
    inline indexed_record(const node_type& node);

    inline const bool operator == (const indexed_record& o) const;
    inline const bool operator != (const indexed_record& o) const;

    inline const bool operator == (const record_type& record) const;
    inline const bool operator != (const record_type& record) const;

    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline const size_type size() const;

    ///@todo operator () replace to operator *()
    inline node_type operator() ();
    inline const node_type operator() () const;
    inline void operator() (const node_type& node);

    static const size_type static_size();
private:
    pos_type   m_parent;
    pos_type   m_left;
    pos_type   m_right;
    node_color m_color;
};

//==============================================================================
//  indexed_record
//==============================================================================
/**
 * Constructor
 */
template <typename Record, template <typename> class Index>
inline indexed_record<Record, Index>::indexed_record() :
    record_type(),
    m_parent(NIL),
    m_left(NIL),
    m_right(NIL),
    m_color(BLACK)
{
}

/**
 * Constructor
 * @param node the node
 */
template <typename Record, template <typename> class Index>
inline indexed_record<Record, Index>::indexed_record(const node_type& node) :
    record_type(node.body()),
    m_parent(node.parent()),
    m_left(node.left()),
    m_right(node.right()),
    m_color(node.color())
{
}

/**
 * Operator ==
 * @param o the another indexed record
 * @return the result of the checking
 */
template <typename Record, template <typename> class Index>
inline const bool indexed_record<Record, Index>::operator == (const indexed_record& o) const
{
    return record_type::operator==(o);
}

/**
 * Operator !=
 * @param o the another indexed record
 * @return the result of the checking
 */
template <typename Record, template <typename> class Index>
inline const bool indexed_record<Record, Index>::operator != (const indexed_record& o) const
{
    return !(*this == o);
}

/**
 * Operator ==
 * @param record the another record
 * @return the result of the checking
 */
template <typename Record, template <typename> class Index>
inline const bool indexed_record<Record, Index>::operator == (const record_type& record) const
{
    return record_type::operator==(record);
}

/**
 * Operator !=
 * @param record the another record
 * @return the result of the checking
 */
template <typename Record, template <typename> class Index>
inline const bool indexed_record<Record, Index>::operator != (const record_type& record) const
{
    return !(*this == record);
}

/**
 * Pack the record to the buffer
 * @param out the pointer to data of the record
 * @return the pointer to data of the next record
 */
template <typename Record, template <typename> class Index>
inline void * indexed_record<Record, Index>::pack(void *out) const
{
    char *buf = static_cast<char *>(out);
    memcpy(buf, &m_parent, sizeof(m_parent));
    buf += sizeof(m_parent);

    memcpy(buf, &m_left, sizeof(m_left));
    buf += sizeof(m_left);

    memcpy(buf, &m_right, sizeof(m_right));
    buf += sizeof(m_right);

    memcpy(buf, &m_color, sizeof(m_color));
    buf += sizeof(m_color);

    return record_type::pack(buf);
}

/**
 * Unpack the record from the buffer
 * @param in the pointer to data of the record
 * @return the pointer to data of the next record
 */
template <typename Record, template <typename> class Index>
inline const void * indexed_record<Record, Index>::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(in);
    memcpy(&m_parent, buf, sizeof(m_parent));
    buf += sizeof(m_parent);

    memcpy(&m_left, buf, sizeof(m_left));
    buf += sizeof(m_left);

    memcpy(&m_right, buf, sizeof(m_right));
    buf += sizeof(m_right);

    memcpy(&m_color, buf, sizeof(m_color));
    buf += sizeof(m_color);

    return record_type::unpack(buf);
}

/**
 * Get the size of the record
 * @return the size of the record
 */
template <typename Record, template <typename> class Index>
inline const size_type indexed_record<Record, Index>::size() const
{
    return sizeof(m_parent) + sizeof(m_left) + sizeof(m_right) +
        sizeof(m_color) + record_type::size();
}

/**
 * Get the size of the record
 * @return the size of the record
 */
//static
template <typename Record, template <typename> class Index>
const size_type indexed_record<Record, Index>::static_size()
{
    return sizeof(pos_type) + sizeof(pos_type) + sizeof(pos_type) +
        sizeof(node_color) + record_type::static_size();
}

/**
 * Get the node from the record
 * @return the node
 */
template <typename Record, template <typename> class Index>
inline typename indexed_record<Record, Index>::node_type
    indexed_record<Record, Index>::operator() ()
{
    node_type node(*this, m_parent, m_color);
    node.left(m_left);
    node.right(m_right);
    return node;
}

/**
 * Get the node from the record
 * @return the node
 */
template <typename Record, template <typename> class Index>
inline const typename indexed_record<Record, Index>::node_type
    indexed_record<Record, Index>::operator() () const
{
    node_type node(*this, m_parent, m_color);
    node.left(m_left);
    node.right(m_right);
    return node;
}

/**
 * Set the node to the record
 * @param node the node
 */
template <typename Record, template <typename> class Index>
inline void indexed_record<Record, Index>::operator() (const node_type& node)
{
    m_parent = node.parent();
    m_left = node.left();
    m_right = node.right();
    m_color = node.color();
    record_type::operator=(node.body());
}

/**
 * Output the record
 * @param s the stream
 * @param record the record
 * @return the stream
 */
template <typename Record, template <typename> class Index>
inline std::ostream& operator << (std::ostream& s, const indexed_record<Record, Index>& record)
{
    s << record();
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_INDEXEDRECORD_H */

