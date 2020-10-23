#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rbtree_test
#include <boost/test/unit_test.hpp>

#include <set>

#include "ouroboros/field_types.h"
#include "ouroboros/datatable.h"
#include "ouroboros/treekey.h"
#include "ouroboros/controlblock.h"
#include "ouroboros/rbtree.h"
#include "ouroboros/interface.h"

using namespace ouroboros;

typedef tree_key skey_type;
typedef control_block<skey_type, local_interface> controlblock_type;
typedef std::set<int32_t> sample_tree_type;
typedef data_node<int32_t, int32_t> node_type;
typedef record1< data_field<node_type> > record_type;
typedef data_source<interface_table, record_type, local_interface> datasource_type;
typedef data_table<interface_table, record_type, controlblock_type> datatable_type;
typedef rbtree_table_adapter<datatable_type> table_type;
typedef table_smart_pnode<node_type, table_type> pnode_type;
typedef fast_rbtree<pnode_type> test_tree_type;
typedef datasource_type::file_region_type file_region_type;

const size_t rec_count = 100;

/**
 * Check the search in the tree
 * @param sample_tree the sample tree (standard)
 * @param test_tree the tested tree
 */
void check_tree_find(sample_tree_type& sample_tree, test_tree_type& test_tree)
{
    for (size_t i = 0; i < rec_count; ++i)
    {
        test_tree_type::iterator test_it = test_tree.lower_bound(i);
        sample_tree_type::iterator sample_it = sample_tree.lower_bound(i);
        BOOST_REQUIRE_EQUAL(test_it != test_tree.end(), sample_it != sample_tree.end());
        if (test_it != test_tree.end() || sample_it != sample_tree.end())
        {
            BOOST_REQUIRE_EQUAL(*sample_it, test_it->get().body());
        }
        test_it = test_tree.upper_bound(i);
        sample_it = sample_tree.upper_bound(i);
        BOOST_REQUIRE_EQUAL(test_it != test_tree.end(), sample_it != sample_tree.end());
        if (test_it != test_tree.end() || sample_it != sample_tree.end())
        {
            BOOST_REQUIRE_EQUAL(*sample_it, test_it->get().body());
        }
    }
}

/**
 * Check the trees are equal
 * @param sample_tree the sample tree (standard)
 * @param test_tree the tested tree
 */
void require_equal_tree(sample_tree_type& sample_tree, test_tree_type& test_tree)
{
    test_tree_type::iterator test_it = test_tree.begin();
    sample_tree_type::iterator sample_it = sample_tree.begin();
    BOOST_REQUIRE_EQUAL(test_tree.size(), sample_tree.size());
    BOOST_REQUIRE_EQUAL(test_it != test_tree.end(), sample_it != sample_tree.end());
    while (test_it != test_tree.end() || sample_it != sample_tree.end())
    {
        BOOST_REQUIRE_EQUAL(*sample_it, test_it->get().body());
        ++test_it;
        ++sample_it;
        BOOST_REQUIRE_EQUAL(test_it != test_tree.end(), sample_it != sample_tree.end());
    }

    test_tree_type::reverse_iterator test_rit = test_tree.rbegin();
    sample_tree_type::reverse_iterator sample_rit = sample_tree.rbegin();
    BOOST_REQUIRE_EQUAL(test_rit != test_tree.rend(), sample_rit != sample_tree.rend());
    while (test_rit != test_tree.rend() || sample_rit != sample_tree.rend())
    {
        BOOST_REQUIRE_EQUAL(*sample_rit, test_rit->get().body());
        ++test_rit;
        ++sample_rit;
        BOOST_REQUIRE_EQUAL(test_rit != test_tree.rend(), sample_rit != sample_tree.rend());
    }
    check_tree_find(sample_tree, test_tree);
}

//==============================================================================
//  Simple test
//==============================================================================
BOOST_AUTO_TEST_CASE(simple_rbtree_test)
{
    skey_type skey;
    datasource_type source("tree.dat", 1, rec_count);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);
    table_type table(source, skey);
    test_tree_type test_tree(table, NIL);
    sample_tree_type sample_tree;

    BOOST_TEST_MESSAGE("add even values:");
    for (size_t i = 0; i < rec_count; i += 2)
    {
        BOOST_TEST_MESSAGE("\tadd " << i);
        test_tree.insert(i);
        sample_tree.insert(i);
        require_equal_tree(sample_tree, test_tree);
    }

    BOOST_TEST_MESSAGE("add odd values:");
    for (size_t i = 1; i < rec_count; i += 2)
    {
        BOOST_TEST_MESSAGE("\tadd " << i);
        test_tree.insert(i);
        sample_tree.insert(i);
        require_equal_tree(sample_tree, test_tree);
    }

    BOOST_TEST_MESSAGE("remove the half of the odd values:");
    for (size_t i = 1; i < rec_count / 2; i += 2)
    {
        BOOST_TEST_MESSAGE("\tdel " << i);
        test_tree.erase(i);
        sample_tree.erase(i);
        require_equal_tree(sample_tree, test_tree);
    }

    BOOST_TEST_MESSAGE("remove the part of the values from middle of the tree:");
    for (size_t i = 3 * rec_count / 4 ; i < 9 * rec_count / 10; ++i)
    {
        BOOST_TEST_MESSAGE("\tdel " << i);
        test_tree.erase(i);
        sample_tree.erase(i);
        require_equal_tree(sample_tree, test_tree);
    }

    // add/remove random values
    BOOST_TEST_MESSAGE("add/remove random values:");
    std::srand(time(NULL));
    for (size_t i = 0; i < 10 * rec_count; ++i)
    {
        int32_t val = std::rand() % rec_count;
        test_tree_type::iterator stg_it = test_tree.find(val);
        sample_tree_type::iterator std_it = sample_tree.find(val);
        BOOST_REQUIRE_EQUAL(stg_it != test_tree.end(), std_it != sample_tree.end());
        if (stg_it != test_tree.end() || std_it != sample_tree.end())
        {
            BOOST_TEST_MESSAGE("\tdel " << val);
            test_tree.erase(val);
            sample_tree.erase(val);
        }
        else if (test_tree.size() < rec_count)
        {
            BOOST_TEST_MESSAGE("\tadd " << val);
            test_tree.insert(val);
            sample_tree.insert(val);
        }
        require_equal_tree(sample_tree, test_tree);
    }

    BOOST_TEST_MESSAGE("remove all values:");
    for (size_t i = 0; i < rec_count; ++i)
    {
        int32_t val = i;
        test_tree_type::iterator stg_it = test_tree.find(val);
        sample_tree_type::iterator std_it = sample_tree.find(val);
        BOOST_REQUIRE_EQUAL(stg_it != test_tree.end(), std_it != sample_tree.end());
        if (stg_it != test_tree.end() || std_it != sample_tree.end())
        {
            BOOST_TEST_MESSAGE("\tdel " << val);
            test_tree.erase(val);
            sample_tree.erase(val);
        }
        require_equal_tree(sample_tree, test_tree);
    }
    BOOST_REQUIRE_EQUAL(test_tree.empty(), sample_tree.empty());

    BOOST_TEST_MESSAGE("add all values with overflow:");
    for (size_t i = 0; i < 2 * rec_count; ++i)
    {
        int32_t val = i;
        BOOST_TEST_MESSAGE("\tadd " << val);
        test_tree.insert(val);
        if (val >= static_cast<int32_t>(rec_count))
        {
            sample_tree.erase(val - rec_count);
        }
        sample_tree.insert(val);
        require_equal_tree(sample_tree, test_tree);
    }
}

/**
 * Check inserting a value
 * @param tree the tree
 * @param val the value
 * @param count the count of the inserting
 */
void check_insert(test_tree_type& tree, const size_t val, const size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_TEST_MESSAGE("\tadd " << val);
        tree.insert(val);
        size_t last;
        size_t j = 0;
        test_tree_type::iterator itbeg = tree.lower_bound(0);
        test_tree_type::iterator itend = tree.upper_bound(100);
        for (test_tree_type::iterator it = itbeg; it != itend; ++it)
        {
            BOOST_TEST_MESSAGE(j << "\t:\t" << it->get().body());
            if (j != 0)
            {
                BOOST_REQUIRE(it->get().body() >= last);
            }
            last = it->get().body();
            ++j;
        }
    }
}

//==============================================================================
//  Add a lot of the same value
//==============================================================================
BOOST_AUTO_TEST_CASE(insert_equal_rbtree_test)
{
    skey_type skey;
    datasource_type source("tree.dat", 1, rec_count);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);
    table_type table(source, skey);
    test_tree_type test_tree(table, NIL);

    const size_t step = 4;
    for (size_t i = 0; i < rec_count; i += step)
    {
        check_insert(test_tree, i, step);
    }
}
