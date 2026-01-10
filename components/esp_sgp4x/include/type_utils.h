#ifndef TYPE_UTILS_H
#define TYPE_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Helpers to view multi-byte integer values as byte arrays

typedef union {
    uint16_t value;
    uint8_t bytes[2];
} bytes_to_uint16_t;

#define BIT16_UINT8_BUFFER_SIZE 2
#define BIT24_UINT8_BUFFER_SIZE 3
#define BIT48_UINT8_BUFFER_SIZE 6
#define BIT64_UINT8_BUFFER_SIZE 8
#define BIT72_UINT8_BUFFER_SIZE 9

typedef uint8_t bit16_uint8_buffer_t[BIT16_UINT8_BUFFER_SIZE];
typedef uint8_t bit24_uint8_buffer_t[BIT24_UINT8_BUFFER_SIZE];
typedef uint8_t bit48_uint8_buffer_t[BIT48_UINT8_BUFFER_SIZE];
typedef uint8_t bit64_uint8_buffer_t[BIT64_UINT8_BUFFER_SIZE];
typedef uint8_t bit72_uint8_buffer_t[BIT72_UINT8_BUFFER_SIZE];

#ifdef __cplusplus
}
#endif

#endif // TYPE_UTILS_H
