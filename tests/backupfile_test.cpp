#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE backupfile_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "ouroboros/cache.h"
#include "ouroboros/backupfile.h"

using namespace ouroboros;

#define TEST_FILE_NAME "test.dat"

typedef backup_file<1024, 8> file_type;

#define TEST_FULL_TRANSACTION
#include "cachefile_test.h"