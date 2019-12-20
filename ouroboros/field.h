/**
 * @file   field.h
 * The fields of data
 */

#ifndef OUROBOROS_FIELD_H
#define	OUROBOROS_FIELD_H

#include <string.h>
#include <string>
#include <iostream>
#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The field for simple data
 */
template <typename T>
class data_field
{
public:
    typedef T data_type;
    data_field();
    explicit data_field(const data_type& data);
    inline void *pack(void *out) const; ///< pack the field to the buffer
    inline const void *unpack(const void *in); ///< unpack the field from the buffer
    inline size_type size() const; ///< get the size of the field
    inline const data_type data() const; ///< get the data of the field
    inline void data(const data_type& adata); ///< set the data of the field
    inline bool operator== (const data_field& o) const;
    inline bool operator!= (const data_field& o) const;
    static size_type static_size(); ///< get the size of the field
private:
    data_type m_data; ///< the data of the field
};

/**
 * The field for string data
 */
template <unsigned int SIZE>
class string_field
{
public:
    typedef std::string data_type;
    string_field();
    explicit string_field(const data_type& adata);
    inline void *pack(void *out) const; ///< pack the field to the buffer
    inline const void *unpack(const void *in); ///< unpack the field from the buffer
    inline size_type size() const; ///< get the size of the field
    inline const data_type data() const; ///< get the data of the field
    inline void data(const data_type& adata); ///< set the data of the field
    inline bool operator== (const string_field& o) const;
    inline bool operator!= (const string_field& o) const;
    static size_type static_size(); ///< get the size of the field
private:
    char m_data[SIZE]; ///< the data of the field
};

//==============================================================================
//  data_field
//==============================================================================
/**
 * Constructor
 */
template <typename T>
data_field<T>::data_field() :
    m_data()
{
}

/**
 * Constructor
 * @param data the data of the field
 */
template <typename T>
data_field<T>::data_field(const data_type& data) :
    m_data(data)
{
}

/**
 * Pack the field to the buffer
 * @param out the pointer to data of the field
 * @return the pointer to data of the next field
 */
template <typename T>
inline void *data_field<T>::pack(void *out) const
{
    char *buf = static_cast<char *>(out);
    memcpy(buf, &m_data, sizeof(m_data));
    buf += sizeof(m_data);
    return buf;
}

/**
 * Unpack the field from the buffer
 * @param in the pointer to data of the field
 * @return the pointer to data of the next field
 */
template <typename T>
inline const void *data_field<T>::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(in);
    memcpy(&m_data, buf, sizeof(m_data));
    buf += sizeof(m_data);
    return buf;
}

/**
 * Get the size of the field
 * @return the size of the field
 */
template <typename T>
inline size_type data_field<T>::size() const
{
    return sizeof(m_data);
}

/**
 * Get the size of the field
 * @return the size of the field
 */
//static
template <typename T>
size_type data_field<T>::static_size()
{
    return sizeof(data_type);
}

/**
 * The operator ==
 * @param o the another field
 * @return the result of the checking
 */
template <typename T>
inline bool data_field<T>::operator== (const data_field& o) const
{
    return m_data == o.m_data;
}

/**
 * The operator !=
 * @param o the another field
 * @return the result of the checking
 */
template <typename T>
inline bool data_field<T>::operator!= (const data_field& o) const
{
    return !(*this == o);
}

/**
 * Get the data of the field
 * @return the data of the field
 */
template <typename T>
inline const typename data_field<T>::data_type data_field<T>::data() const
{
    return m_data;
}

/**
 * Set the data of the field
 * @param adata the data of the field
 */
template <typename T>
inline void data_field<T>::data(const data_type& adata)
{
    m_data = adata;
}

/**
 * Output the field
 * @param s the stream
 * @param field the field
 * @return the stream
 */
template <typename T>
std::ostream& operator << (std::ostream& s, const data_field<T>& field)
{
    s << field.data();
    return s;
}

/**
 * Input the field
 * @param s the stream
 * @param field the field
 * @return the stream
 */
template <typename T>
std::istream& operator >> (std::istream& s, data_field<T>& field)
{
    typename data_field<T>::data_type data;
    s >> data;
    field.data(data);
    return s;
}

//==============================================================================
//  string_field
//==============================================================================
/**
 * Constructor
 */
template <unsigned int SIZE>
string_field<SIZE>::string_field()
{
    memset(m_data, 0, size());
}

/**
 * Constructor
 * @param adata the data of the field
 */
template <unsigned int SIZE>
string_field<SIZE>::string_field(const data_type& adata)
{
    data(adata);
}

/**
 * Pack the field to the buffer
 * @param out the pointer to data of the field
 * @return the pointer to data of the next field
 */
template <unsigned int SIZE>
inline void *string_field<SIZE>::pack(void *out) const
{
    char *buf = static_cast<char *>(out);
    memcpy(buf, m_data, size());
    buf[size() - 1] = 0;
    buf += size();
    return buf;
}

/**
 * Unpack the field from the buffer
 * @param in the pointer to data of the field
 * @return the pointer to data of the next field
 */
template <unsigned int SIZE>
inline const void *string_field<SIZE>::unpack(const void *in)
{
    const char *buf = static_cast<const char *>(in);
    memcpy(m_data, buf, size());
    m_data[size() - 1] = 0;
    buf += size();
    return buf;
}

/**
 * Get the size of the field
 * @return the size of the field
 */
template <unsigned int SIZE>
inline size_type string_field<SIZE>::size() const
{
    return SIZE;
}

/**
 * Get the size of the field
 * @return the size of the field
 */
//static
template <unsigned int SIZE>
size_type string_field<SIZE>::static_size()
{
    return SIZE;
}

/**
 * The operator ==
 * @param o the another field
 * @return the result of the checking
 */
template <unsigned int SIZE>
inline bool string_field<SIZE>::operator== (const string_field& o) const
{
    return !strcmp(m_data, o.m_data);
}

/**
 * The operator !=
 * @param o the another field
 * @return the result of the checking
 */
template <unsigned int SIZE>
inline bool string_field<SIZE>::operator!= (const string_field& o) const
{
    return !(*this == o);
}

/**
 * Get the data of the field
 * @return the data of the field
 */
template <unsigned int SIZE>
inline const typename string_field<SIZE>::data_type string_field<SIZE>::data() const
{
    return std::string(m_data);
}

/**
 * Set the data of the field
 * @param adata the data of the field
 */
template <unsigned int SIZE>
inline void string_field<SIZE>::data(const data_type& adata)
{
    size_type size = adata.size();
    if (size > SIZE - 1)
    {
        size = SIZE - 1;
    }
    memcpy(m_data, adata.c_str(), size);
    m_data[size] = '\0';
}

/**
 * Output the field
 * @param s the stream
 * @param field the field
 * @return the stream
 */
template <unsigned int SIZE>
std::ostream& operator << (std::ostream& s, const string_field<SIZE>& field)
{
    s << field.data();
    return s;
}

/**
 * Input the field
 * @param s the stream
 * @param field the field
 * @return the stream
 */
template <unsigned int SIZE>
std::istream& operator >> (std::istream& s, string_field<SIZE>& field)
{
    typename string_field<SIZE>::data_type data;
    s >> data;
    field.data(data);
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_FIELD_H */

