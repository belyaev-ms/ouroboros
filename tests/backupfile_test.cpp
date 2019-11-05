#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE backupfile_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "ouroboros/cache.h"
#include "ouroboros/backupfile.h"
#include "ouroboros/page.h"

using namespace ouroboros;

#define TEST_FILE_NAME "test.dat"

typedef file_page<1024> file_page_type;
typedef backup_file<file_page_type, 8> file_type;
typedef file_type::file_region_type file_region_type;

#define TEST_FULL_TRANSACTION
#include "cachefile_test.h"