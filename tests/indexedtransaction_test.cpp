#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE indextransaction_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/key.h"
#include "ouroboros/journalfile.h"
#include "ouroboros/dataset.h"
#include "ouroboros/transaction.h"
#include "ouroboros/sharedinterface.h"
#include "test.h"

struct test_table_interface
{
    template <typename T> struct object_type : public shared_object<T> {};
    typedef file_page<OUROBOROS_PAGE_SIZE, sizeof(journal_status_type)> file_page_type;
    typedef journal_file<file_page_type, OUROBOROS_PAGE_COUNT> file_type;
    struct locker_type : public locker<mutex_locker>
    {
        typedef typename locker<mutex_locker>::lock_type lock_type;
        locker_type(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_locker>(name, scoped_count, sharable_count)
        {}
        locker_type(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_locker>(lock, scoped_count, sharable_count)
        {}
    };
    typedef gateway<boost::interprocess::interprocess_mutex> gateway_type;
};

struct test_interface : public base_interface<test_table_interface, indexed_table>,
    public base_dataset_shared_interface {};
typedef test_interface interface_type;
typedef data_set<simple_key, record_type, index1, interface_type> dataset_type;

#include "transaction_test.h"


