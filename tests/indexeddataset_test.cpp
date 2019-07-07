#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE indexdataset_test
#include <boost/test/unit_test.hpp>

#include "ouroboros/key.h"
#include "ouroboros/datatable.h"
#include "ouroboros/dataset.h"
#include "ouroboros/interface.h"
#include "test.h"

typedef data_set<simple_key, record_type, index1, local_interface> dataset_type;

#include "dataset_test.h"