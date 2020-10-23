#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE fragsource_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <vector>
#include "ouroboros/key.h"
#include "ouroboros/controlblock.h"
#include "ouroboros/find.h"
#include "ouroboros/container.h"
#include "ouroboros/index.h"
#include "ouroboros/datatable.h"
#include "ouroboros/interface.h"
#include "test.h"

options_type options(0, record_type::static_size());
typedef simple_key skey_type;
typedef control_block<skey_type, local_interface> controlblock_type;
typedef data_source<interface_table, record_type, local_interface> datasource_type;
typedef data_table<interface_table, record_type, controlblock_type> datatable_type;

#include "datatable_test.h"