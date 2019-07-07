/**
 * @file   index.h
 * The indexes for records
 */

#ifndef OUROBOROS_INDEX_H
#define	OUROBOROS_INDEX_H

namespace ouroboros
{

/**
 * The class for undefined index
 */
template <typename T>
class index_null{};

/**
 * The class for the index of field number 1
 */
template <typename Record>
class index1
{
public:
    typedef Record record_type;
    typedef typename record_type::field1_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field1();
    }
};

/**
 * The class for the index of field number 2
 */
template <typename Record>
class index2
{
public:
    typedef Record record_type;
    typedef typename record_type::field2_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field2();
    }
};

/**
 * The class for the index of field number 3
 */
template <typename Record>
class index3
{
public:
    typedef Record record_type;
    typedef typename record_type::field3_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field3();
    }
};

/**
 * The class for the index of field number 4
 */
template <typename Record>
class index4
{
public:
    typedef Record record_type;
    typedef typename record_type::field4_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field4();
    }
};

/**
 * The class for the index of field number 5
 */
template <typename Record>
class index5
{
public:
    typedef Record record_type;
    typedef typename record_type::field5_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field5();
    }
};

/**
 * The class for the index of field number 6
 */
template <typename Record>
class index6
{
public:
    typedef Record record_type;
    typedef typename record_type::field6_type field_type;
    static inline const field_type value(const record_type& record)
    {
        return record.field6();
    }
};

}   //namespace ouroboros


#endif	/* OUROBOROS_INDEX_H */

