#ifndef __LIB_TYPES_H__
#define __LIB_TYPES_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Represents true-or-false values */
typedef int bool;

/* Explicitly-sized versions of integer types */
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

/* *
 * We use pointer types to represent addresses,
 * uintptr_t to represent the numerical values of addresses.
 * */
typedef long intptr_t;
typedef unsigned long uintptr_t;

/* size_t is used for memory object sizes */
typedef uintptr_t size_t;

#endif /* !__LIBS_TYPES_H__ */
