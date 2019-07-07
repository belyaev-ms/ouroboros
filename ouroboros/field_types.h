/**
 * @file   field_types.h
 * The kinds of fields
 */

#ifndef OUROBOROS_FIELD_TYPES_H
#define	OUROBOROS_FIELD_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include "ouroboros/field.h"

namespace ouroboros
{

typedef data_field<float>       FIELD_FLOAT;
typedef data_field<double>      FIELD_DOUBLE;
typedef data_field<uint8_t>     FIELD_UINT8;
typedef data_field<uint16_t>    FIELD_UINT16;
typedef data_field<uint32_t>    FIELD_UINT32;
typedef data_field<uint64_t>    FIELD_UINT64;
typedef data_field<int8_t>      FIELD_INT8;
typedef data_field<int16_t>     FIELD_INT16;
typedef data_field<int32_t>     FIELD_INT32;
typedef data_field<int64_t>     FIELD_INT64;

#define FIELD_STRING string_field

}   //namespace ouroboros

#endif	/* OUROBOROS_FIELD_TYPES_H */

