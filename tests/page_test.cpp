#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE page_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/page.h"
#include <stdint.h>
#include <vector>

using namespace ouroboros;

template <typename FilePage>
void test_file_page()
{
    typedef FilePage file_page_type;
    BOOST_REQUIRE_EQUAL(file_page_type::static_size(), file_page_type::TOTAL_SIZE);
    BOOST_REQUIRE_EQUAL(file_page_type::static_data_size(), file_page_type::DATA_SIZE);
    BOOST_REQUIRE_EQUAL(file_page_type::TOTAL_SIZE, file_page_type::SERVICE_SIZE + file_page_type::DATA_SIZE);
    std::vector<uint8_t> buffer(3 * file_page_type::DATA_SIZE);
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
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::DATA_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], 32), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(NULL, 32), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::DATA_SIZE + 1), bug_error);
    }
    {
        file_page_type page(0);
        BOOST_REQUIRE(!page.valid());
        BOOST_REQUIRE_EQUAL(page.pos(), 0);
        BOOST_REQUIRE_EQUAL(page.index(), 0);
        BOOST_REQUIRE_THROW(page.read(&buffer[0]), bug_error);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        std::vector<uint8_t> data(file_page_type::TOTAL_SIZE);
        page.assign(&data[0]);
        BOOST_REQUIRE(page.valid());
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0]), &buffer[file_page_type::DATA_SIZE]);
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.read_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read_rest(&buffer[0]), &buffer[1]);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::DATA_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(NULL), bug_error);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0]), &buffer[file_page_type::DATA_SIZE]);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.write_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write_rest(&buffer[0]), &buffer[1]);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::DATA_SIZE + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        file_page_type page0 = page;
        BOOST_REQUIRE(page == page0);

        ++page;
        BOOST_REQUIRE(page0 < page);
        BOOST_REQUIRE_EQUAL(page.pos(), file_page_type::TOTAL_SIZE);
        BOOST_REQUIRE_EQUAL(page.index(), 1);
        BOOST_REQUIRE(!page.valid());
        
        const pos_type offset = file_page_type::DATA_SIZE / 4;
        const pos_type index = 2;
        const pos_type pos = file_page_type::TOTAL_SIZE * index + offset;
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
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0]), &buffer[file_page_type::DATA_SIZE - offset]);
        BOOST_REQUIRE_NO_THROW(page.read(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.read(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.read_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.read_rest(&buffer[0]), &buffer[offset + 1]);
        BOOST_REQUIRE_THROW(page.read(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.read(&buffer[0], file_page_type::DATA_SIZE -offset + 1), bug_error);
        BOOST_REQUIRE_THROW(page.read_rest(NULL), bug_error);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0]), &buffer[file_page_type::DATA_SIZE - offset]);
        BOOST_REQUIRE_NO_THROW(page.write(&buffer[0], 32));
        BOOST_REQUIRE_EQUAL(page.write(&buffer[0], 32), &buffer[32]);
        BOOST_REQUIRE_NO_THROW(page.write_rest(&buffer[0]));
        BOOST_REQUIRE_EQUAL(page.write_rest(&buffer[0]), &buffer[offset + 1]);
        BOOST_REQUIRE_THROW(page.write(NULL), bug_error);
        BOOST_REQUIRE_THROW(page.write(&buffer[0], file_page_type::DATA_SIZE - offset + 1), bug_error);
        BOOST_REQUIRE_THROW(page.write_rest(NULL), bug_error);
        ++page;
        BOOST_REQUIRE(page1 < page);
        BOOST_REQUIRE_EQUAL(page.pos(), (index + 1) * file_page_type::TOTAL_SIZE);
        BOOST_REQUIRE_EQUAL(page.index(), index + 1);
        BOOST_REQUIRE(!page.valid());
    }
}

//==============================================================================
//  Check the file page
//==============================================================================
BOOST_AUTO_TEST_CASE(file_page_test)
{
    typedef file_page<1024> file_page_type;
    test_file_page<file_page_type>();
}

//==============================================================================
//  Check the file page that has a service data
//==============================================================================
BOOST_AUTO_TEST_CASE(file_page_service_data_test)
{
    typedef file_page<1024, 32> file_page_type;
    test_file_page<file_page_type>();
}

template <typename FilePage>
void test_file_region()
{
    size_t count = 3;
    size_t size = 5;
    typedef FilePage file_page_type;
    typedef file_region<file_page_type> file_region_type;
    typedef typename file_region_type::range_type range_type;
    file_region_type subregion(count, size);
    {
        offset_type offset = 0;
        for (size_t i = 0; i < count; ++i)
        {
            range_type result = subregion[i];
            BOOST_REQUIRE_EQUAL(result.first, offset);
            BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
            offset += file_page_type::TOTAL_SIZE;
        }
        BOOST_REQUIRE_THROW(subregion[count], bug_error);
        offset = 0;
        for (size_t i = 0; i < size * count; ++i)
        {
            if (i > 0 && i % size == 0)
            {
                offset += file_page_type::TOTAL_SIZE - size;
            }
            BOOST_REQUIRE_EQUAL(subregion.convert_offset(i), offset);
            ++offset;
        }
        BOOST_REQUIRE_THROW(subregion.convert_offset(size * count), bug_error);
        BOOST_REQUIRE_EQUAL(subregion.convert_size(size * count), file_page_type::TOTAL_SIZE * count);
    }
    {
        file_region_type region(0, size);
        offset_type offset = 0;
        for (size_t i = 0; i < count; ++i)
        {
            range_type result = subregion[i];
            BOOST_REQUIRE_EQUAL(result.first, offset);
            BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
            offset += file_page_type::TOTAL_SIZE;
        }
        BOOST_REQUIRE_NO_THROW(region[count + 1000]);
        offset = 0;
        for (size_t i = 0; i < size * count; ++i)
        {
            if (i > 0 && i % size == 0)
            {
                offset += file_page_type::TOTAL_SIZE - size;
            }
            BOOST_REQUIRE_EQUAL(region.convert_offset(i), offset);
            ++offset;
        }
        BOOST_REQUIRE_NO_THROW(region.convert_offset(size * count));
        BOOST_REQUIRE_EQUAL(subregion.convert_size(size * count), file_page_type::TOTAL_SIZE * count);
    }
    {
        file_region_type region(1, subregion);
        offset_type offset = 0;
        for (size_t i = 0; i < count; ++i)
        {
            range_type result = region[i];
            BOOST_REQUIRE_EQUAL(result.first, offset);
            BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
            offset += file_page_type::TOTAL_SIZE;
        }
        BOOST_REQUIRE_THROW(region[count], bug_error);
        offset = 0;
        for (size_t i = 0; i < size * count; ++i)
        {
            if (i > 0 && i % size == 0)
            {
                offset += file_page_type::TOTAL_SIZE - size;
            }
            BOOST_REQUIRE_EQUAL(region.convert_offset(i), offset);
            ++offset;
        }
        BOOST_REQUIRE_THROW(region.convert_offset(size * count), bug_error);
        BOOST_REQUIRE_EQUAL(subregion.convert_size(size * count), file_page_type::TOTAL_SIZE * count);
    }
    {
        file_region_type region(3, subregion);
        offset_type offset = 0;
        for (size_t i = 0; i < 3 * count; ++i)
        {
            range_type result = region[i];
            BOOST_REQUIRE_EQUAL(result.first, offset);
            BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
            offset += file_page_type::TOTAL_SIZE;
        }
        BOOST_REQUIRE_THROW(region[3 * count], bug_error);
        offset = 0;
        for (size_t i = 0; i < 3 * size * count; ++i)
        {
            if (i > 0 && i % size == 0)
            {
                offset += file_page_type::TOTAL_SIZE - size;
            }
            BOOST_REQUIRE_EQUAL(region.convert_offset(i), offset);
            ++offset;
        }
        BOOST_REQUIRE_THROW(region.convert_offset(3 * size * count), bug_error);
    }
    {
        file_region_type region(0, subregion);
        offset_type offset = 0;
        for (size_t i = 0; i < 3 * count; ++i)
        {
            range_type result = region[i];
            BOOST_REQUIRE_EQUAL(result.first, offset);
            BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
            offset += file_page_type::TOTAL_SIZE;
        }
        BOOST_REQUIRE_NO_THROW(region[3 * count]);
        offset = 0;
        for (size_t i = 0; i < 3 * size * count; ++i)
        {
            if (i > 0 && i % size == 0)
            {
                offset += file_page_type::TOTAL_SIZE - size;
            }
            BOOST_REQUIRE_EQUAL(region.convert_offset(i), offset);
            ++offset;
        }
        BOOST_REQUIRE_NO_THROW(region.convert_offset(3 * size * count));
    }
    {
        size_t count1 = 2;
        file_region_type subregion1(2, 10);
        typename file_region_type::region_list subregions;
        subregions.push_back(subregion);
        subregions.push_back(subregion1);
        file_region_type region(3, subregions);
        offset_type offset = 0;
        size_t j = 0;
        for (size_t k = 0; k < 3; ++k)
        {
            for (size_t i = 0; i < count; ++i)
            {
                range_type result = region[j++];
                BOOST_REQUIRE_EQUAL(result.first, offset);
                BOOST_REQUIRE_EQUAL(result.second, offset + file_page_type::TOTAL_SIZE);
                offset += file_page_type::TOTAL_SIZE;
            }
            for (size_t i = 0; i < count1; ++i)
            {
                range_type result = region[j++];
                BOOST_REQUIRE_EQUAL(result.first, offset);
                BOOST_REQUIRE_EQUAL(result.second, offset + 2 * file_page_type::TOTAL_SIZE);
                offset += 2 * file_page_type::TOTAL_SIZE;
            }
        }
        BOOST_REQUIRE_THROW(region[j], bug_error);
    }
}

//==============================================================================
//  Check the file region
//==============================================================================
BOOST_AUTO_TEST_CASE(file_region_test)
{
    typedef file_page<8> file_page_type;
    test_file_region<file_page_type>();
}

//==============================================================================
//  Check the file region that has a file page with service data
//==============================================================================
BOOST_AUTO_TEST_CASE(file_region_with_service_data_test)
{
    typedef file_page<12, 4> file_page_type;
    test_file_region<file_page_type>();
}

//==============================================================================
//  Check the cache of a file region
//==============================================================================
template <typename T>
inline T make_file_regions(const size_type info_size, const size_type key_size,
    const size_type table_size)
{
    typedef T file_region_type;
    file_region_type region(1, info_size);
    file_region_type region_key(1, key_size);
    file_region_type region_table(1, table_size);
    typename file_region_type::region_list regions;
    regions.push_back(region_key);
    regions.push_back(region_table);
    file_region_type region_keytable(0, regions);
    region.add(region_keytable);
    return region;
}

BOOST_AUTO_TEST_CASE(cache_file_region_test)
{
    typedef file_page<1024> file_page_type;
    typedef file_region<file_page_type> file_region_type;
    file_region_type region = make_file_regions<file_region_type>(784, 32, 280000);
    size_t p[1024] = { 0 };
    for (size_t i = 0; i < 1024 * 1024; ++i)
    {
        if (i > 1024)
        {
            BOOST_CHECK_EQUAL(region.convert_offset(i - 1024), p[(i - 1024) % 1024]);
        }
        p[i % 1024] = region.convert_offset(i);
    }
}
