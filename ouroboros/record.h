/**
 * @file   record.h
 * The records of data of data
 */

#ifndef OUROBOROS_RECORD_H
#define	OUROBOROS_RECORD_H

#include <stddef.h>
#include <ostream>
#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The record of data that has 1 field
 */
template <typename F1>
class record1
{
    typedef record1<F1> record_type;
public:
    typedef typename F1::data_type field1_type;

    enum {COUNT = 1};

    record1();
    record1(const record_type& record);
    record1(const field1_type& field1);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field1_type field1() const;
    inline void field1(const field1_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F1 m_field1;
};

/**
 * The record of data that has 2 fields
 */
template <typename F1, typename F2>
class record2 : public record1<F1>
{
    typedef record2<F1, F2> record_type;
    typedef record1<F1> base_class;
public:
    typedef typename F1::data_type field1_type;
    typedef typename F2::data_type field2_type;

    enum {COUNT = base_class::COUNT + 1};

    record2();
    record2(const record_type& record);
    record2(const field1_type& field1, const field2_type& field2);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field2_type field2() const;
    inline void field2(const field2_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F2 m_field2;
};

/**
 * The record of data that has 3 fields
 */
template <typename F1, typename F2, typename F3>
class record3 : public record2<F1, F2>
{
    typedef record3<F1, F2, F3> record_type;
    typedef record2<F1, F2> base_class;
public:
    typedef typename F1::data_type field1_type;
    typedef typename F2::data_type field2_type;
    typedef typename F3::data_type field3_type;

    enum {COUNT = base_class::COUNT + 1};

    record3();
    record3(const record_type& record);
    record3(const field1_type& field1, const field2_type& field2, const field3_type& field3);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field3_type field3() const;
    inline void field3(const field3_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F3 m_field3;
};

/**
 * The record of data that has 4 fields
 */
template <typename F1, typename F2, typename F3, typename F4>
class record4 : public record3<F1, F2, F3>
{
    typedef record4<F1, F2, F3, F4> record_type;
    typedef record3<F1, F2, F3> base_class;
public:
    typedef typename F1::data_type field1_type;
    typedef typename F2::data_type field2_type;
    typedef typename F3::data_type field3_type;
    typedef typename F4::data_type field4_type;

    enum {COUNT = base_class::COUNT + 1};

    record4();
    record4(const record_type& record);
    record4(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field4_type field4() const;
    inline void field4(const field4_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F4 m_field4;
};

/**
 * The record of data that has 5 fields
 */
template <typename F1, typename F2, typename F3, typename F4, typename F5>
class record5 : public record4<F1, F2, F3, F4>
{
    typedef record5<F1, F2, F3, F4, F5> record_type;
    typedef record4<F1, F2, F3, F4> base_class;
public:
    typedef typename F1::data_type field1_type;
    typedef typename F2::data_type field2_type;
    typedef typename F3::data_type field3_type;
    typedef typename F4::data_type field4_type;
    typedef typename F5::data_type field5_type;

    enum {COUNT = base_class::COUNT + 1};

    record5();
    record5(const record_type& record);
    record5(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4, const field5_type& field5);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field5_type field5() const;
    inline void field5(const field5_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F5 m_field5;
};

/**
 * The record of data that has 6 fields
 */
template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
class record6 : public record5<F1, F2, F3, F4, F5>
{
    typedef record6<F1, F2, F3, F4, F5, F6> record_type;
    typedef record5<F1, F2, F3, F4, F5> base_class;
public:
    typedef typename F1::data_type field1_type;
    typedef typename F2::data_type field2_type;
    typedef typename F3::data_type field3_type;
    typedef typename F4::data_type field4_type;
    typedef typename F5::data_type field5_type;
    typedef typename F6::data_type field6_type;

    enum {COUNT = base_class::COUNT + 1};

    record6();
    record6(const record_type& record);
    record6(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4, const field5_type& field5, const field6_type& field6);
    inline void *pack(void *out) const;
    inline const void *unpack(const void *in);
    inline size_type size() const;
    inline const field6_type field6() const;
    inline void field6(const field6_type& field);

    inline bool operator == (const record_type& o) const;
    inline bool operator != (const record_type& o) const;

    static size_type static_size();
private:
    F6 m_field6;
};

//==============================================================================
//  record1
//==============================================================================
template <typename F1>
record1<F1>::record1() :
    m_field1()
{
}

template <typename F1>
record1<F1>::record1(const record_type& record) :
    m_field1(record.m_field1)
{
}

template <typename F1>
record1<F1>::record1(const field1_type& field1) :
    m_field1(field1)
{
}

template <typename F1>
inline void *record1<F1>::pack(void *out) const
{
    return m_field1.pack(out);
}

template <typename F1>
inline const void *record1<F1>::unpack(const void *in)
{
    return m_field1.unpack(in);
}

template <typename F1>
inline size_type record1<F1>::size() const
{
    return m_field1.size();
}

//static
template <typename F1>
size_type record1<F1>::static_size()
{
    return F1::static_size();
}

template <typename F1>
inline void record1<F1>::field1(const field1_type& field)
{
    m_field1.data(field);
}

template <typename F1>
inline const typename record1<F1>::field1_type record1<F1>::field1() const
{
    return m_field1.data();
}

template <typename F1>
inline bool record1<F1>::operator == (const record_type& o) const
{
    return field1() == o.field1();
}

template <typename F1>
inline bool record1<F1>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1>
std::ostream& operator << (std::ostream& s, const record1<F1>& record)
{
    s << record.field1();
    return s;
}

template <typename F1>
std::istream& operator >> (std::istream& s, record1<F1>& record)
{
    typename F1::data_type field;
    s >> field;
    record.field1(field);
    return s;
}

//==============================================================================
//  record2
//==============================================================================
template <typename F1, typename F2>
record2<F1, F2>::record2() :
    record1<F1>(),
    m_field2()
{
}

template <typename F1, typename F2>
record2<F1, F2>::record2(const record_type& record) :
    record1<F1>(record),
    m_field2(record.m_field2)
{
}

template <typename F1, typename F2>
record2<F1, F2>::record2(const field1_type& field1, const field2_type& field2) :
    record1<F1>(field1),
    m_field2(field2)
{
}

template <typename F1, typename F2>
inline void *record2<F1, F2>::pack(void *out) const
{
    return m_field2.pack(base_class::pack(out));
}

template <typename F1, typename F2>
inline const void *record2<F1, F2>::unpack(const void *in)
{
    return m_field2.unpack(base_class::unpack(in));
}

template <typename F1, typename F2>
inline size_type record2<F1, F2>::size() const
{
    return base_class::size() + m_field2.size();
}

//static
template <typename F1, typename F2>
size_type record2<F1, F2>::static_size()
{
    return base_class::static_size() + F2::static_size();
}

template <typename F1, typename F2>
inline void record2<F1, F2>::field2(const field2_type& field)
{
    m_field2.data(field);
}

template <typename F1, typename F2>
inline const typename record2<F1, F2>::field2_type record2<F1, F2>::field2() const
{
    return m_field2.data();
}

template <typename F1, typename F2>
inline bool record2<F1, F2>::operator == (const record_type& o) const
{
    return base_class::field1() == o.field1() && field2() == o.field2();
}

template <typename F1, typename F2>
inline bool record2<F1, F2>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1, typename F2>
std::ostream& operator << (std::ostream& s, const record2<F1, F2>& record)
{
    s << static_cast<const record1<F1>& >(record) << ", " << record.field2();
    return s;
}

template <typename F1, typename F2>
std::istream& operator >> (std::istream& s, record2<F1, F2>& record)
{
    s >> static_cast< record1<F1>& >(record);
    std::string delim;
    s >> delim;
    typename F2::data_type field;
    s >> field;
    record.field2(field);
    return s;
}

//==============================================================================
//  record3
//==============================================================================
template <typename F1, typename F2, typename F3>
record3<F1, F2, F3>::record3() :
    record2<F1, F2>(),
    m_field3()
{
}

template <typename F1, typename F2, typename F3>
record3<F1, F2, F3>::record3(const record_type& record) :
    record2<F1, F2>(record),
    m_field3(record.m_field3)
{
}

template <typename F1, typename F2, typename F3>
record3<F1, F2, F3>::record3(const field1_type& field1, const field2_type& field2, const field3_type& field3) :
    record2<F1, F2>(field1, field2),
    m_field3(field3)
{
}

template <typename F1, typename F2, typename F3>
inline void *record3<F1, F2, F3>::pack(void *out) const
{
    return m_field3.pack(base_class::pack(out));
}

template <typename F1, typename F2, typename F3>
inline const void *record3<F1, F2, F3>::unpack(const void *in)
{
    return m_field3.unpack(base_class::unpack(in));
}

template <typename F1, typename F2, typename F3>
inline size_type record3<F1, F2, F3>::size() const
{
    return base_class::size() + m_field3.size();
}

//static
template <typename F1, typename F2, typename F3>
size_type record3<F1, F2, F3>::static_size()
{
    return base_class::static_size() + F3::static_size();
}

template <typename F1, typename F2, typename F3>
inline void record3<F1, F2, F3>::field3(const field3_type& field)
{
    m_field3.data(field);
}

template <typename F1, typename F2, typename F3>
inline const typename record3<F1, F2, F3>::field3_type record3<F1, F2, F3>::field3() const
{
    return m_field3.data();
}

template <typename F1, typename F2, typename F3>
inline bool record3<F1, F2, F3>::operator == (const record_type& o) const
{
    return base_class::field1() == o.field1()
        && base_class::field2() == o.field2()
        && field3() == o.field3();
}

template <typename F1, typename F2, typename F3>
inline bool record3<F1, F2, F3>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1, typename F2, typename F3>
std::ostream& operator << (std::ostream& s, const record3<F1, F2, F3>& record)
{
    s << static_cast<const record2<F1, F2>& >(record) << ", " << record.field3();
    return s;
}

template <typename F1, typename F2, typename F3>
std::istream& operator >> (std::istream& s, record3<F1, F2, F3>& record)
{
    s >> static_cast< record2<F1, F2>& >(record);
    std::string delim;
    s >> delim;
    typename F3::data_type field;
    s >> field;
    record.field3(field);
    return s;
}

//==============================================================================
//  record4
//==============================================================================
template <typename F1, typename F2, typename F3, typename F4>
record4<F1, F2, F3, F4>::record4() :
    record3<F1, F2, F3>(),
    m_field4()
{
}

template <typename F1, typename F2, typename F3, typename F4>
record4<F1, F2, F3, F4>::record4(const record_type& record) :
    record3<F1, F2, F3>(record),
    m_field4(record.m_field4)
{
}

template <typename F1, typename F2, typename F3, typename F4>
record4<F1, F2, F3, F4>::record4(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4) :
    record3<F1, F2, F3>(field1, field2, field3),
    m_field4(field4)
{
}

template <typename F1, typename F2, typename F3, typename F4>
inline void *record4<F1, F2, F3, F4>::pack(void *out) const
{
    return m_field4.pack(base_class::pack(out));
}

template <typename F1, typename F2, typename F3, typename F4>
inline const void *record4<F1, F2, F3, F4>::unpack(const void *in)
{
    return m_field4.unpack(base_class::unpack(in));
}

template <typename F1, typename F2, typename F3, typename F4>
inline size_type record4<F1, F2, F3, F4>::size() const
{
    return base_class::size() + m_field4.size();
}

//static
template <typename F1, typename F2, typename F3, typename F4>
size_type record4<F1, F2, F3, F4>::static_size()
{
    return base_class::static_size() + F4::static_size();
}

template <typename F1, typename F2, typename F3, typename F4>
inline void record4<F1, F2, F3, F4>::field4(const field4_type& field)
{
    m_field4.data(field);
}

template <typename F1, typename F2, typename F3, typename F4>
inline const typename record4<F1, F2, F3, F4>::field4_type record4<F1, F2, F3, F4>::field4() const
{
    return m_field4.data();
}

template <typename F1, typename F2, typename F3, typename F4>
inline bool record4<F1, F2, F3, F4>::operator == (const record_type& o) const
{
    return base_class::field1() == o.field1()
        && base_class::field2() == o.field2()
        && base_class::field3() == o.field3()
        && field4() == o.field4();
}

template <typename F1, typename F2, typename F3, typename F4>
inline bool record4<F1, F2, F3, F4>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1, typename F2, typename F3, typename F4>
std::ostream& operator << (std::ostream& s, const record4<F1, F2, F3, F4>& record)
{
    s << static_cast<const record3<F1, F2, F3>& >(record) << ", " << record.field4();
    return s;
}

template <typename F1, typename F2, typename F3, typename F4>
std::istream& operator >> (std::istream& s, record4<F1, F2, F3, F4>& record)
{
    s >> static_cast< record3<F1, F2, F3>& >(record);
    std::string delim;
    s >> delim;
    typename F4::data_type field;
    s >> field;
    record.field4(field);
    return s;
}

//==============================================================================
//  record5
//==============================================================================
template <typename F1, typename F2, typename F3, typename F4, typename F5>
record5<F1, F2, F3, F4, F5>::record5() :
    record4<F1, F2, F3, F4>(),
    m_field5()
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
record5<F1, F2, F3, F4, F5>::record5(const record_type& record) :
    record4<F1, F2, F3, F4>(record),
    m_field5(record.m_field5)
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
record5<F1, F2, F3, F4, F5>::record5(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4, const field5_type& field5) :
    record4<F1, F2, F3, F4>(field1, field2, field3, field4),
    m_field5(field5)
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline void *record5<F1, F2, F3, F4, F5>::pack(void *out) const
{
    return m_field5.pack(base_class::pack(out));
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline const void *record5<F1, F2, F3, F4, F5>::unpack(const void *in)
{
    return m_field5.unpack(base_class::unpack(in));
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline size_type record5<F1, F2, F3, F4, F5>::size() const
{
    return base_class::size() + m_field5.size();
}

//static
template <typename F1, typename F2, typename F3, typename F4, typename F5>
size_type record5<F1, F2, F3, F4, F5>::static_size()
{
    return base_class::static_size() + F5::static_size();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline void record5<F1, F2, F3, F4, F5>::field5(const field5_type& field)
{
    m_field5.data(field);
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline const typename record5<F1, F2, F3, F4, F5>::field5_type record5<F1, F2, F3, F4, F5>::field5() const
{
    return m_field5.data();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline bool record5<F1, F2, F3, F4, F5>::operator == (const record_type& o) const
{
    return base_class::field1() == o.field1()
        && base_class::field2() == o.field2()
        && base_class::field3() == o.field3()
        && base_class::field4() == o.field4()
        && field5() == o.field5();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
inline bool record5<F1, F2, F3, F4, F5>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
std::ostream& operator << (std::ostream& s, const record5<F1, F2, F3, F4, F5>& record)
{
    s << static_cast<const record4<F1, F2, F3, F4>& >(record) << ", " << record.field5();
    return s;
}

template <typename F1, typename F2, typename F3, typename F4, typename F5>
std::istream& operator >> (std::istream& s, record5<F1, F2, F3, F4, F5>& record)
{
    s >> static_cast< record4<F1, F2, F3, F4>& >(record);
    std::string delim;
    s >> delim;
    typename F5::data_type field;
    s >> field;
    record.field5(field);
    return s;
}

//==============================================================================
//  record6
//==============================================================================
template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
record6<F1, F2, F3, F4, F5, F6>::record6() :
    record5<F1, F2, F3, F4, F5>(),
    m_field6()
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
record6<F1, F2, F3, F4, F5, F6>::record6(const record_type& record) :
    record5<F1, F2, F3, F4, F5>(record),
    m_field6(record.m_field6)
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
record6<F1, F2, F3, F4, F5, F6>::record6(const field1_type& field1, const field2_type& field2, const field3_type& field3, const field4_type& field4, const field5_type& field5, const field6_type& field6) :
    record5<F1, F2, F3, F4, F5>(field1, field2, field3, field4, field5),
    m_field6(field6)
{
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline void *record6<F1, F2, F3, F4, F5, F6>::pack(void *out) const
{
    return m_field6.pack(base_class::pack(out));
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline const void *record6<F1, F2, F3, F4, F5, F6>::unpack(const void *in)
{
    return m_field6.unpack(base_class::unpack(in));
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline size_type record6<F1, F2, F3, F4, F5, F6>::size() const
{
    return base_class::size() + m_field6.size();
}

//static
template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
size_type record6<F1, F2, F3, F4, F5, F6>::static_size()
{
    return base_class::static_size() + F6::static_size();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline void record6<F1, F2, F3, F4, F5, F6>::field6(const field6_type& field)
{
    m_field6.data(field);
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline const typename record6<F1, F2, F3, F4, F5, F6>::field6_type record6<F1, F2, F3, F4, F5, F6>::field6() const
{
    return m_field6.data();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline bool record6<F1, F2, F3, F4, F5, F6>::operator == (const record_type& o) const
{
    return base_class::field1() == o.field1()
        && base_class::field2() == o.field2()
        && base_class::field3() == o.field3()
        && base_class::field4() == o.field4()
        && base_class::field5() == o.field5()
        && field6() == o.field6();
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
inline bool record6<F1, F2, F3, F4, F5, F6>::operator != (const record_type& o) const
{
    return !(*this == o);
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
std::ostream& operator << (std::ostream& s, const record6<F1, F2, F3, F4, F5, F6>& record)
{
    s << static_cast<const record5<F1, F2, F3, F4, F5>& >(record) << ", " << record.field6();
    return s;
}

template <typename F1, typename F2, typename F3, typename F4, typename F5, typename F6>
std::istream& operator >> (std::istream& s, record6<F1, F2, F3, F4, F5, F6>& record)
{
    s >> static_cast< record5<F1, F2, F3, F4, F5>& >(record);
    std::string delim;
    s >> delim;
    typename F6::data_type field;
    s >> field;
    record.field6(field);
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_RECORD_H */

