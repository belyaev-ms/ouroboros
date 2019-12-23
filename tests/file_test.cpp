#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE file_test
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "ouroboros/table.h"

using namespace ouroboros;

BOOST_AUTO_TEST_CASE(test1)
{
    const size_type size = 1024;
    const std::string filename = "test.dat";

    base_file::remove(filename.c_str());
    BOOST_REQUIRE(!boost::filesystem::exists(filename));
    {
        base_file file(filename.c_str());
        BOOST_REQUIRE(boost::filesystem::exists(filename));
        BOOST_REQUIRE_EQUAL(size, file.resize(size));
        BOOST_REQUIRE_EQUAL(size, file.size());

        char out[size] = { 0 };
        char in[size] = { 0 };
        for (size_t i = 0; i < size; ++i)
        {
            out[i] = i;
        }
        file.write(out, size, 0);
        file.read(in, size, 0);
        for (size_t i = 0; i < size; ++i)
        {
            BOOST_REQUIRE_EQUAL(out[i], in[i]);
        }
        for (size_t i = 0; i < size; ++i)
        {
            out[i] = i + 1;
            file.write(&out[i], 1, i);
            file.read(&in[i], 1, i);
            BOOST_REQUIRE_EQUAL(out[i], in[i]);
        }
    }
    base_file::remove(filename.c_str());
    BOOST_REQUIRE(!boost::filesystem::exists(filename));
}
