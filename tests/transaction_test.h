#define DATASET_NAME "ouroboros"
typedef dataset_type::record_list record_list;
const size_t tbl_count = 10;
const size_t rec_count = 100;

/**
 * Get the test dataset
 * @return the test dataset
 */
dataset_type& db()
{
    static bool is_init = false;
    if (!is_init)
    {
        dataset_type::remove(DATASET_NAME);
    }
    static dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
    if (!is_init)
    {
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE("add a table " << PE(index));
            dataset.add_table(index);
        }
        is_init = true;
    }
    return dataset;
}

template <typename Transaction>
struct transaction_helper
{
    void operator()(base_transaction& dest)
    {
        dest.attach(transaction_maker<Transaction>::make(db()));
    }
};

typedef dataset_transaction<dataset_type> transaction_type;
typedef lazy_transaction<dataset_type> lazy_transaction_type;
typedef global_transaction<interface_type, transaction_helper<transaction_type> > global_transaction_type;
typedef global_lazy_transaction<interface_type, transaction_helper<lazy_transaction_type> > global_lazy_transaction_type;

//==============================================================================
//  Check for transactions of R/W operations
//==============================================================================
BOOST_AUTO_TEST_CASE(transact_full)
{
    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            record_list records_wr;
            BOOST_TEST_MESSAGE(PE(index));
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
    }

    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            BOOST_REQUIRE(!db().session_rd(index)->empty());
            BOOST_REQUIRE_EQUAL(rec_count, db().session_rd(index)->count());
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            record_list records_rd(rec_count);
            db().session_rd(index)->read(records_rd, 0);
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
                records_rd.begin(), records_rd.end());
        }
    }
}

//==============================================================================
//  Check for cancellation a write transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(transact_wr_cancel_0)
{
    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count / 2, 1 + tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(!db().session_rd(index)->empty());
        BOOST_REQUIRE_EQUAL(rec_count, db().session_rd(index)->count());
        record_list records_wr;
        fill_records(records_wr, rec_count, tbl_count * index);
        record_list records_rd(rec_count);
        db().session_rd(index)->read(records_rd, 0);
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for cancellation a write transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(transact_clr_wr_cancel)
{
    for (size_t index = 0; index < tbl_count; ++index)
    {
        db().session_wr(index)->clear();
    }

    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a read transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(transact_rd_cancel)
{
    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count / 2, 1 + tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            BOOST_REQUIRE(db().session_rd(index)->empty());
        }
        transact.cancel();
    }
}

//==============================================================================
//  Check for cancellation a write transaction after the exception
//==============================================================================
BOOST_AUTO_TEST_CASE(transact_wr_cancel_except)
{
    try
    {
        transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        throw(1);
    }
    catch (...)
    {

    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a lazy write transaction after the exception
//==============================================================================
BOOST_AUTO_TEST_CASE(lazy_transact_wr_cancel_except)
{
    try
    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        throw(1);
    }
    catch (...)
    {

    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a lazy write transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(lazy_transact_wr_cancel)
{
    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for a lazy write transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(lazy_transact_wr_stop)
{
    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
    }

    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            BOOST_REQUIRE(!db().session_rd(index)->empty());
            BOOST_REQUIRE_EQUAL(rec_count, db().session_rd(index)->count());
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            record_list records_rd(rec_count);
            db().session_rd(index)->read(records_rd, 0);
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
                records_rd.begin(), records_rd.end());
        }
    }

    {
        lazy_transaction_type transact(db());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            db().session_wr(index)->clear();
        }
        transact.stop();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a global transaction after the exception
//==============================================================================
BOOST_AUTO_TEST_CASE(global_transact_cancel_except)
{
    try
    {
        global_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        throw(1);
    }
    catch (...)
    {

    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a global transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(global_transact_cancel)
{
    {
        global_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for a global transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(global_transact_stop)
{
    {
        global_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
    }

    {
        global_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            BOOST_REQUIRE(!db().session_rd(index)->empty());
            BOOST_REQUIRE_EQUAL(rec_count, db().session_rd(index)->count());
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            record_list records_rd(rec_count);
            db().session_rd(index)->read(records_rd, 0);
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
                records_rd.begin(), records_rd.end());
        }
    }

    {
        global_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            db().session_wr(index)->clear();
        }
        transact.stop();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a global lazy transaction after the exception
//==============================================================================
BOOST_AUTO_TEST_CASE(global_lazy_transact_cancel_except)
{
    try
    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        throw(1);
    }
    catch (...)
    {

    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for cancellation a global lazy transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(global_lazy_transact_cancel)
{
    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
        transact.cancel();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

//==============================================================================
//  Check for a global lazy transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(global_lazy_transact_stop)
{
    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            db().session_wr(index)->add(records_wr);
        }
    }

    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            BOOST_REQUIRE(!db().session_rd(index)->empty());
            BOOST_REQUIRE_EQUAL(rec_count, db().session_rd(index)->count());
            record_list records_wr;
            fill_records(records_wr, rec_count, tbl_count * index);
            record_list records_rd(rec_count);
            db().session_rd(index)->read(records_rd, 0);
            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
                records_rd.begin(), records_rd.end());
        }
    }

    {
        global_lazy_transaction_type transact;
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE(PE(index));
            db().session_wr(index)->clear();
        }
        transact.stop();
    }

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        BOOST_REQUIRE(db().session_rd(index)->empty());
    }
}

