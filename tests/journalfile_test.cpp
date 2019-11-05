#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE journalfile_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "ouroboros/cache.h"
#include "ouroboros/journalfile.h"
#include "ouroboros/interface.h"
#include "ouroboros/page.h"

using namespace ouroboros;

#define TEST_FILE_NAME "test.dat"

typedef file_page<1024, sizeof(journal_status_type)> file_page_type;
typedef journal_file<file_page_type, 8> file_type;
typedef file_type::file_region_type file_region_type;

#define TEST_FULL_TRANSACTION
#include "cachefile_test.h"

//==============================================================================
//  Check for the rollback of transaction when the process crashes
//==============================================================================
BOOST_AUTO_TEST_CASE(recovery_test)
{
    file_type::remove(TEST_FILE_NAME);

    const size_t blockSize = file_type::CACHE_PAGE_SIZE;
    char outbuf[blockSize * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    {
        file_region_type file_region(0, sizeof(outbuf));
        file_type file(TEST_FILE_NAME, file_region);
        file.resize(sizeof(outbuf));

        file.start();
        for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
        {
            file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
        }
        file.stop();
    }

    {
        file_region_type file_region(0, sizeof(outbuf));
        file_type file(TEST_FILE_NAME, file_region);
        char stubbuf[blockSize * file_type::CACHE_PAGE_COUNT * 2];
        for (size_t i = 0; i < sizeof(outbuf); i++)
        {
            stubbuf[i] = std::rand();
        }

        file.start();
        for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
        {
            file.write(&stubbuf[i * blockSize], blockSize, i * blockSize);
            char inbuf[blockSize];
            if (i >= file_type::CACHE_PAGE_COUNT)
            {
                const size_t j = (i - file_type::CACHE_PAGE_COUNT) * blockSize;
                file.read(inbuf, sizeof(inbuf), j);
                BOOST_CHECK_EQUAL_COLLECTIONS(&stubbuf[j], &stubbuf[j + blockSize], inbuf, ARRAY_END(inbuf));
            }
        }
        // WITHOUT CANCEL
    }

    {
        file_region_type file_region(0, sizeof(outbuf));
        file_type file(TEST_FILE_NAME, file_region);
        file.init();
        char inbuf[sizeof(outbuf)];
        file.read(inbuf, sizeof(inbuf), 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
    }
}