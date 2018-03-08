/* See LICENSE file for copyright and license details. */

#ifndef DONT_INCLUDE_FLOAT
# define PROCESS process_lf
# define TYPE double
# define SCAN_TYPE "lf"
# define PRINT_CAST double
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
#endif

#ifndef DONT_INCLUDE_DOUBLE
# define PROCESS process_f
# define TYPE float
# define SCAN_TYPE "f"
# define PRINT_CAST double
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
#endif

#ifdef INCLUDE_DOUBLE_LONG
# define PROCESS process_llf
# define TYPE long double
# define SCAN_TYPE "Lf"
# define PRINT_CAST long double
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
#endif

#ifdef INCLUDE_UINT8
# define PROCESS process_u8
# define TYPE uint8_t
# define SCAN_TYPE SCNu8
# define PRINT_CAST unsigned
# define INTEGER_TYPE
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
# undef INTEGER_TYPE
#endif

#ifdef INCLUDE_UINT16
# define PROCESS process_u16
# define TYPE uint16_t
# define SCAN_TYPE SCNu16
# define PRINT_CAST unsigned
# define INTEGER_TYPE
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
# undef INTEGER_TYPE
#endif

#ifdef INCLUDE_UINT32
# define PROCESS process_u32
# define TYPE uint32_t
# define SCAN_TYPE SCNu32
# define PRINT_CAST uint32_t
# define INTEGER_TYPE
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
# undef INTEGER_TYPE
#endif

#ifdef INCLUDE_UINT64
# define PROCESS process_u64
# define TYPE uint64_t
# define SCAN_TYPE SCNu64
# define PRINT_CAST uint64_t
# define INTEGER_TYPE
# include FILE
# undef PROCESS
# undef TYPE
# undef SCAN_TYPE
# undef PRINT_CAST
# undef INTEGER_TYPE
#endif
