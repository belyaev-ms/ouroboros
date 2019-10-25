#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))
#define ARRAY_END(x)	(x + ARRAY_SIZE(x))
//==============================================================================
//  Check for an exception if a file has incorrect parameters
//==============================================================================
BOOST_AUTO_TEST_CASE(simple_error_test)
{
    file_type::remove(TEST_FILE_NAME);
    file_type file(TEST_FILE_NAME);
    BOOST_CHECK_THROW(file.stop(), ouroboros::bug_error);
    BOOST_CHECK_THROW(file.cancel(), ouroboros::bug_error);
    BOOST_CHECK_THROW(file.cancel(), ouroboros::bug_error);
    file.start();
    BOOST_CHECK_THROW(file.start(), ouroboros::bug_error);
}

//==============================================================================
//  Check for R/W operations
//  W - whole file
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.reset();
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - whole file
//  R - in parts of size CACHE_PAGE_SIZE
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_page_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    char inbuf[file_page_type::DATA_SIZE];
    for (size_t i = 0; i < sizeof(outbuf) /  sizeof(inbuf); i++)
    {
        file.read(inbuf, sizeof(inbuf), i * sizeof(inbuf));
        char *beg = outbuf + i * sizeof(inbuf);
        char *end = beg + sizeof(inbuf);

        BOOST_CHECK_EQUAL_COLLECTIONS(beg, end, inbuf, ARRAY_END(inbuf));
    }
}

//==============================================================================
//  Check for R/W operations
//  W - whole file
//  R - in parts of size CACHE_PAGE_SIZE / 2
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_paged2_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    char inbuf[file_page_type::DATA_SIZE / 2];
    for (size_t i = 0; i < sizeof(outbuf) /  sizeof(inbuf); i++)
    {
        file.read(inbuf, sizeof(inbuf), i * sizeof(inbuf));
        char *beg = outbuf + i * sizeof(inbuf);
        char *end = beg + sizeof(inbuf);

        BOOST_CHECK_EQUAL_COLLECTIONS(beg, end, inbuf, ARRAY_END(inbuf));
    }
}

//==============================================================================
//  Check for R/W operations
//  W - whole file
//  R - in parts of size CACHE_PAGE_SIZE * 2
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_pagep2_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    char inbuf[file_page_type::DATA_SIZE * 2];
    for (size_t i = 0; i < sizeof(outbuf) /  sizeof(inbuf); i++)
    {
        file.read(inbuf, sizeof(inbuf), i * sizeof(inbuf));
        char *beg = outbuf + i * sizeof(inbuf);
        char *end = beg + sizeof(inbuf);

        BOOST_CHECK_EQUAL_COLLECTIONS(beg, end, inbuf, ARRAY_END(inbuf));
    }
}

//==============================================================================
//  Check for R/W operations
//  W - whole file
//  R - in parts of variable size from 1 to whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_pg_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2 + 100];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    for (size_t count = 1; count <= sizeof(outbuf); count++)
    {
        char inbuf[sizeof(outbuf)];
        memset(inbuf, 0, sizeof(inbuf));
        for (size_t i = 0; true; i++)
        {
            char *beg = outbuf + i * count;
            char *end = beg + count;
            if (beg == ARRAY_END(outbuf))
            {
                break;
            }
            if (end > ARRAY_END(outbuf))
            {
                size_t size = ARRAY_END(outbuf) - beg;
                file.read(inbuf, ARRAY_END(outbuf) - beg, i * count);
                BOOST_CHECK_EQUAL_COLLECTIONS(beg, ARRAY_END(outbuf), inbuf, inbuf + size);
                break;
            }
            else
            {
                file.read(inbuf, count, i * count);
                BOOST_CHECK_EQUAL_COLLECTIONS(beg, end, inbuf, inbuf + count);
            }
        }
    }
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_page_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE;
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE / 2
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_paged2_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE / 2;
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE * 2
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_pagep2_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE * 2;
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE (with Transaction)
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_page_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE;

    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    file.stop();

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));

    memset(inbuf, 0, sizeof(inbuf));
    file.start();
    file.read(inbuf, sizeof(inbuf), 0);
    file.stop();

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE / 2 (with Transaction)
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_paged2_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE / 2;

    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    file.stop();

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations
//  W - in parts of size CACHE_PAGE_SIZE * 2 (with Transaction)
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_pagep2_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE * 2;

    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    file.stop();

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations (in two stages)
//  W - whole file
//  R - whole file
//  W - in parts of size CACHE_PAGE_SIZE / 2 (with Transaction)
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_full_wr_paged2_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    BOOST_TEST_MESSAGE("write to file");
    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    file.write(outbuf, sizeof(outbuf), 0);

    BOOST_TEST_MESSAGE("read from file");
    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));

    for (size_t i = sizeof(outbuf) / 2; i < sizeof(outbuf); i++)
    {
        outbuf[i] = std::rand() % 100;
    }

    BOOST_TEST_MESSAGE("block write to file");
    const size_t blockSize = file_page_type::DATA_SIZE / 2;
    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    file.stop();

    BOOST_TEST_MESSAGE("read from file");
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for R/W operations (in two stages)
//  W - whole file
//  R - whole file
//  W - a byte in each pages (with Transaction)
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wr_full_rd_full_wr_one_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];

    {
        for (size_t i = 0; i < sizeof(outbuf); i++)
        {
            outbuf[i] = i;
        }
        BOOST_TEST_MESSAGE("write to file");
        file_type file(TEST_FILE_NAME);
        file.resize(sizeof(outbuf));
        file.write(outbuf, sizeof(outbuf), 0);

        BOOST_TEST_MESSAGE("read from file");
        char inbuf[sizeof(outbuf)];
        file.read(inbuf, sizeof(inbuf), 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
    }

    {
        for (size_t i = file_page_type::DATA_SIZE / 2; i < sizeof(outbuf); i += file_page_type::DATA_SIZE)
        {
            outbuf[i] = std::rand() % 100;
        }

        BOOST_TEST_MESSAGE("write 1 byte to file");
        file_type file(TEST_FILE_NAME);
        file.resize(sizeof(outbuf));
        file.start();
        for (size_t i = file_page_type::DATA_SIZE / 2; i < sizeof(outbuf); i += file_page_type::DATA_SIZE)
        {
            file.write(&outbuf[i], 1, i);
        }
        file.stop();

        BOOST_TEST_MESSAGE("read from file");
        char inbuf[sizeof(outbuf)];
        file.read(inbuf, sizeof(inbuf), 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
    }
}

#ifdef TEST_FULL_TRANSACTION
//==============================================================================
//  Check for R/W operations (in two stages)
//  { Transaction
//      W - in parts of size CACHE_PAGE_SIZE
//      R - in parts of size CACHE_PAGE_SIZE with a lag on the size of the cache
//  }
//  R - whole file
//==============================================================================
BOOST_AUTO_TEST_CASE(wrrd_page_TR_rd_full_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE;

    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
        char inbuf[blockSize];
        if (i >= file_type::CACHE_PAGE_COUNT)
        {
            const size_t j = (i - file_type::CACHE_PAGE_COUNT) * blockSize;
            file.read(inbuf, sizeof(inbuf), j);
            BOOST_CHECK_EQUAL_COLLECTIONS(&outbuf[j], &outbuf[j + blockSize], inbuf, ARRAY_END(inbuf));
        }
    }
    file.stop();

    char inbuf[sizeof(outbuf)];
    file.read(inbuf, sizeof(inbuf), 0);

    BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
}

//==============================================================================
//  Check for the rollback of transaction
//==============================================================================
BOOST_AUTO_TEST_CASE(rollback_test)
{
    file_type::remove(TEST_FILE_NAME);

    char outbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
    for (size_t i = 0; i < sizeof(outbuf); i++)
    {
        outbuf[i] = i;
    }

    file_type file(TEST_FILE_NAME);
    file.resize(sizeof(outbuf));
    const size_t blockSize = file_page_type::DATA_SIZE;

    file.start();
    for (size_t i = 0; i < sizeof(outbuf) /  blockSize; i++)
    {
        file.write(&outbuf[i * blockSize], blockSize, i * blockSize);
    }
    file.stop();

    {
        char inbuf[sizeof(outbuf)];
        file.read(inbuf, sizeof(inbuf), 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
    }

    char stubbuf[file_page_type::DATA_SIZE * file_type::CACHE_PAGE_COUNT * 2];
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
    file.cancel();

    {
        char inbuf[sizeof(outbuf)];
        file.read(inbuf, sizeof(inbuf), 0);

        BOOST_CHECK_EQUAL_COLLECTIONS(outbuf, ARRAY_END(outbuf), inbuf, ARRAY_END(inbuf));
    }
}
#endif