#include <stdio.h>
#include <iostream>
#include <vector>
#include <boost/lexical_cast.hpp>
#if __APPLE__
#include <mach/mach_time.h>
#endif

#define OUROBOROS_LOG
#define OUROBOROS_INFO(msg)
#define OUROBOROS_ERROR(msg)
#define OUROBOROS_DEBUG(msg)

#include "ouroboros/treekey.h"
#include "ouroboros/treedatatable.h"
#include "ouroboros/dataset.h"
#include "ouroboros/transaction.h"
#include "ouroboros/sharedinterface.h"
#include "ouroboros/field_types.h"

using namespace ouroboros;

struct test_table_interface
{
    template <typename T> struct object_type : public shared_object<T> {};
    template <typename Key, typename Field>
    struct skey_list : public shared_map<Key, Field> {};
    typedef file_page<OUROBOROS_PAGE_SIZE, sizeof(journal_status_type)> file_page_type;
    typedef journal_file<file_page_type, OUROBOROS_PAGE_COUNT> file_type;
    struct locker_type : public shared_locker<mutex_lock>
    {
        locker_type(const std::string& name, count_type& scoped_count, count_type& sharable_count) :
            shared_locker<mutex_lock>(name, scoped_count, sharable_count)
        {}
        locker_type(lock_type& lock, count_type& scoped_count, count_type& sharable_count) :
            shared_locker<mutex_lock>(lock, scoped_count, sharable_count)
        {}
    };
    typedef gateway<boost::interprocess::interprocess_mutex> gateway_type;
};

typedef record3< FIELD_INT32, FIELD_FLOAT, FIELD_INT32 > record_type;
#if 0
struct test_interface : public base_interface<test_table_interface, indexed_table> {};
typedef data_set<simple_key, record_type, index1, test_interface> dataset_type;
#else
struct test_interface : public base_interface<test_table_interface, tree_data_table_adapter> {};
typedef data_set<tree_key, record_type, index1, test_interface> dataset_type;
#endif
typedef dataset_type::record_list record_list;
typedef dataset_type::session_read session_read;
typedef dataset_type::session_write session_write;

/**
 * Fill the records
 * @param records the records
 * @param count count of records
 * @param val initial value
 */
void fill_records(record_list& records, const size_t count, const size_t val)
{
    records.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        records.push_back(record_type(val + i, val + i + 1, val + i + 2));
    }
}

const char *progname = "speed_test";

/**
 * Get monotonic time
 * @return monotonic time
 */
const size_t time_us()
{
#if __APPLE__
    return mach_absolute_time();
#else
    struct timespec res = {0};
	clock_gettime(CLOCK_MONOTONIC, &res);
    return res.tv_sec * 1000000 + res.tv_nsec / 1000;
#endif
}

int main(int argc, char *argv[])
{
    std::string name = progname;
    size_t itrCount = 1;
    size_t tbl_count = 10;
    size_t rec_count = 1000;
    bool is_session = false;
    if (argc > 1)
    {
        const char *options = "n:t:r:i:s";
        int opt;
        while ((opt = getopt(argc, argv, options)) != -1)
        {
            switch (opt)
            {
                case 'n':
                    name = optarg;
                    break;
                case 't':
                    tbl_count = boost::lexical_cast<size_t>(optarg);
                    break;
                case 'r':
                    rec_count = boost::lexical_cast<size_t>(optarg);
                    break;
                case 'i':
                    itrCount = boost::lexical_cast<size_t>(optarg);
                    break;
                case 's':
                    is_session = true;
                    break;
            }
        }
    }
    std::cout << "The options:" << std::endl;
    std::cout << "\t count of repeats: " << itrCount << std::endl;
    std::cout << "\t count of tables:  " << tbl_count << std::endl;
    std::cout << "\t count of records: " << rec_count << std::endl;
    std::cout << "\t single session:   " << (is_session ? "yes" : "no") << std::endl;

    std::cout << std::endl;
    std::cout << "Test the ouroboros: " << std::endl;

    dataset_type::remove(name.c_str());
    dataset_type dataset(name.c_str(), tbl_count, rec_count);
    for (size_t itr = 0; itr < itrCount; ++itr)
    {
        std::cout << std::endl;
        std::cout << "Repeat: " << itr << std::endl;
        size_t wrTime = 0;
        size_t rdTime = 0;
        for (size_t index = 0; index < tbl_count; index++)
        {
            std::cout << "\tTable: " << index << std::endl;
            if (!dataset.table_exists(index))
            {
                dataset.add_table(index);
            }
            record_list wrList;
            fill_records(wrList, rec_count, rec_count * index);
            const size_t wrTime1 = time_us();
            if (is_session)
            {
                session_write session = dataset.session_wr(index);
                for (size_t i = 0; i < rec_count; i++)
                {
                    session->add(wrList[i]);
                }
            }
            else
            {
                for (size_t i = 0; i < rec_count; i++)
                {
                    dataset.session_wr(index)->add(wrList[i]);
                }
            }
            wrTime += time_us() - wrTime1;
            record_list rdList(rec_count);
            const size_t rdTime1 = time_us();
            if (is_session)
            {
                session_read session = dataset.session_rd(index);
                for (size_t i = 0; i < rec_count; i++)
                {
                    session->read(rdList[i], i);
                }
            }
            else
            {
                for (size_t i = 0; i < rec_count; i++)
                {
                    dataset.session_rd(index)->read(rdList[i], i);
                }
            }
            rdTime += time_us() - rdTime1;
            for (size_t i = 0; i < rec_count; i++)
            {
                if (wrList[i] != rdList[i])
                {
                    std::cout << "Error: the data is wrong" << std::endl;
                    return -1;
                }
            }
        }
        std::cout << "time of WR: " << wrTime << std::endl;
        std::cout << "time of RD: " << rdTime << std::endl;
        std::cout << "total time: " << (wrTime + rdTime) << std::endl;
    }
    return 0;
}

