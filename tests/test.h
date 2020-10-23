#ifndef OUROBOROS_TEST_H
#define	OUROBOROS_TEST_H

#include "ouroboros/record.h"
#include "ouroboros/field_types.h"

using namespace ouroboros;
typedef record3< FIELD_INT32, FIELD_FLOAT, FIELD_STRING<64> > record_type;

/**
 * Fill the records
 * @param records the records
 * @param count count of records
 * @param val initial value
 */
template <typename Records>
static void fill_records(Records& records, const size_t count, const size_t val)
{
    const std::string str("test");
    records.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        records.push_back(record_type(val + i, val + i + 1, str));
    }
}

#endif	/* OUROBOROS_TEST_H */

