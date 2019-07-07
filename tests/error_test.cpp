#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE error_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/error.h"

template <typename T>
void error_test()
{
    const std::string message = "test error";
    T error;
    error << message;
    BOOST_REQUIRE_EQUAL(message, error.what());
    const ouroboros::base_exception& except = error;
    BOOST_REQUIRE_EQUAL(message, except.what());
}

BOOST_AUTO_TEST_CASE(simple_test)
{
    error_test<ouroboros::bug_error>();
    error_test<ouroboros::base_error>();
    error_test<ouroboros::range_error>();
    error_test<ouroboros::compatibility_error>();
    error_test<ouroboros::version_error>();
    error_test<ouroboros::lock_error>();
}
