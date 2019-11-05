#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE table_test
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "ouroboros/key.h"
#include "ouroboros/table.h"
#include "ouroboros/page.h"

using namespace ouroboros;

BOOST_AUTO_TEST_CASE(test1)
{
    typedef simple_key skey_type;
    typedef source<file<> > source_type;
    typedef typename source_type::file_type file_type;
    typedef table<source_type, skey_type> table_type;
    typedef boost::shared_ptr<table_type> ptable_type;
    typedef std::vector<skey_type> skey_list;
    typedef std::vector<ptable_type> ptable_list;


    const size_type rec_size = 64;
    const count_type rec_count = 64;
    const count_type tbl_count = 64;
    const size_type size = rec_size * rec_count * tbl_count;
    const std::string filename = "test.dat";

    file_type::remove(filename.c_str());
    BOOST_REQUIRE(!boost::filesystem::exists(filename));
    {
        file_type file(filename.c_str());
        BOOST_REQUIRE(boost::filesystem::exists(filename));
        BOOST_REQUIRE_EQUAL(size, file.resize(size));
        BOOST_REQUIRE_EQUAL(size, file.size());

        source_type source(file, tbl_count, rec_count, rec_size);
        char out[size] = { 0 };
        char in[size] = { 0 };
        for (size_t i = 0; i < size; ++i)
        {
            out[i] = i;
        }
        skey_list keys(tbl_count);
        ptable_list tables(tbl_count);
        for (size_t i = 0; i < tbl_count; ++i)
        {
            keys[i] = skey_type(10 * i, i, 0, 0, 0, 0);
            tables[i] = ptable_type(new table_type(source, keys[i]));
        }
        for (size_t i = 0; i < tbl_count; ++i)
        {
            BOOST_REQUIRE(tables[i]->empty());
            BOOST_REQUIRE_EQUAL(tables[i]->count(), 0);
            BOOST_REQUIRE_EQUAL(tables[i]->beg_pos(), 0);
            BOOST_REQUIRE_EQUAL(tables[i]->end_pos(), 0);
            BOOST_REQUIRE_EQUAL(tables[i]->revision(), 0);
            BOOST_REQUIRE_EQUAL(tables[i]->limit(), rec_count);
            BOOST_REQUIRE_EQUAL(tables[i]->cast_skey(), keys[i]);
            BOOST_REQUIRE_EQUAL(tables[i]->skey(), keys[i]);
            BOOST_REQUIRE_EQUAL(tables[i]->front_pos(), NIL);
            BOOST_REQUIRE_EQUAL(tables[i]->back_pos(), NIL);
            BOOST_REQUIRE_EQUAL(tables[i]->index(), i);
            for (size_t j = 0; j < 2 * rec_count; ++j)
            {
                unsigned char rec[rec_size] = { (unsigned char)(i), (unsigned char)(j) };
                for (size_t k = 2; k < rec_size; ++k)
                {
                    rec[k] = rec[k - 1] + 1;
                }
                BOOST_REQUIRE_EQUAL(tables[i]->add(rec), (j + 1) % rec_count);
                BOOST_REQUIRE(!tables[i]->empty());
                BOOST_REQUIRE_EQUAL(tables[i]->end_pos(), (j + 1) % rec_count);
                BOOST_REQUIRE_EQUAL(tables[i]->back_pos(), j % rec_count);
                BOOST_REQUIRE_EQUAL(tables[i]->revision(), 0);
                BOOST_REQUIRE_EQUAL(tables[i]->skey(), keys[i]);
                if (j < rec_count)
                {
                    BOOST_REQUIRE_EQUAL(tables[i]->count(), j + 1);
                    BOOST_REQUIRE_EQUAL(tables[i]->beg_pos(), 0);
                    BOOST_REQUIRE_EQUAL(tables[i]->front_pos(), 0);
                }
                else
                {
                    BOOST_REQUIRE_EQUAL(tables[i]->count(), rec_count);
                    BOOST_REQUIRE_EQUAL(tables[i]->beg_pos(), (j + 1) % rec_count);
                    BOOST_REQUIRE_EQUAL(tables[i]->front_pos(), (j + 1) % rec_count);
                }
            }
        }
    }
    file_type::remove(filename.c_str());
    BOOST_REQUIRE(!boost::filesystem::exists(filename));
}
