#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE key_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/key.h"

BOOST_AUTO_TEST_CASE(simple_test)
{
    ouroboros::simple_key key;
    BOOST_REQUIRE(key.valid());
    key.count = 0;
    key.end = 1;
    BOOST_REQUIRE(!key.valid());
}
