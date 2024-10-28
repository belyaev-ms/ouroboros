#include "test.h"

#define DATASET_NAME "dataset"

typedef dataset_type::record_list record_list;

//==============================================================================
//  Check for R/W operations
//      R/W operations are performed sequentially
//      R/W operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(wrrd_wrrd_onblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    record_list records_wr[tbl_count];
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, tbl_count * index);

        dataset.session_wr(index)->add(records_wr[index]);

        for (size_t num = 0; num <= index; ++num)
        {
            record_list records_rd(rec_count);
            dataset.session_rd(num)->read(records_rd, 0);

            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[num].begin(), records_wr[num].end(),
                records_rd.begin(), records_rd.end());
        }
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_onblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    record_list records_wr[tbl_count];

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * tbl_count);
        dataset.session_wr(index)->add(records_wr[index]);
    }

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        dataset.session_rd(index)->read(records_rd, 0);
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      R/W operations are performed sequentially
//      R/W operations are performed in one at a time
//==============================================================================
BOOST_AUTO_TEST_CASE(wrrd_wrrd_offblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    record_list records_wr[tbl_count];
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, tbl_count * index);

        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_wr(index)->add(records_wr[index][i]);
        }

        for (size_t num = 0; num <= index; ++num)
        {
            record_list records_rd(rec_count);
            for (size_t i = 0; i < rec_count; ++i)
            {
                dataset.session_rd(num)->read(records_rd[i], i);
            }

            BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[num].begin(), records_wr[num].end(),
                records_rd.begin(), records_rd.end());
        }
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in one at a time
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_offblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    record_list records_wr[tbl_count];
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * rec_count);
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_wr(index)->add(records_wr[index][i]);
        }
    }

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_rd(index)->read(records_rd[i], i);
        }
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check reading of records from previously prepared dataset
//      R operations are performed sequentially
//      R operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(rd_onblk_test)
{
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        record_list records_wr;
        fill_records(records_wr, rec_count, index * rec_count);
        BOOST_TEST_MESSAGE(PE(index));
        dataset.session_rd(index)->read(records_rd, 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in one at a time
//      W operations are performed in a session
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwrSes_rdrd_offblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    record_list records_wr[tbl_count];
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * rec_count);
        dataset_type::session_write session = dataset.session_wr(index);
        for (size_t i = 0; i < rec_count; ++i)
        {
           session->add(records_wr[index][i]);
        }
    }

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_rd(index)->read(records_rd[i], i);
        }
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in one at a time
//      R/W operations are performed in a transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwrTR_rdrdTR_offblk_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE("add a table " << PE(index));
        dataset.add_table(index);
    }

    dataset.start();
    record_list records_wr[tbl_count];
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * rec_count);
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_wr(index)->add(records_wr[index][i]);
        }
    }
    dataset.cancel();

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset_type::session_read session = dataset.session_rd(index);
            BOOST_REQUIRE(session->empty());
            BOOST_CHECK_EQUAL(0u, session->beg_pos());
            BOOST_CHECK_EQUAL(0u, session->end_pos());
        }
    }

    dataset.start();
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_wr(index)->add(records_wr[index][i]);
        }
    }
    dataset.stop();

    record_list records_rd(rec_count);
    dataset.start();
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        for (size_t i = 0; i < rec_count; ++i)
        {
            dataset.session_rd(index)->read(records_rd[i], i);
        }
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
    dataset.stop();
}

//==============================================================================
//  Check for a rollback of a transaction when something throws an exception
//==============================================================================
BOOST_AUTO_TEST_CASE(session_wr_except_cancel)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    dataset_type dataset(DATASET_NAME, tbl_count, rec_count);

    dataset.add_table(0);

    record_list records_wr;
    fill_records(records_wr, rec_count, 0);

    try
    {
        dataset_type::session_write session = dataset.session_wr(0);
        for (size_t i = 0; i < rec_count; ++i)
        {
            session->add(records_wr[i]);
        }
        throw(1);
    }
    catch (...)
    {

    }

    for (size_t i = 0; i < rec_count; ++i)
    {
        dataset_type::session_read session = dataset.session_rd(0);
        BOOST_REQUIRE(session->empty());
        BOOST_CHECK_EQUAL(0u, session->beg_pos());
        BOOST_CHECK_EQUAL(0u, session->end_pos());
    }
}

//==============================================================================
//  Check for add and remove table
//==============================================================================
BOOST_AUTO_TEST_CASE(add_and_remove_table)
{

    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    
    // add and remove one table without reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        dataset.add_table(0);
        dataset.remove_table(0);
    }
    
    // add and remove all tables without reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.remove_table(i);
        }
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
    }
    
    // add and remove all tables in different order without reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.remove_table(tbl_count - i - 1);
        }
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(tbl_count - i - 1);
        }
    }
    
    // add and remove one table with reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        dataset.add_table(0);
    }
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        dataset.remove_table(0);
    }
    
    // add and remove all tables with reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
    }
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.remove_table(i);
        }
    }
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
    }
    
    // add and remove all tables in different order with reopening the dataset
    dataset_type::remove(DATASET_NAME);
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(i);
        }
    }
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.remove_table(tbl_count - i - 1);
        }
    }
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            dataset.add_table(tbl_count - i - 1);
        }
    }
}

//==============================================================================
//  Check for read version
//==============================================================================
BOOST_AUTO_TEST_CASE(check_wrong_ver_test)
{
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    typedef dataset_type dataset_type;
    {
        const size_t VERSION = 1;
        BOOST_CHECK_NO_THROW(dataset_type dataset(DATASET_NAME, tbl_count, rec_count, VERSION));
    }
    {
        const size_t VERSION = 2;
        BOOST_CHECK_THROW(dataset_type dataset(DATASET_NAME, tbl_count, rec_count, VERSION), ouroboros::version_error);
    }
    {
        BOOST_CHECK_NO_THROW(dataset_type dataset(DATASET_NAME, tbl_count, rec_count));
    }
}

//==============================================================================
//  Check the reopening of the dataset
//==============================================================================
BOOST_AUTO_TEST_CASE(reopen_test)
{
    dataset_type::remove(DATASET_NAME);

    const size_t tbl_count = 2;
    const size_t rec_count = 10000;
    {
        dataset_type dataset(DATASET_NAME, tbl_count, rec_count);
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_TEST_MESSAGE("add a table " << PE(index));
            dataset.add_table(index);
        }
    }

    {
        dataset_type dataset(DATASET_NAME);
        dataset.open();
        BOOST_CHECK_EQUAL(tbl_count, dataset.table_count());
        BOOST_CHECK_EQUAL(rec_count, dataset.rec_count());
        for (size_t index = 0; index < tbl_count; ++index)
        {
            BOOST_CHECK(dataset.table_exists(index));
        }
    }
}