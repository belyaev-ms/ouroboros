#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE dataset_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/key.h"
#include "ouroboros/dataset.h"
#include "ouroboros/interface.h"
#include "test.h"

typedef data_set<simple_key, record_type, index_null, local_interface> dataset_type;

#include "dataset_test.h"