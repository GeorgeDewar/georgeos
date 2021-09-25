#include "libsys.h"

/* TYPE DEFINITIONS */
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

/* MAXIMUMS */
#define UINT8_T_MAX     0xFF
#define UINT16_T_MAX    0xFFFF
#define UINT32_T_MAX    0xFFFFFFFF

/* BUILTIN FUNCTIONS */
#define va_start(v,l)        __builtin_va_start(v,l)
#define va_end(v)        __builtin_va_end(v)
#define va_arg(v,l)        __builtin_va_arg(v,l)
#define va_copy(d,s)        __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

/* SPECIAL FILES */
#define stdout      0
#define stderr      1

/* Memory management & manipulation */
void *malloc(uint32_t size);
void free(void *ptr);
void memset(uint8_t* source, uint8_t value, uint32_t length);
void memcpy(uint8_t* source, uint8_t* dest, uint32_t length);

/** A filesystem-agnostic representation of a directory entry (file or directory) */
typedef struct {
    char filename[256];

    uint32_t location_on_disk;
    uint32_t file_size;

    // Bit field
    unsigned char read_only      : 1;
    unsigned char hidden         : 1;
    unsigned char system         : 1;
    unsigned char directory      : 1;
    unsigned char archive        : 1;
} DirEntry;
