#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sharedlocker_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/sharedlocker.h"

BOOST_AUTO_TEST_CASE(simple_test)
{
    ouroboros::mutex_lock locker("locktest");
    BOOST_REQUIRE(locker.lock());
    BOOST_REQUIRE(!locker.lock());
    BOOST_REQUIRE(!locker.lock_sharable());
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(!locker.lock());
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());
}