#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE find_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/key.h"
#include "ouroboros/dataset.h"
#include "ouroboros/interface.h"
#include "ouroboros/session.h"
#include "ouroboros/find.h"
#include "test.h"

#define DATASET_NAME "ouroboros"
typedef data_set<simple_key, record_type, index_null, local_interface> dataset_type;
typedef dataset_type::record_list record_list;

BOOST_AUTO_TEST_CASE(find_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t count = 100;
    dataset_type dataset(DATASET_NAME, 1, count);

    dataset.add_table(0);
    record_list records_wr;
    fill_records(records_wr, count, 0);
    dataset.session_wr(0)->add(records_wr);

    {
        typedef comp_greater_equal<record_type, index1> comparator_type;
        typedef finder<comparator_type> finder_type;
        finder_type finder(comparator_type(count / 2));
        {
            dataset_type::session_read session = dataset.session_rd(0);
            session->find(finder, session->beg_pos(), session->count());
            const finder_type::result_type& result = finder.result();
            BOOST_CHECK_EQUAL(count / 2, result.size());
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin() + (count / 2), records_wr.end(), result.begin(), result.end());
        }
    }

    {
        typedef comp_greater_equal<record_type, index1> greater_equal_type;
        typedef comp_less<record_type, index1> less_type;
        typedef comb_and<greater_equal_type, less_type> comparator_type;
        typedef finder<comparator_type> finder_type;
        finder_type finder(comparator_type(greater_equal_type(10), less_type(count - 10)));
        {
            dataset_type::session_read session = dataset.session_rd(0);
            session->find(finder, session->beg_pos(), session->count());
            const finder_type::result_type& result = finder.result();
            BOOST_CHECK_EQUAL(count - 20, result.size());
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin() + 10, records_wr.end() - 10, result.begin(), result.end());
        }
    }

    {
        typedef comp_greater_equal<record_type, index1> greater_equal_type;
        typedef comp_less<record_type, index2> less_type;
        typedef comb_and<greater_equal_type, less_type> comparator_type;
        typedef finder<comparator_type> finder_type;
        finder_type finder(comparator_type(greater_equal_type(10), less_type(count - 10)));
        {
            dataset_type::session_read session = dataset.session_rd(0);
            session->find(finder, session->beg_pos(), session->count());
            const finder_type::result_type& result = finder.result();
            BOOST_CHECK_EQUAL(count - 21, result.size());
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin() + 10, records_wr.end() - 11, result.begin(), result.end());
        }
    }

    {
        typedef comp_greater_equal<record_type, index1> comparator1_type;
        typedef fields_comparator<record_type, index1, index2, op_less> comparator2_type;
        typedef comb_and<comparator1_type, comparator2_type> comparator_type;
        typedef finder<comparator_type> finder_type;
        finder_type finder(comparator_type(comparator1_type(10), comparator2_type()));
        {
            dataset_type::session_read session = dataset.session_rd(0);
            session->find(finder, session->beg_pos(), session->count());
            const finder_type::result_type& result = finder.result();
            BOOST_CHECK_EQUAL(count - 10, result.size());
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin() + 10, records_wr.end(), result.begin(), result.end());
        }
    }

    {
        typedef comp_greater_equal<record_type, index1> comparator1_type;
        typedef fields_comparator<record_type, index1, index2, op_greater> comparator2_type;
        typedef comb_and<comparator1_type, comparator2_type> comparator_type;
        typedef finder<comparator_type> finder_type;
        finder_type finder(comparator_type(comparator1_type(10), comparator2_type()));
        {
            dataset_type::session_read session = dataset.session_rd(0);
            session->find(finder, session->beg_pos(), session->count());
            const finder_type::result_type& result = finder.result();
            BOOST_CHECK_EQUAL(0, result.size());
        }
    }
}
