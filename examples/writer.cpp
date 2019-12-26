#include <ctime>
#include <signal.h>
#include "ouroboros/field_types.h"
#include "ouroboros/record.h"
#include "ouroboros/dataset.h"
#include "ouroboros/transaction.h"
#include "ouroboros/sharedinterface.h"

using namespace ouroboros;

// type of a data record
typedef record3<
    FIELD_UINT32,   // time stamp
    FIELD_DOUBLE,   // value
    FIELD_UINT16    // status
> record_type;
// type of a data set
typedef data_set<simple_key, record_type, index_null, shared_interface> dataset_type;
// type of a transaction
typedef dataset_transaction<dataset_type> transaction_type;


static volatile sig_atomic_t terminated = 0;
bool is_terminated()
{
    return terminated;
}
static void termhandler(int signo)
{
    terminated = 1;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, termhandler);
    {
        struct timeval t;
        gettimeofday(&t, NULL);
        std::srand((t.tv_sec * 1000) + (t.tv_usec / 1000));
    }
    
    const std::string name = "db";      // the name of the data set
    const count_type tbl_count = 10;    // the count of tables in the data set
    const count_type rec_count = 1000;  // the count of records in a table
    
    // select a key for work
    dataset_type::key_type key = std::rand() % tbl_count;

    // open or create the dataset
    std::cout << "open/create dataset " << name << "...";
    dataset_type dataset(name.c_str(), tbl_count, rec_count);
    std::cout << "OK" << std::endl;

    {
        // open a transaction to add a new table to the dataset
        transaction_type transaction(dataset);
        if (dataset.table_exists(key))
        {
            std::cout << "attach table " << key << " ...OK" << std::endl;
        }
        else
        {
           std::cout << "table key = " << key << " not found" << std::endl;
           std::cout << "add table " << key << " ...";
           dataset.add_table(key);
           std::cout << "OK" << std::endl;
        }
    }

    // work cycle
    while (!is_terminated())
    {
        /* ... recive/calculate some values ... */
        const count_type count = std::rand() % rec_count; // count of values
        dataset_type::record_list records(count);
        for (count_type i = 0; i < count; ++i)
        {
            sleep(1);
            records[i].field1(std::time(nullptr));  // current time
            records[i].field2(std::rand());         // some value
            records[i].field3(std::rand());         // some status
        }
        
        // write the values to dataset
        dataset.session_wr(key)->add(records);
    }
    
    return 0;
}
