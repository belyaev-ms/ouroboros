/**
 * @file   find.h
 * The classes for searching records in a table
 */

#ifndef OUROBOROS_FIND_H
#define	OUROBOROS_FIND_H

#include "ouroboros/record.h"
#include "ouroboros/container.h"

namespace ouroboros
{

/**
 * The class for comparing a field with a value
 */
template <typename Record, template <typename> class Index, template <typename> class Operator>
class value_comparator
{
public:
    typedef Record record_type;
    typedef Index<record_type> index_type;
    typedef typename index_type::field_type field_type;
    typedef Operator<field_type> operator_type;

    explicit value_comparator(const field_type& field);
    inline bool operator ()(const record_type& record) const;
private:
    value_comparator();
    field_type m_field;
};

/**
 * The class for comparing two fields
 */
template <typename Record, template <typename> class Index1, template <typename> class Index2,
        template <typename> class Operator>
struct fields_comparator
{
    typedef Record record_type;
    typedef Index1<record_type> index1_type;
    typedef Index2<record_type> index2_type;
    typedef typename index1_type::field_type field1_type;
    typedef typename index2_type::field_type field2_type;
    typedef Operator<field1_type> operator_type;

    inline bool operator ()(const record_type& record) const;
};

/**
 * The class for combining two comparator
 */
template <typename Comparator1, typename Comparator2, template <typename> class Operator>
class combiner
{
public:
    typedef Operator<bool> operator_type;
    typedef Comparator1 comparator1_type;
    typedef Comparator2 comparator2_type;
    typedef typename comparator1_type::record_type record_type;

    combiner(const comparator1_type& comp1, const comparator2_type& comp2);
    inline bool operator ()(const record_type& record) const;
private:
    const comparator1_type m_comp1;
    const comparator2_type m_comp2;
};

/**
 * The class for searching records in a table
 */
template <typename Comparator, template <typename> class Container = vector>
class finder
{
public:
    typedef Comparator comparator_type;
    typedef typename comparator_type::record_type record_type;
    typedef typename Container<record_type>::vector_type result_type;

    explicit finder(const comparator_type& comp);
    finder(const comparator_type& comp, const count_type count);

    inline bool operator()(); ///< compare
    inline record_type& record(const pos_type pos); ///< get the buffer of a record
    inline const result_type& result() const; ///< get the search result
    inline pos_type pos() const; ///< get the position of the last processed record
    void reset(const count_type count = 0); ///< reset to the first state
protected:
    inline bool push(const record_type& record); ///< push a found record
protected:
    const comparator_type m_comp; ///< the comparator
    record_type m_record; ///< the buffer of a record
    result_type m_result; ///< container for found records
    count_type m_count; ///< the limit of the number of records found
    pos_type m_pos; ///< the position of the last processed record
};

/**
 * Comparator (field == val)
 */
template <typename Record, template <typename> class Index>
class comp_equal : public value_comparator<Record, Index, std::equal_to>
{
    typedef value_comparator<Record, Index, std::equal_to> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_equal(const field_type& field) : base_class(field) {};
};

/**
 * Comparator (field != val)
 */
template <typename Record, template <typename> class Index>
class comp_not_equal : public value_comparator<Record, Index, std::not_equal_to>
{
    typedef value_comparator<Record, Index, std::not_equal_to> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_not_equal(const field_type& field) : base_class(field) {};
};

/**
 * Comparator (field > val)
 */
template <typename Record, template <typename> class Index>
class comp_greater : public value_comparator<Record, Index, std::greater>
{
    typedef value_comparator<Record, Index, std::greater> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_greater(const field_type& field) : base_class(field) {};
};

/**
 * Comparator (field < val)
 */
template <typename Record, template <typename> class Index>
class comp_less : public value_comparator<Record, Index, std::less>
{
    typedef value_comparator<Record, Index, std::less> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_less(const field_type& field) : base_class(field) {};
};

/**
 * Comparator (field >= val)
 */
template <typename Record, template <typename> class Index>
class comp_greater_equal : public value_comparator<Record, Index, std::greater_equal>
{
    typedef value_comparator<Record, Index, std::greater_equal> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_greater_equal(const field_type& field) : base_class(field) {};
};

/**
 * Comparator (field <= val)
 */
template <typename Record, template <typename> class Index>
class comp_less_equal : public value_comparator<Record, Index, std::less_equal>
{
    typedef value_comparator<Record, Index, std::less_equal> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_less_equal(const field_type& field) : base_class(field) {};
};

namespace
{

/** Operator (x & y) == 0 */
template <typename T>
struct not_bit_mask : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return (x & y) == 0;
    }
};

}

/**
 * Comparator (field & val == 0)
 */
template <typename Record, template <typename> class Index>
class comp_not_mask : public value_comparator<Record, Index, not_bit_mask>
{
    typedef value_comparator<Record, Index, not_bit_mask> base_class;
public:
    typedef typename base_class::field_type field_type;
    explicit comp_not_mask(const field_type& field) : base_class(field) {};
};

namespace
{

/** Operator (x && y) */
template <typename T>
struct op_and : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x && y;
    }
};

/** Operator (x || y) */
template <typename T>
struct op_or : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x || y;
    }
};

/** Operator (x > y) */
template <typename T>
struct op_greater : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x > y;
    }
};

/** Operator (x >= y) */
template <typename T>
struct op_greater_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x >= y;
    }
};

/** Operator (x < y) */
template <typename T>
struct op_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x < y;
    }
};

/** Operator (x <= y) */
template <typename T>
struct op_less_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x <= y;
    }
};

}

/** Combiner of two comparators by AND */
template <typename Comparator1, typename Comparator2>
struct comb_and : public combiner<Comparator1, Comparator2, op_and>
{
    comb_and(const Comparator1& comp1, const Comparator2& comp2) :
        combiner<Comparator1, Comparator2, op_and>(comp1, comp2)
    {}
};

/** Combiner of two comparators by OR */
template <typename Comparator1, typename Comparator2>
struct comb_or : public combiner<Comparator1, Comparator2, op_or>
{
    comb_or(const Comparator1& comp1, const Comparator2& comp2) :
        combiner<Comparator1, Comparator2, op_or>(comp1, comp2)
    {}
};

//==============================================================================
//  value_comparator
//==============================================================================
/**
 * Constructor
 * @param field the value of field for comparing
 */
template <typename Record, template <typename> class Index, template <typename> class Operator>
value_comparator<Record, Index, Operator>::value_comparator(const field_type& field) :
    m_field(field)
{
}

/**
 * Operator of comparing
 * @param record the record for the comparing
 * @return result of the comparing
 */
template <typename Record, template <typename> class Index, template <typename> class Operator>
inline bool value_comparator<Record, Index, Operator>::operator ()(const record_type& record) const
{
    const field_type field = index_type::value(record);
    return operator_type()(field, m_field);
}

//==============================================================================
//  fields_comparator
//==============================================================================
/**
 * Operator of comparing
 * @param record the record for the comparing
 * @return result of the comparing
 */
template <typename Record, template <typename> class Index1, template <typename> class Index2,
        template <typename> class Operator>
inline bool fields_comparator<Record, Index1, Index2, Operator>::operator ()(const record_type& record) const
{
    const field1_type field1 = index1_type::value(record);
    const field2_type field2 = index2_type::value(record);
    return operator_type()(field1, field2);
}

//==============================================================================
//  combiner
//==============================================================================
/**
 * Constructor
 * @param comp1 the first comparator
 * @param comp2 the second comparator
 */
template <typename TComparer1, typename TComparer2, template <typename> class Operator>
combiner<TComparer1, TComparer2, Operator>::combiner(const comparator1_type& comp1,
        const comparator2_type& comp2) :
    m_comp1(comp1),
    m_comp2(comp2)
{
}

/**
 * Operator of comparing
 * @param record the record for the comparing
 * @return result of the comparing
 */
template <typename TComparer1, typename TComparer2, template <typename> class Operator>
inline bool combiner<TComparer1, TComparer2, Operator>::operator ()(const record_type& record) const
{
    return operator_type()(m_comp1(record), m_comp2(record));
}

//==============================================================================
//  finder
//==============================================================================
/**
 * Constructor
 * @param comp the comparator
 */
template <typename Comparator, template <typename> class Container>
finder<Comparator, Container>::finder(const comparator_type& comp) :
    m_comp(comp),
    m_count(0),
    m_pos(NIL)
{
}

/**
 * Constructor
 * @param comp the comparator
 * @param count the limit of the number of records found
 */
template <typename Comparator, template <typename> class Container>
finder<Comparator, Container>::finder(const comparator_type& comp,
        const count_type count) :
    m_comp(comp),
    m_count(count),
    m_pos(NIL)
{
    m_result.reserve(count);
}

/**
 * Push a found record
 * @param record data of the found record
 * @return false - the record was added, but the container is full
 */
template <typename Comparator, template <typename> class Container>
inline bool finder<Comparator, Container>::push(const record_type& record)
{
    m_result.push_back(record);
    return (--m_count > 0);
}

/**
 * Compare, and if the result is success, add the found record in the container
 * @return true - continue the searching
 */
template <typename Comparator, template <typename> class Container>
inline bool finder<Comparator, Container>::operator ()()
{
    if (m_comp(m_record))
    {
        return push(m_record);
    }
    return true;
}

/**
 * Get the buffer of a record
 * @param pos the position of the record
 * @return the buffer of a record
 */
template <typename Comparator, template <typename> class Container>
inline typename finder<Comparator, Container>::record_type&
    finder<Comparator, Container>::record(const pos_type pos)
{
    m_pos = pos;
    return m_record;
}

/**
 * Get the search result
 * @return the search result
 */
template <typename Comparator, template <typename> class Container>
inline const typename finder<Comparator, Container>::result_type&
    finder<Comparator, Container>::result() const
{
    return m_result;
}

/**
 * Get the position of the last processed record
 * @return the position of the last processed record
 */
template <typename Comparator, template <typename> class Container>
inline pos_type finder<Comparator, Container>::pos() const
{
    return m_pos;
}

/**
 * Reset to the first state
 */
template <typename Comparator, template <typename> class Container>
void finder<Comparator, Container>::reset(count_type count)
{
    m_count = count;
    m_pos = NIL;
    m_result.clear();
}

}   //namespace ouroboros

#endif	/* OUROBOROS_FIND_H */

