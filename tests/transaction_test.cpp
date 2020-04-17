#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transaction_test
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
    struct locker_type : public locker<mutex_lock>
    {
        locker_type(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_lock>(name, scoped_count, sharable_count)
        {}
    };
    typedef gateway<boost::interprocess::interprocess_mutex> gateway_type;
};

struct test_interface : public base_interface<test_table_interface, indexed_table>,
    public base_dataset_shared_interface {};
typedef test_interface interface_type;
typedef data_set<simple_key, record_type, index_null, interface_type> dataset_type;

#include "transaction_test.h"

template <typename Transaction>
struct stupid_transaction_helper
{
    void operator()(base_transaction& dest)
    {
        dest.attach(transaction_maker<Transaction>::make(db()));
        dest.attach(transaction_maker<Transaction>::make(db()));
    }
};

typedef global_transaction<interface_type, stupid_transaction_helper<transaction_type> > stupid_global_transaction;
typedef global_lazy_transaction<interface_type, stupid_transaction_helper<lazy_transaction_type> > stupid_global_lazy_transaction;

//==============================================================================
//  Check removing locks after a transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(session_blocked_test)
{
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // check the exception handling inside the read session
    for (size_t index = 0; index < tbl_count; ++index)
    {
        try
        {
            typename dataset_type::session_read session = db().session_rd(index);
            throw(1);
        }
        catch (...)
        {

        }
    }
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // check the exception handling inside the write session
    for (size_t index = 0; index < tbl_count; ++index)
    {
        try
        {
            typename dataset_type::session_write session = db().session_wr(index);
            throw(1);
        }
        catch (...)
        {

        }
    }
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // lock the table 0
    mutex_lock locker0("ouroboros.dat.812.locker");
    locker0.lock();
    // check that only the table 0 is locked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        if (index != 0)
        {
            BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        }
        else
        {
            BOOST_CHECK_THROW(db().session_wr(index)->empty(), ouroboros::lock_error);
        }
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    locker0.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // lock the table 1
    mutex_lock locker1("ouroboros.dat.8040.locker");
    locker1.lock();
    // check that only the table 1 is locked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        if (index != 1)
        {
            BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        }
        else
        {
            BOOST_CHECK_THROW(db().session_wr(index)->empty(), ouroboros::lock_error);
        }
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    locker1.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // check that when the table 0 is locked whole transaction will be rolled back
    locker0.lock();
    try
    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            db().session_wr(index)->empty();
        }
    }
    catch (const lock_error&)
    {
    }
    locker0.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // check that when the table 1 is locked whole transaction will be rolled back
    locker1.lock();
    try
    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            db().session_wr(index)->empty();
        }
    }
    catch (const lock_error&)
    {
    }
    locker1.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // lock the key table
    mutex_lock klocker("ouroboros.dat.784.locker");
    klocker.lock();
    try
    {
        lazy_transaction_type transact(db());
    }
    catch (const lock_error&)
    {
    }
    klocker.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // perform an unsuccessful global lock
    try
    {
        stupid_global_transaction transact;
    }
    catch (...)
    {
    }
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
    }
    // perform an unsuccessful global lazy lock
    try
    {
        stupid_global_lazy_transaction transact;
    }
    catch (...)
    {
    }
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
        BOOST_CHECK(!db().lazy_transaction_exists());
    }
    locker1.lock();
    // perform an unsuccessful global lazy lock
    try
    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            db().session_wr(index)->empty();
        }
    }
    catch (...)
    {
    }
    locker1.unlock();
    locker0.lock();
    // perform an unsuccessful global lazy lock
    try
    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            db().session_wr(index)->empty();
        }
    }
    catch (...)
    {
    }
    locker0.unlock();
    // check that all tables are unlocked
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_CHECK_NO_THROW(db().session_wr(index)->empty());
        typename dataset_type::table_type& table = static_cast<dataset_type::table_type&>(db().session_wr(index).table());
        BOOST_CHECK_EQUAL(table.sharable_count(), 0);
        BOOST_CHECK_EQUAL(table.scoped_count(), 0);
        BOOST_CHECK_EQUAL(db().state(), TR_STOPPED);
        BOOST_CHECK(!db().lazy_transaction_exists());
    }
    BOOST_CHECK_EQUAL(base_global_locker::sharable_count(), 0);
    BOOST_CHECK_EQUAL(base_global_locker::scoped_count(), 0);
}
