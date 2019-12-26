#include "ouroboros/field_types.h"
#include "ouroboros/record.h"
#include "ouroboros/dataset.h"
#include "ouroboros/interface.h"

using namespace ouroboros;

// type of a data record
typedef record3<FIELD_UINT32, FIELD_INT64, FIELD_DOUBLE> record_type;

// type of a data set
typedef data_set<simple_key, record_type, index_null, local_interface> dataset_type;

int main(int argc, char *argv[])
{
    const std::string name = "sample";  // the name of the data set
    const count_type tbl_count = 10;    // the count of tables in the data set
    const count_type rec_count = 1000;  // the count of records in a table

    // create the data set
    dataset_type dataset(name.c_str(), tbl_count, rec_count);

    // add tables to the data set
    for (size_t key = 0; key < tbl_count; ++key)
    {
        dataset.add_table(key);
    }

    // add records to a table that has the key 0
    {
        const size_t key = 0;
        dataset_type::session_write session_wr = dataset.session_wr(key);
        for (count_type i = 0; i < rec_count; ++i)
        {
            const record_type record(i, i + 1, i + 2);
            session_wr->add(record);
        }
    }

    // read all records from a table that has the key 0
    {
        const size_t key = 0;
        dataset_type::session_read session_rd = dataset.session_rd(key);
        count_type count = session_rd->count();
        dataset_type::record_list records(count);
        session_rd->read(records, session_rd->beg_pos());
    }
}
