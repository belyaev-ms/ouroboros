#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE cachefile_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "ouroboros/cache.h"
#include "ouroboros/cachefile.h"

using namespace ouroboros;

#define TEST_FILE_NAME "test.dat"

typedef cache_file<1024, 8> file_type;

#include "cachefile_test.h"