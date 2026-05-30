/**
 * @file types.h
 * @brief NIMBU OS type definitions
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * This file contains fundamental type definitions used throughout
 * the NIMBU OS kernel. All types are defined for x86 32-bit architecture.
 */

#ifndef TYPES_H
#define TYPES_H

/* Integer types */
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long ulong_t;
typedef signed long long_t;

/* Pointer types */
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;

/* Boolean type */
typedef int bool_t;
#define TRUE 1
#define FALSE 0

/* Size types */
typedef uint32_t size_t;
typedef int32_t ssize_t;

/* Physical and Virtual addresses */
typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;

/* NULL pointer */
#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* TYPES_H */
