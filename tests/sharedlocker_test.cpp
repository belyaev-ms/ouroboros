#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sharedlocker_test
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include "ouroboros/sharedlocker.h"

typedef boost::mpl::list<ouroboros::mutex_lock, ouroboros::simple_mutex_lock> lock_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_lock_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE(locker.lock());
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock(1));
    BOOST_REQUIRE(locker.unlock());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(simple_lock_sharable_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());
    BOOST_REQUIRE(locker.lock_sharable(1));
    BOOST_REQUIRE(locker.unlock_sharable());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking_lock_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE(locker.lock());
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());

    BOOST_REQUIRE(locker.lock());
    BOOST_REQUIRE(!locker.lock());
    BOOST_REQUIRE(!locker.lock(1));
    BOOST_REQUIRE(!locker.lock_sharable());
    BOOST_REQUIRE(!locker.lock_sharable(1));
    BOOST_REQUIRE(locker.unlock());

    BOOST_REQUIRE(locker.lock(1));
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock_sharable(1));
    BOOST_REQUIRE(locker.unlock_sharable());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking_lock_sharable_test, lock_type, lock_types)
{
    lock_type locker;
    BOOST_REQUIRE(locker.lock());
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());

    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(!locker.lock());
    BOOST_REQUIRE(!locker.lock(1));
    BOOST_REQUIRE(locker.lock_sharable());
    BOOST_REQUIRE(locker.lock_sharable(1));
    BOOST_REQUIRE(locker.unlock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());
    BOOST_REQUIRE(locker.unlock_sharable());

    BOOST_REQUIRE(locker.lock(1));
    BOOST_REQUIRE(locker.unlock());
    BOOST_REQUIRE(locker.lock_sharable(1));
    BOOST_REQUIRE(locker.unlock_sharable());
}
