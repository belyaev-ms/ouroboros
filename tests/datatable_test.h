#define DATASOURCE_NAME "datasource.dat"

struct table_type : public datatable_type
{
    using datatable_type::inc_count;
    using datatable_type::dec_count;
    using datatable_type::valid_range;
    using datatable_type::rec_offset;
};

typedef datatable_type::record_list record_list;
typedef datasource_type::file_region_type file_region_type;

//==============================================================================
//  Check for exception if a table has incorrect parameters
//==============================================================================
BOOST_AUTO_TEST_CASE(simple_error_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const count_type tbl_count = 10;
    const count_type rec_count = 100;
    skey_type skey;
    datasource_type source(DATASOURCE_NAME, tbl_count, rec_count, options);
    file_region_type file_region(tbl_count, source.table_size());
    source.set_file_region(file_region);
    BOOST_CHECK_THROW(source.init(tbl_count, rec_count), ouroboros::bug_error);
    skey.pos = -1;
    BOOST_CHECK_THROW(datatable_type table(source, skey), ouroboros::range_error);
    skey.pos = rec_count;
    BOOST_CHECK_THROW(datatable_type table(source, skey), ouroboros::range_error);
    skey.pos = 0;
    datatable_type table(source, skey);
    table.clear();
    table_type& testtable = static_cast<table_type&>(table);
    BOOST_CHECK_THROW(testtable.inc_count(rec_count + 1), ouroboros::range_error);
    BOOST_CHECK_THROW(testtable.dec_count(1), ouroboros::range_error);
    BOOST_CHECK_THROW(testtable.inc_pos(0, rec_count + 1), ouroboros::range_error);
    BOOST_CHECK_THROW(testtable.dec_pos(0, rec_count + 1), ouroboros::range_error);
    BOOST_CHECK_THROW(testtable.rec_offset(rec_count + 1), ouroboros::range_error);
    BOOST_CHECK_EQUAL(false, testtable.valid_range(0, 0));
    BOOST_CHECK_THROW(testtable.remove(0, 0), ouroboros::range_error);
    record_type record;
    table.add(record);
    table.add(record);
    table.add(record);
    table.add(record);
    BOOST_CHECK_EQUAL(true, testtable.valid_range(table.beg_pos(), table.end_pos()));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.beg_pos(), table.inc_pos(table.end_pos())));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.dec_pos(table.beg_pos()), table.end_pos()));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.dec_pos(table.beg_pos()), table.inc_pos(table.end_pos())));
    for (count_type i = 0; i < rec_count; ++i)
    {
        table.add(record);
    }
    table.remove_back(1);
    BOOST_CHECK_EQUAL(true, testtable.valid_range(table.beg_pos(), table.end_pos()));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.beg_pos(), table.inc_pos(table.end_pos())));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.dec_pos(table.beg_pos()), table.end_pos()));
    BOOST_CHECK_EQUAL(false, testtable.valid_range(table.dec_pos(table.beg_pos()), table.inc_pos(table.end_pos())));
}

//==============================================================================
//  Check for R/W operations
//      R/W operations are performed sequentially
//      R/W operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(wrrd_wrrd_onblk_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, tbl_count, rec_count, options);
    file_region_type file_region(tbl_count, source.table_size());
    source.set_file_region(file_region);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        table.clear();
        record_list records_wr;
        fill_records(records_wr, rec_count, tbl_count * index);

        table.add(records_wr);

        record_list records_rd(rec_count);
        table.read(records_rd, 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_onblk_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, tbl_count, rec_count, options);
    file_region_type file_region(tbl_count, source.table_size());
    source.set_file_region(file_region);
    record_list records_wr[tbl_count];

    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * tbl_count);
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        table.clear();
        table.add(records_wr[index]);
    }

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        table.read(records_rd, 0);
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      R/W operations are performed in blocks
//      the table has an offset of the first record
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_onblk_offset_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t rec_count = 100;
    skey_type skey;
    datasource_type source(DATASOURCE_NAME, 1, rec_count, options);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);
    datatable_type table(source, skey);
    table.clear();

    record_list records_wr;
    fill_records(records_wr, rec_count / 2, 0);
    table.add(records_wr); // offset

    fill_records(records_wr, rec_count / 2, rec_count / 2);
    table.add(records_wr);

    record_list records_rd(rec_count);
    table.read(records_rd, table.beg_pos());
    BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed one at a time
//      R operations are performed in blocks
//      the table has an offset of the first record
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_onblk_offset_test1)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t rec_count = 100;
    skey_type skey;
    datasource_type source(DATASOURCE_NAME, 1, rec_count, options);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);
    datatable_type table(source, skey);
    table.clear();

    record_list records_wr;
    fill_records(records_wr, rec_count / 2, 0);
    table.add(records_wr); // offset

    fill_records(records_wr, rec_count / 2, rec_count / 2);
    for (count_type i = 0; i < rec_count; ++i)
    {
        table.add(records_wr[i]);
    }

    record_list records_rd(rec_count);
    table.read(records_rd, table.beg_pos());
    BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
}

//==============================================================================
//  Check for R/W operations
//      R/W operations are performed sequentially
//      R/W operations are performed in one at a time
//==============================================================================
BOOST_AUTO_TEST_CASE(wrrd_wrrd_offblk_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, tbl_count, rec_count, options);
    file_region_type file_region(tbl_count, source.table_size());
    source.set_file_region(file_region);
    for (size_t index = 0; index < tbl_count; ++index)
    {
        BOOST_TEST_MESSAGE(PE(index));
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        table.clear();
        record_list records_wr;
        fill_records(records_wr, rec_count, tbl_count * index);

        for (size_t i = 0; i < rec_count; ++i)
        {
            table.add(records_wr[i]);
        }

        record_list records_rd(rec_count);
        for (size_t i = 0; i < rec_count; ++i)
        {
            table.read(records_rd[i], i);
        }

        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W operations
//      W operations are performed first, then R operations are executed
//      R/W operations are performed in one at a time
//==============================================================================
BOOST_AUTO_TEST_CASE(wrwr_rdrd_offblk_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t tbl_count = 10;
    const size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, tbl_count, rec_count, options);
    file_region_type file_region(tbl_count, source.table_size());
    source.set_file_region(file_region);

    record_list records_wr[tbl_count];

    for (size_t index = 0; index < tbl_count; index++)
    {
        BOOST_TEST_MESSAGE(PE(index));
        fill_records(records_wr[index], rec_count, index * rec_count);
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        table.clear();
        for (size_t i = 0; i < rec_count; ++i)
        {
            table.add(records_wr[index][i]);
        }
    }

    record_list records_rd(rec_count);
    for (size_t index = 0; index < tbl_count; index++)
    {
        BOOST_TEST_MESSAGE(PE(index));
        skey_type skey(index, index, 0, 0, 0, 0);
        datatable_type table(source, skey);
        for (size_t i = 0; i < rec_count; ++i)
        {
            table.read(records_rd[i], i);
        }
        BOOST_CHECK_EQUAL_COLLECTIONS(records_wr[index].begin(), records_wr[index].end(),
            records_rd.begin(), records_rd.end());
    }
}

//==============================================================================
//  Check for R/W/D operations
//      W operations are performed first, then D operations are executed,
//          then R operation are executed
//      R/W operations are performed in blocks
//      D operations are performed in one at a time
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_d1_rd_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, 1, rec_count, options);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);
    record_list records_wr;
    fill_records(records_wr, rec_count, 0);

    skey_type skey;
    datatable_type table(source, skey);
    table.clear();
    table.add(records_wr);

    // delete a record from the begin of the table
    --rec_count;
    table.remove(table.beg_pos());
    records_wr.erase(records_wr.begin());

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }

    // delete a record from the end of the table
    --rec_count;
    table.remove(table.back_pos());
    records_wr.erase(records_wr.begin() + rec_count);

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }

    // delete a record from the middle of the table
    --rec_count;
    table.remove(table.beg_pos() + rec_count / 2);
    records_wr.erase(records_wr.begin() + rec_count / 2);

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }

    // delete all records and more
    BOOST_REQUIRE(table.remove_back(2 * rec_count) == 0);
    BOOST_REQUIRE(table.empty());
}

//==============================================================================
//  Check for R/W/D operations
//      W operations are performed first, then D operations are executed,
//          then R operation are executed
//      R/W/D operations are performed in blocks
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_d_onblk_rd_test)
{
    datasource_type::remove(DATASOURCE_NAME);
    const size_t count = 10;
    size_t rec_count = 100;
    datasource_type source(DATASOURCE_NAME, 1, rec_count, options);
    file_region_type file_region(1, source.table_size());
    source.set_file_region(file_region);

    skey_type skey;
    datatable_type table(source, skey);
    table.clear();

    record_list records_wr;
    fill_records(records_wr, rec_count / 2, 0);
    table.add(records_wr); // offset

    fill_records(records_wr, rec_count / 2, rec_count / 2);
    table.add(records_wr);

    // delete records from the begin of the table
    rec_count -= count;
    table.remove(table.beg_pos(), count);
    records_wr.erase(records_wr.begin(), records_wr.begin() + count);

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }

    // delete records from the end of the table
    rec_count -= count;
    BOOST_REQUIRE(table.remove_back(count) == table.count());
    records_wr.erase(records_wr.begin() + rec_count, records_wr.end());

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }

    // delete records from the middle of the table
    rec_count -= count;
    table.remove(table.beg_pos() + 10, count);
    records_wr.erase(records_wr.begin() + 10, records_wr.begin() + count + 10);

    {
        record_list records_rd(table.count());
        table.read(records_rd, table.beg_pos());
        BOOST_REQUIRE_EQUAL_COLLECTIONS(records_wr.begin(), records_wr.end(), records_rd.begin(), records_rd.end());
    }
}