#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE treedatatable_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <vector>
#include "ouroboros/treekey.h"
#include "ouroboros/find.h"
#include "ouroboros/container.h"
#include "ouroboros/index.h"
#include "ouroboros/treedatatable.h"
#include "ouroboros/interface.h"
#include "test.h"

options_type options;
typedef tree_key skey_type;
typedef base_local_interface<tree_data_table_adapter> test_interface;
typedef indexed_record<record_type, index1> indexed_record_type;
typedef data_source<interface_table, indexed_record_type, test_interface> datasource_type;
typedef tree_data_table<interface_table, indexed_record_type, skey_type, test_interface> datatable_type;

#include "datatable_test.h"
//@todo need to add specific test for TreeDataTable