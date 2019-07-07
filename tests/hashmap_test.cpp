#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE hashmap_test
#include <boost/test/unit_test.hpp>

#include <map>
#include "ouroboros/hashmap.h"

using namespace ouroboros;

typedef hash_map<size_t, size_t, 8> test_map_type;
typedef std::map<size_t, size_t> sample_map_type;

const size_t count = 100;
void require_equal_map(sample_map_type& sample_map, test_map_type& test_map)
{
    for (size_t i = 0; i < count; ++i)
    {
        sample_map_type::iterator sample_it = sample_map.find(i);
        test_map_type::iterator test_it = test_map.find(i);
        BOOST_REQUIRE_EQUAL(sample_it != sample_map.end(), test_it != test_map.end());
        if (sample_it != sample_map.end())
        {
            BOOST_REQUIRE_EQUAL(sample_it->first, test_it->first);
            BOOST_REQUIRE_EQUAL(sample_it->second, test_it->second);
        }
    }
}

BOOST_AUTO_TEST_CASE(stg_simple_test)
{
    test_map_type test_map;
    sample_map_type sample_map;

    // add even values
    for (size_t i = 0; i < count; i += 2)
    {
        test_map.insert(std::make_pair(i, 10 * i));
        sample_map.insert(std::make_pair(i, 10 * i));
        require_equal_map(sample_map, test_map);
    }

    // add odd values
    for (size_t i = 1; i < count; i += 2)
    {
        test_map.insert(std::make_pair(i, 10 * i));
        sample_map.insert(std::make_pair(i, 10 * i));
        require_equal_map(sample_map, test_map);
    }

    // pass through the whole range
    size_t size = 0;
    for (typename test_map_type::const_iterator it = test_map.begin(); it != test_map.end(); ++it)
    {
        ++size;
    }
    BOOST_REQUIRE_EQUAL(test_map.size(), size);

    // delete half
    for (size_t i = 1; i < count / 2; i += 2)
    {
        test_map.erase(i);
        sample_map.erase(i);
        require_equal_map(sample_map, test_map);
    }

    // delete a part from middle of the remaining range
    for (size_t i = 3 * count / 4 ; i < 9 * count / 10; ++i)
    {
        test_map.erase(i);
        sample_map.erase(i);
        require_equal_map(sample_map, test_map);
    }

    // add and delete random values
    for (size_t i = 0; i < 10 * count; ++i)
    {
        int32_t val = std::rand() % count;
        test_map_type::iterator stg_it = test_map.find(val);
        sample_map_type::iterator std_it = sample_map.find(val);
        BOOST_REQUIRE_EQUAL(stg_it != test_map.end(), std_it != sample_map.end());
        if (stg_it != test_map.end() || std_it != sample_map.end())
        {
            test_map.erase(val);
            sample_map.erase(val);
        }
        else
        {
            test_map.insert(std::make_pair(val, 10 * val));
            sample_map.insert(std::make_pair(val, 10 * val));
        }
        require_equal_map(sample_map, test_map);
    }

    // delete all values
    for (size_t i = 0; i < count; ++i)
    {
        int32_t val = i;
        test_map_type::iterator stg_it = test_map.find(val);
        sample_map_type::iterator std_it = sample_map.find(val);
        BOOST_REQUIRE_EQUAL(stg_it != test_map.end(), std_it != sample_map.end());
        if (stg_it != test_map.end() || std_it != sample_map.end())
        {
            test_map.erase(val);
            sample_map.erase(val);
        }
        require_equal_map(sample_map, test_map);
    }
}
