[![build and test](https://github.com/belyaev-ms/ouroboros/actions/workflows/cmake.yml/badge.svg)](https://github.com/belyaev-ms/ouroboros/actions/workflows/cmake.yml)
[![codecov](https://codecov.io/gh/belyaev-ms/ouroboros/branch/master/graph/badge.svg)](https://codecov.io/gh/belyaev-ms/ouroboros)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Ouroboros
Embedded NoSQL storage of data. This storage can use like round-robin database. **Ouroboros** aims to handle time series data such as sensors values, network bandwidth, temperatures or CPU load. The data is stored in a circular buffer based database, thus the system storage footprint remains constant over time. This storage stores data in a data table that has some depth and if number of records over the depth, the new record overwrites the oldest record.

## Help
Ouroboros provides access to a dataset that has a set of tables. In one dataset, tables have the same record structure and the same maximum number of records (the same depth). Access to a table is provided by key.

### Fields
The following field type are supported:
> - **FIELD_FLOAT**.
> - **FIELD_DOUBLE**.
> - **FIELD_UINT8**.
> - **FIELD_UINT16**.
> - **FIELD_UINT32**.
> - **FIELD_UINT64**.
> - **FIELD_INT8**.
> - **FIELD_INT16**.
> - **FIELD_INT32**.
> - **FIELD_INT64**.
> - **FIELD_STRING**.

### Record
If you want to use records that should have, for example, 3 fields, you should use the following type of record:
```сpp
typedef record3<FIELD_UINT32, FIELD_DOUBLE, FIELD_UINT8> record_type;
```

### Index
Records of a table can be indexed by single field.
> - **index_null** determines that the record doesn't have an indexed field.
> - **index1** determines that the record has an index by the *first* field.
> - **index2** determines that the record has an index by the *second* field.
> - etc.

### Key
The type of key determines where the table stores its indexes. The two types of key are supported:
> - **simple_key** determines the table stores its indexes in memory.
> - **tree_key**  determines the table stores its indexes in itself.

### Interface
Type of interface determines the behavior of a dataset. The two interface type are supported:
> - **local_interface** determines the dataset coud be used by a single process.
> - **shared_interface**  determines the dataset coud be used by several processes.

### Dataset
The dataset determines the number of tables stored and their depth.
For example, following data set has 10 tables that can store 1000 records:
```сpp
typedef dataset<simple_key, record_type, index_null, shared_interface> dataset_type;
dataset_type dataset("dbname", 10, 1000);
```

### Session
Session provides access to a table. A dataset has two types of session:
> - **session_rd** this session can provide only read operations.
> - **session_wr** this session can provide read and write operations.

Session of one table can't influnce to a session of another table, but there are the following rules for sessions of one tables:
> - If there is an active **session_rd**, you can open several new **session_rd**s, but you can't open any **sessions_wr**s.
> - If there is an active **session_wr**, you can't open any new **session_rd**s and any **sessions_wr**s.

Thus for a single table, read operations can be handled at one time but active write operations can be only one at one time.

For example, the following code works with a table that has a key equal to 0, and first gets count of stored records and then gets their:
```сpp
{
    dataset_type::serssion_rd session_rd = dataset.session_rd(0);
    const size_t count = session_rd->count();
    dataset_type::records_list records(count);
    session_rd->read_front(records);
}
```
Thus, a session restricts access to a single table in a dataset.
If there will be some exception when you use some kind of session, then all changes made by the session will be discarded.

### Transaction
Transaction, unlike a session, allows to lock the entire dataset. If there will be some exception when you use some transaction, then all changes made by the transaction will be discarded.
For example, if we need to add the new table:
```сpp
{
    typedef transaction<dataset_type> transaction_type;
    transaction_type transaction(dataset);
    if (dataset.table_exists(0))
    {
    	dataset.add_table(0);
    }
}
```

## Examples
This project has several examples, you can see them here: examples/ :)

## License
MIT License

Copyright (c) 2019 Mikhail Belyaev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
