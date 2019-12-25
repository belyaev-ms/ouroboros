/**
 * The test tool for checking the use of one data set by different process:
 * -    a lot of process read a table from the data set (blocks of random size
 *      from random position);
 * -    a lot of process write a table to the data set (blocks of random size
 *      from random position);
 * -    read/write is performed using either global lock (whole data set is
 *      blocked) or using a session (only the table is blocked);
 * -    each record has a counter field;
 * -    the value of the new record counter must be one greater the previous
 *      one;
 */
#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>
#include <signal.h>

#include "ouroboros/treekey.h"
#include "ouroboros/find.h"
#include "ouroboros/treedatatable.h"
#include "ouroboros/dataset.h"
#include "ouroboros/transaction.h"
#include "ouroboros/sharedinterface.h"
#include "ouroboros/record.h"
#include "ouroboros/field_types.h"

#define OUROBOROS_TOOLS_LOG(msg) do { std::ofstream flog("ouroboros.log", std::ios::app); flog << msg << std::endl; } while(0)
#define OUROBOROS_TOOLS_MESSAGE(prefix, msg) OUROBOROS_TOOLS_LOG(prefix << "\t" \
            << OUROBOROS_SHORT_FILE << "::" << __FUNCTION__ << "(" << __LINE__ << "):\t" << msg)
#define OUROBOROS_TOOLS_INFO(msg)   OUROBOROS_TOOLS_MESSAGE("INFO:", msg)
#define OUROBOROS_TOOLS_ERROR(msg)  OUROBOROS_TOOLS_MESSAGE("ERROR:", msg)
#define OUROBOROS_TOOLS_DEBUG(msg)  OUROBOROS_TOOLS_MESSAGE("DEBUG:", msg)

using namespace ouroboros;

typedef record4<FIELD_INT32, FIELD_INT32, FIELD_FLOAT, FIELD_INT32> record_type;

struct test_table_interface
{
    template <typename T> struct object_type : public shared_object<T> {};
    typedef file_page<OUROBOROS_PAGE_SIZE, sizeof(journal_status_type)> file_page_type;
    typedef journal_file<file_page_type, OUROBOROS_PAGE_COUNT> file_type;
    template <typename Key, typename Field>
    struct skey_list : public shared_map<Key, Field> {};
    struct locker_type : public locker<mutex_lock>
    {
        locker_type(const char* name, count_type& scoped_count, count_type& sharable_count) :
            locker<mutex_lock>(name, scoped_count, sharable_count)
        {}
    };
};
struct test_interface : public base_interface<test_table_interface, tree_data_table_adapter> {};

typedef data_set<tree_key, record_type, index2, test_interface> dataset_type;
typedef dataset_type::record_list record_list;
typedef dataset_type::session_read session_read;
typedef dataset_type::session_write session_write;

typedef dataset_transaction<dataset_type> transaction_type;

class condition_transaction
{
public:
    explicit condition_transaction(dataset_type& dataset) :
        m_transact(s_enabled ? new transaction_type(dataset) : NULL)
    {
    }
    virtual ~condition_transaction()
    {
        if (m_transact != NULL)
        {
            delete m_transact;
        }
    }
    static bool s_enabled;
private:
    transaction_type *m_transact;
};

//static
bool condition_transaction::s_enabled = false;

static volatile sig_atomic_t terminated = 0;

bool is_terminated()
{
    return terminated;
}

static void termhandler(int signo)
{
    terminated = 1;
}

enum
{
    RET_OK = 0,     ///< ok
    RET_ERROR,      ///< unknown error
    RET_FAIL,       ///< fail (data is broken)
    RET_LOCK_RD,    ///< lock error for reading
    RET_LOCK_WR,    ///< lock error for writing
    RET_LOCK_CR     ///< lock error for creating
};

struct hand_error : public base_error { hand_error() : base_error() {} }; ///< hand-made error

const size_t lock_count = 10;   ///< maximum count of attemts to lock

/**
 * Execute the test
 * @param name the name of the data set
 * @param key the key of the table
 * @param tbl_count the count of the tables
 * @param rec_count the count of the records in a table
 * @param itr_count the count of test iterations
 * @param is_writer the sign of writing test
 * @param is_full the sign of full verification of records
 * @param is_except the sign of the exception handling test
 * @return the result of the executing
 */
const int exec_test(const std::string& name, const dataset_type::key_type& key,
    const size_t tbl_count, const size_t rec_count, const size_t itr_count,
    const bool is_writer, const bool is_full, const bool is_except)
{
    size_t except_counter = 0;
    size_t lock_counter = 0;
    const bool itr_enabled = itr_count > 0;
    const int pid = getpid();

    try
    {
        OUROBOROS_TOOLS_INFO("[START] : " << PR(pid) << PE(key));
        dataset_type dataset(name.c_str(), tbl_count, rec_count);
        {
            transaction_type transact(dataset);
            if (dataset.table_exists(key))
            {
                std::cout << "attach table " << key << " ...OK" << std::endl;
            }
            else
            {
               std::cout << "table key = " << key << " not found" << std::endl;
               std::cout << "create table " << key << " ...";
               dataset.add_table(key);
               std::cout << "OK" << std::endl;
            }
        }

        if (!is_writer)
        {
            size_t j = 0;
            while (!is_terminated())
            {
                std::cout << "============================" << std::endl;
                record_list records;
                try
                {
                    condition_transaction transact(dataset);
                    session_read session = dataset.session_rd(key);
                    typename dataset_type::table_type& table= static_cast<typename dataset_type::table_type&>(session.table());
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\tread beg " << PR(key) << "t = " << condition_transaction::s_enabled << ", " << &(table.skey()) << " " << PR(table.get_root()) << table.skey());
                    const count_type count = session->count();
                    if (count > 0)
                    {
                        const pos_type rbeg = session->beg_pos();
                        const pos_type rend = session->end_pos();
                        const pos_type root = session->get_root();
                        std::cout << "---\t" << rbeg << "\t" << rend << std::endl;
                        OUROBOROS_TOOLS_INFO(PR(pid) << "\tread " << PR(key) << PR(rbeg) << PR(rend) << PE(root));
                        pos_type beg = std::rand() % count;
                        pos_type end = std::rand() % (count - beg);
                        if (is_full)
                        {
                            beg = rbeg;
                            end = rend;
                        }
                        else
                        {
                            beg = session->inc_pos(rbeg, beg);
                            end = session->dec_pos(rend, end);
                        }
                        std::cout << "---\t" << beg << "\t" << end << std::endl;
                        OUROBOROS_TOOLS_INFO(PR(pid) << "\tread " << PR(key) << PR(beg) << PE(end));
                        pos_type i = beg;
                        do
                        {
                            record_type record;
                            session->read(record, i);
                            records.push_back(record);
                            i = session->inc_pos(i);
                        } while (i != end);
                        session->test();
                        if (is_except && (std::rand() % 10 == 0))
                        {
                            OUROBOROS_THROW_ERROR(hand_error, "hand-made exception");
                        }
                    }
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\tread end " << PR(key) << "t = " << condition_transaction::s_enabled << ", " << &(table.skey()) << " " << PR(table.get_root()) << table.skey());
                }
                catch (const hand_error& e)
                {
                    std::cout << "[HAND EXCEPT]" << std::endl;
                    OUROBOROS_TOOLS_INFO("[HAND EXCEPT] : " << PR(pid) << PR(key) << PE(except_counter));
                    if (++except_counter < lock_count)
                    {
                        continue;
                    }
                    throw;
                }
                catch (const ouroboros::lock_error& e)
                {
                    std::cout << "[LOCK RD]" << std::endl;
                    OUROBOROS_TOOLS_ERROR("[LOCK RD] : " << PR(pid) << PR(key) << PR(lock_counter) << PE(e));
                    usleep(100000);
                    if (lock_counter++ < lock_count)
                    {
                        continue;
                    }
                    return RET_LOCK_RD;
                }
                for (size_t i = 0; i < records.size(); ++i)
                {
                    std::cout << i << "\t> " << records[i] << std::endl;
                    if ((i < records.size() - 1) && (records[i].field1() + 1 != records[i + 1].field1()))
                    {
                        std::cout << "[FAIL]" << std::endl;
                        OUROBOROS_TOOLS_ERROR(PR(pid) << PR(key) << PR(records[i].field1()) << PE(records[i + 1].field1()));
                        for (size_t n = 0; n < records.size(); ++n)
                        {
                            OUROBOROS_TOOLS_DEBUG("\t" << records[n]);
                        }
                        return RET_FAIL;
                    }
                }
                std::cout << "============================ " << ++j << std::endl;
                if (itr_enabled && j > itr_count)
                {
                    std::cout << "[OK]" << std::endl;
                    return RET_OK;
                }
                lock_counter = 0;
                usleep(100000);
            }
        }
        else
        {
            size_t j = 0;
            while (!is_terminated())
            {
                std::cout << "============================" << std::endl;
                try
                {
                    condition_transaction transact(dataset);
                    session_write session = dataset.session_wr(key);
                    typename dataset_type::table_type &table = static_cast<typename dataset_type::table_type&>(session.table());
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\twrite beg " << PR(key) << "t = " << condition_transaction::s_enabled << ", " << &(table.skey()) << " " << PR(table.get_root()) << table.skey());
                    session->test();
                    const pos_type last = session->back_pos();
                    size_t num = 0;
                    if (last != NIL)
                    {
                        record_type record;
                        session->read(record, last);
                        num = record.field1() + 1;
                    }
                    const size_t count = std::rand() % (rec_count / 10);
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\twrite " << PR(key) << PR(last) << PR(num) << PE(count));
                    for (size_t i = 0 ; i < count; ++i)
                    {
                        record_type record(num++, std::rand(), key, pid);
                        std::cout << "add record = " << record << " ...";
                        session->add(record);
                        std::cout << "OK" << std::endl;
                    }
                    const pos_type rbeg = session->beg_pos();
                    const pos_type rend = session->end_pos();
                    const pos_type root = session->get_root();
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\twrite " << PR(key) << PR(rbeg) << PR(rend) << PE(root));
                    if (is_except && (std::rand() % 10 == 0))
                    {
                        OUROBOROS_THROW_ERROR(hand_error, "hand-made exception");
                    }
                    session.stop();
                    OUROBOROS_TOOLS_INFO(PR(pid) << "\twrite end " << PR(key) << "t = " << condition_transaction::s_enabled << ", " << &(table.skey()) << " " << PR(table.get_root()) << table.skey());
                }
                catch (const hand_error& e)
                {
                    std::cout << "[HAND EXCEPT]" << std::endl;
                    OUROBOROS_TOOLS_INFO("[HAND EXCEPT] : " << PR(pid) << PR(key) << PE(except_counter));
                    if (++except_counter < lock_count)
                    {
                        continue;
                    }
                    throw;
                }
                catch (const ouroboros::lock_error& e)
                {
                    std::cout << "[LOCK WR]" << std::endl;
                    OUROBOROS_TOOLS_ERROR("[LOCK WR] : " << PR(pid) << PR(key) << PR(lock_counter) << PE(e));
                    usleep(100000);
                    if (lock_counter++ < lock_count)
                    {
                        continue;
                    }
                    return RET_LOCK_WR;
                }
                std::cout << "============================ " << ++j << std::endl;
                if (itr_enabled && j > itr_count)
                {
                    std::cout << "[OK]" << std::endl;
                    return RET_OK;
                }
                lock_counter = 0;
                usleep(10);
            }
        }
        std::cout << "[EXIT]" << std::endl;
        OUROBOROS_TOOLS_INFO(PR(pid) << PE(key));
    }
    catch (const hand_error& e)
    {
        std::cout << "[HAND EXCEPT]" << std::endl;
        OUROBOROS_TOOLS_INFO("[HAND EXCEPT] : " << PR(pid) << PR(key) << PE(except_counter));
        return RET_OK;
    }
    catch (const ouroboros::lock_error& e)
    {
        if (0 == lock_counter)
        {
            std::cout << "[LOCK CR]" << std::endl;
            OUROBOROS_TOOLS_ERROR("[LOCK CR] : " << PR(pid) << PR(key) << PE(e));
            return RET_LOCK_CR;
        }
        else if (is_writer)
        {
            std::cout << "[LOCK WR]" << std::endl;
            OUROBOROS_TOOLS_ERROR("[LOCK WR] : " << PR(pid) << PR(key) << PE(e));
            return RET_LOCK_WR;
        }
        else
        {
            std::cout << "[LOCK RD]" << std::endl;
            OUROBOROS_TOOLS_ERROR("[LOCK RD] : " << PR(pid) << PR(key) << PE(e));
            return RET_LOCK_RD;
        }
    }
    catch (const base_exception& e)
    {
        std::cout << "[ERROR] : " << e << std::endl;
        OUROBOROS_TOOLS_ERROR(PR(pid) << e);
        return RET_ERROR;
    }
    catch (const std::exception& e)
    {
        std::cout << "[ERROR] : " << e.what() << std::endl;
        OUROBOROS_TOOLS_ERROR(PR(pid) << e.what());
        return RET_ERROR;
    }
    catch (...)
    {
        std::cout << "[ERROR]" << std::endl;
        OUROBOROS_TOOLS_ERROR(PE(pid));
        return RET_ERROR;
    }

    return RET_OK;
}

/**
 * Main function of the worker
 */
int main(int argc, char *argv[])
{
    const int pid = getpid();
    signal(SIGINT, termhandler);

    std::string name = "worker_transaction";
    size_t itrCount = 0;
    size_t tbl_count = 10;
    size_t rec_count = 1000;
    bool is_writer = false;
    bool is_full = false;
    bool is_except = false;
    bool is_pause = false;
    dataset_type::key_type key = 0;
    {
        struct timeval t;
        gettimeofday(&t, NULL);
        std::srand((t.tv_sec * 1000) + (t.tv_usec / 1000));
    }

    shared_memory::set_name("worker_transaction");

    if (argc > 1)
    {
        const char *options = "k:n:t:r:i:wfesp";
        int opt;
        while ((opt = getopt(argc, argv, options)) != -1)
        {
            switch (opt)
            {
                case 'n':
                    name = optarg;
                    break;
                case 'k':
                    key = boost::lexical_cast<size_t>(optarg);
                    break;
                case 't':
                    tbl_count = boost::lexical_cast<size_t>(optarg);
                    break;
                case 'r':
                    rec_count = boost::lexical_cast<size_t>(optarg);
                    break;
                case 'w':
                    is_writer = true;
                    break;
                case 'i':
                    itrCount = boost::lexical_cast<size_t>(optarg);
                    break;
                case 'f':
                    is_full = true;
                    break;
                case 'e':
                    is_except = true;
                    break;
                case 's':
                    condition_transaction::s_enabled = true;
                    break;
                case 'p':
                    is_pause = true;
                    break;
                default:
                    break;
            }
        }
    }

    while (is_pause && !is_terminated())
    {
        std::cout << "waiting for the signal to start ..." << std::endl;
        sleep(1);
    }
    terminated = 0;

    size_t lock_counter = 0;
    while (lock_counter++ < lock_count)
    {
        const int ret = exec_test(name, key, tbl_count, rec_count, itrCount, is_writer, is_full, is_except);
        if (ret != RET_LOCK_CR)
        {
            return ret;
        }
        OUROBOROS_TOOLS_ERROR("[LOCK CR] : " << PR(pid) << PR(key) << PE(lock_counter));
        usleep(100000);
    }
    std::cout << "[LOCK CR]" << std::endl;
    OUROBOROS_TOOLS_ERROR("[LOCK CR] : " << PR(pid) << PE(key));
    return RET_LOCK_CR;
}

