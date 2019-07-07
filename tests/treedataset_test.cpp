#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE treedataset_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/treekey.h"
#include "ouroboros/treedatatable.h"
#include "ouroboros/dataset.h"
#include "ouroboros/interface.h"
#include "test.h"

typedef base_local_interface<tree_data_table_adapter> test_interface;
typedef data_set<tree_key, record_type, index1, test_interface> dataset_type;

#include "dataset_test.h"

