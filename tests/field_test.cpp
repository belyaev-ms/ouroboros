#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE field_test
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>
#include "ouroboros/field.h"

typedef ouroboros::string_field<6> field_type;
const std::string str("1234567890");
//==============================================================================
//  Check the value setting through the constructor
//==============================================================================
BOOST_AUTO_TEST_CASE(str_field_construct_test)
{
    field_type field(str);
    std::vector<char> buffer(100, 'x');
    void *ptr = field.pack(&buffer[0]);
    BOOST_REQUIRE_EQUAL(ptr, &buffer[field_type::static_size()]);
    BOOST_REQUIRE_EQUAL(field.data(), "12345");
    size_t i;
    for (i = 0; i < field_type::static_size() - 1; ++i)
    {
        BOOST_REQUIRE_EQUAL(buffer[i], str[i]);
    }
    BOOST_REQUIRE_EQUAL(buffer[i++], 0);
    for (; i < buffer.size(); ++i)
    {
        BOOST_REQUIRE_EQUAL(buffer[i], 'x');
    }
}

//==============================================================================
//  Check the value setting through the method
//==============================================================================
BOOST_AUTO_TEST_CASE(str_field_method_test)
{
    field_type field;
    field.data(str);
    std::vector<char> buffer(100, 'x');
    void *ptr = field.pack(&buffer[0]);
    BOOST_REQUIRE_EQUAL(ptr, &buffer[field_type::static_size()]);
    BOOST_REQUIRE_EQUAL(field.data(), "12345");
    size_t i;
    for (i = 0; i < field_type::static_size() - 1; ++i)
    {
        BOOST_REQUIRE_EQUAL(buffer[i], str[i]);
    }
    BOOST_REQUIRE_EQUAL(buffer[i++], 0);
    for (; i < buffer.size(); ++i)
    {
        BOOST_REQUIRE_EQUAL(buffer[i], 'x');
    }
}

//==============================================================================
//  Check for the string_field
//==============================================================================
BOOST_AUTO_TEST_CASE(str_field_test)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        BOOST_TEST_MESSAGE(PE(i));
        field_type field1(str.substr(0, i));
        std::vector<char> buffer(100, 'x');
        field1.pack(&buffer[0]);
        field_type field2;
        field2.unpack(&buffer[0]);
        BOOST_REQUIRE_EQUAL(field1.data(), field2.data());
    }
}

