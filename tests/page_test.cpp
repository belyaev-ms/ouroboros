#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE page_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/page.h"
#include <stdint.h>
#include <vector>

using namespace ouroboros;

//==============================================================================
//  Check the file page is valid
//==============================================================================
BOOST_AUTO_TEST_CASE(file_page_test)
{
    typedef file_page<1024> file_page_type;
    std::vector<uint8_t> buffer(3 * file_page_type::FILE_PAGE_SIZE);
    BOOST_REQUIRE_EQUAL(file_page_type::static_size(), 1024);
    BOOST_REQUIRE_EQUAL(file_page_type::FILE_PAGE_SIZE, 1024);
    {
        file_page_type page;
        BOOST_REQUIRE(!page.valid());
        BOOST_REQUIRE_EQUAL(page.pos(), NIL);
        BOOST_REQUIRE_EQUAL(page.index(), NIL);
        BOOST_REQUIRE_THROW(page.read(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], 32), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(NULL, 32), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::FILE_PAGE_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], 32), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(NULL, 32), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::FILE_PAGE_SIZE + 1), bug_error);
    }
    {
        file_page_type page(0);
        BOOST_REQUIRE(!page.valid());
        BOOST_REQUIRE_EQUAL(page.pos(), 0);
        BOOST_REQUIRE_EQUAL(page.index(), 0);
        BOOST_REQUIRE_THROW(page.read(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        std::vector<uint8_t> data(file_page_type::FILE_PAGE_SIZE);
        page.assign(&data[0]);
        BOOST_REQUIRE(page.valid());
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0]), &buffer[file_page_type::FILE_PAGE_SIZE]);
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.read_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read_rest(&buffer[0]), &buffer[1]);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::FILE_PAGE_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(NULL), bug_error);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0]), &buffer[file_page_type::FILE_PAGE_SIZE]);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.write_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write_rest(&buffer[0]), &buffer[1]);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::FILE_PAGE_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        file_page_type page0 = page;
        BOOST_REQUIRE(page == page0);
        ++page;
        BOOST_REQUIRE(page0 < page);
        BOOST_REQUIRE_EQUAL(page.pos(), file_page_type::FILE_PAGE_SIZE);
        BOOST_REQUIRE_EQUAL(page.index(), 1);
        BOOST_REQUIRE(!page.valid());
        
        const pos_type offset = file_page_type::FILE_PAGE_SIZE / 4;
        const pos_type index = 2;
        const pos_type pos = file_page_type::FILE_PAGE_SIZE * index + offset;
        file_page_type page2(pos);
        file_page_type page1(page2);
        page = page1;
        BOOST_REQUIRE_EQUAL(page.pos(), pos);
        BOOST_REQUIRE_EQUAL(page.index(), 2);
        BOOST_REQUIRE_THROW(page.read(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        page.assign(&data[0]);
        BOOST_REQUIRE(page.valid());
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0]), &buffer[file_page_type::FILE_PAGE_SIZE - offset]);
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.read_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read_rest(&buffer[0]), &buffer[offset + 1]);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::FILE_PAGE_SIZE -offset + 1), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(NULL), bug_error);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0]), &buffer[file_page_type::FILE_PAGE_SIZE - offset]);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.write_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write_rest(&buffer[0]), &buffer[offset + 1]);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::FILE_PAGE_SIZE - offset + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        ++page;
        BOOST_REQUIRE(page1 < page);
        BOOST_REQUIRE_EQUAL(page.pos(), (index + 1) * file_page_type::FILE_PAGE_SIZE);
        BOOST_REQUIRE_EQUAL(page.index(), index + 1);
        BOOST_REQUIRE(!page.valid());
    }
}

