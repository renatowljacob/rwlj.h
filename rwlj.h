/*
 *  Acknowledgements: this library is hugely inspired by handmade hero, gb.h,
 *  odin, the raddebugger codebase, vkrajacic's own library excerpt at BSC and
 *  Tsoding.
 *
 *  The functionality revolves around Linux because that's what I use :P
 *  although I intend to extend it to Windows and BSD as well.
 *
 *  Wouldn't recommend using it because it's for my own personal use (and the
 *  code probably sucks too), so use it at your own risk.
 */

#ifndef RWLJ_H
#define RWLJ_H

#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
 *
 *  DECLARATION
 *
 */

/*
 *  Types, Utility Functions and Macros
 */

#define rwlj_global     static
#define rwlj_internal   static
#define rwlj_persistent static

#define true  (0 == 0)
#define false (0 != 0)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 bool;
typedef bool b8;
typedef i16 b16;
typedef i32 b32;

typedef intptr_t intptr;
typedef uintptr_t uintptr;
typedef ptrdiff_t isize;
typedef size_t usize;

typedef float f32;
typedef double f64;

#define U8_MIN 0u
#define U8_MAX 0xffu
#define I8_MIN (-0x7f - 1)
#define I8_MAX 0x7f

#define U16_MIN 0u
#define U16_MAX 0xffffu
#define I16_MIN (-0x7fff - 1)
#define I16_MAX 0x7fff

#define U32_MIN 0u
#define U32_MAX 0xffffffffu
#define I32_MIN (-0x7fffffff - 1)
#define I32_MAX 0x7fffffff

#define U64_MIN 0ull
#define U64_MAX 0xffffffffffffffffull
#define I64_MIN (-0x7fffffffffffffffll - 1)
#define I64_MAX 0x7fffffffffffffffll

#define rwlj_concat(x, y)  x##y
#define rwlj_concat_(x, y) x##_##y

#define rwlj_cast(T)              (T)
#define rwlj_size_of(x)           (isize)(sizeof(x))
#define rwlj_offset_of(T, member) ((isize) & (((T *)0)->member))
#define rwlj_align_of(T)                                                       \
        ((isize)rwlj_offset_of(                                                \
                struct {                                                       \
                        char c;                                                \
                        T member;                                              \
                },                                                             \
                member                                                         \
        ))
#define rwlj_containerof(T, object, member)                                    \
        rwlj_cast(void *)(rwlj_cast(intptr)(object) - rwlj_offset_of(T, member))

#define rwlj_size_of_array(a) (rwlj_size_of(a) / rwlj_size_of(0 [a]))

#define rwlj_bit(n) (1 << n)
#define rwlj_kb(n)  (n << 10)
#define rwlj_mb(n)  (n << 20)
#define rwlj_gb(n)  (n << 30)

#define rwlj_swap(T, x, y)                                                     \
        do {                                                                   \
                T __temp = (x);                                                \
                (x) = (y);                                                     \
                (y) = __temp;                                                  \
        } while (false)

#define rwlj_max(x, y)              (x > y ? x : y)
#define rwlj_min(x, y)              (x < y ? x : y)
#define rwlj_clamp(x, lower, upper) (rwlj_max(rwlj_min(x, upper), lower))

#define rwlj_abs(x) (x >= 0 ? x : -(x))

#define rwlj_truncate(x) (rwlj_cast(i64) x)
#define rwlj_floor(x)    (rwlj_truncate(x > 0.0f ? x : x - 1.0f))
#define rwlj_ceil(x)     (rwlj_truncate(x < 0.0f ? x : x + 1.0f))
#define rwlj_round(x) (x >= 0.0f ? rwlj_ceil(x - 0.5f) : rwlj_floor(x + 0.5f))

#define rwlj_align_pow2(x, b) ((x + b - 1) & (~(b - 1)))
#define rwlj_is_pow2(x)       ((x & (x - 1)) == 0)

#define rwlj_is_nil(ptr, nil) (ptr == NULL || ptr == nil)

#define rwlj_is_space(c)                                                       \
        (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||       \
         c == '\v')
#define rwlj_is_upper(c)        (c >= 'A' && c <= 'Z')
#define rwlj_is_lower(c)        (c >= 'a' && c <= 'z')
#define rwlj_to_upper(c)        (rwlj_is_lower(c) ? c - 32 : c)
#define rwlj_to_lower(c)        (rwlj_is_upper(c) ? c + 32 : c)
#define rwlj_is_alpha(c)        (rwlj_is_upper(c) || rwlj_is_lower(c))
#define rwlj_is_digit(c)        (c >= '0' && c <= '9')
#define rwlj_is_alphanumeric(c) (rwlj_is_alpha(c) || rwlj_is_digit(c))
#define rwlj_is_digit_hex(c)                                                   \
        (rwlj_is_digit(c) ||                                                   \
         (rwlj_to_upper(c) >= 'A' && rwlj_to_upper(c) <= 'F'))

/*
 *  Memory Allocators
 */

/* arena */

#define RWLJ_DEFAULT_ALIGNMENT sizeof(void *)

typedef enum rwlj_arena_kind {
        RWLJ_ARENA_STATIC,
        RWLJ_ARENA_GROWING,
        RWLJ_ARENA_BUFFER,
} rwlj_arena_kind;
typedef struct rwljArena {
        u8 *backing_buf;
        usize total_size;
        usize allocated_size;
        rwlj_arena_kind kind;
} rwljArena;

void
rwlj_arena_init(rwljArena *arena, void *mem, usize size, rwlj_arena_kind kind);

#define rwlj_arena_static(arena)                                               \
        rwlj_arena_init(arena, NULL, 0, RWLJ_ARENA_STATIC)
#define rwlj_arena_static_size(arena, size)                                    \
        rwlj_arena_init(arena, NULL, size, RWLJ_ARENA_STATIC)

#define rwlj_arena_growing(arena)                                              \
        rwlj_arena_init(arena, NULL, 0, RWLJ_ARENA_GROWING)
#define rwlj_arena_growing_starting_size(arena, starting_size)                 \
        rwlj_arena_init(arena, NULL, starting_size, RWLJ_ARENA_GROWING)

#define rwlj_arena_backing_buf(arena, backing_buf, size)                       \
        rwlj_arena_init(arena, backing_buf, size, RWLJ_ARENA_BUFFER)

void *rwlj_arena_alloc_aligned(rwljArena *arena, usize size, usize alignment);
#define rwlj_arena_alloc(arena, size)                                          \
        rwlj_arena_alloc_aligned(arena, size, RWLJ_DEFAULT_ALIGNMENT)

void *rwlj_arena_realloc_aligned(
        rwljArena *arena,
        void *old_mem,
        usize old_size,
        usize new_size,
        usize alignment
);
#define rwlj_arena_realloc(arena, old_mem, old_size, new_size)                 \
        rwlj_arena_realloc_aligned(                                            \
                arena, old_mem, old_size, new_size, RWLJ_DEFAULT_ALIGNMENT     \
        )

void rwlj_arena_free_all(rwljArena *arena);
void rwlj_arena_destroy(rwljArena *arena);

typedef struct rwljArenaTemp {
        rwljArena *arena;
        usize allocated_size;
} rwljArenaTemp;

void rwlj_arena_temp_init(rwljArenaTemp *arena_temp, rwljArena *arena);
void rwlj_arena_temp_free_all(rwljArenaTemp *arena_temp);

/*
 *  Data Structures
 */

/* slices */

typedef enum rwlj_slice_kind {
        RWLJ_SLICE_STATIC,  // for wrapping C arrays
        RWLJ_SLICE_GROWING, // useful for growing arrays in the stack
} rwlj_slice_kind;
#define GENERIC_SLICE(T, name)                                                 \
        typedef struct rwlj_concat(rwljSlice, name) {                          \
                T *data;                                                       \
                isize count;                                                   \
                isize capacity;                                                \
                rwlj_slice_kind kind;                                          \
        } rwlj_concat(rwljSlice, name)

GENERIC_SLICE(i8, I8);
GENERIC_SLICE(i16, I16);
GENERIC_SLICE(i32, I32);
GENERIC_SLICE(i64, I64);
GENERIC_SLICE(u8, U8);
GENERIC_SLICE(u16, U16);
GENERIC_SLICE(u32, U32);
GENERIC_SLICE(u64, U64);
GENERIC_SLICE(f32, F32);
GENERIC_SLICE(f64, F64);

#define rwlj_slice_get(slice, index)                                           \
        (index >= 0 && index < (slice)->count ? (slice)->data[index] : 0)
#define rwlj_slice_set(slice, index, value)                                    \
        (index >= 0 && index < (slice)->count ? (slice)->data[index] = value   \
                                              : 0)
#define rwlj_slice_unpack(slice) (slice)->data, (slice)->count
#define rwlj_slice_unpack_void(slice)                                          \
        (slice)->data, (slice)->count, rwlj_size_of((slice)->data[0])

#define rwlj_slice_pack(slice_T, pointer_to_data, count_)                      \
        (slice_T)                                                              \
        {                                                                      \
                .count = (count_), .data = (pointer_to_data),                  \
        }

#define rwlj_gslice_append(slice, value)                                       \
        do {                                                                   \
                if ((slice)->count < (slice)->capacity &&                      \
                    (slice)->kind == RWLJ_SLICE_GROWING) {                     \
                        (slice)->data[(slice)->count++] = (value);             \
                }                                                              \
        } while (false)

#define rwlj_gslice_preppend(slice, value)                                     \
        do {                                                                   \
                if ((slice)->count >= (slice)->capacity ||                     \
                    (slice)->kind != RWLJ_SLICE_GROWING) {                     \
                        break;                                                 \
                }                                                              \
                                                                               \
                memmove(&(slice)->data[1],                                     \
                        &(slice)->data[0],                                     \
                        rwlj_cast(usize)(                                      \
                                (slice)->count *                               \
                                rwlj_size_of((slice)->data[0])                 \
                        ));                                                    \
                (slice)->data[0] = (value);                                    \
                (slice)->count += 1;                                           \
        } while (false)

#define rwlj_gslice_pop(slice)                                                 \
        ((slice)->count > 0 && (slice)->kind == RWLJ_SLICE_GROWING             \
                 ? (slice)->data[--(slice)->count]                             \
                 : 0)

#define rwlj_gslice_remove_unordered(slice, index)                             \
        do {                                                                   \
                if ((index) < (slice)->count && (index) >= 0 &&                \
                    (slice)->count > 0) {                                      \
                        (slice)->data[index] =                                 \
                                (slice)->data[--(slice)->count];               \
                }                                                              \
        } while (false)

#define rwlj_gslice_remove_ordered(slice, index)                               \
        do {                                                                   \
                if ((index) < (slice)->count && (index) >= 0 &&                \
                    (slice)->count > 0) {                                      \
                        (slice)->data[index] =                                 \
                                (slice)->data[--(slice)->count];               \
                        memmove(&(slice)->data[index],                         \
                                &(slice)->data[index + 1],                     \
                                rwlj_cast(usize)(                              \
                                        ((slice)->count - index) *             \
                                        rwlj_size_of((slice)->data[0])         \
                                ));                                            \
                        (slice)->count -= 1;                                   \
                }                                                              \
        } while (false)

/* dynamic arrays */

typedef enum rwlj_array_kind {
        RWLJ_ARRAY_GROWING,
        RWLJ_ARRAY_FIXED,
} rwlj_array_kind;
#define GENERIC_ARRAY(T, name)                                                 \
        typedef struct rwlj_concat(rwljArray, name) {                          \
                rwljArena *arena;                                              \
                T *data;                                                       \
                isize count;                                                   \
                isize capacity;                                                \
                rwlj_array_kind kind;                                          \
        } rwlj_concat(rwljArray, name)

GENERIC_ARRAY(i8, I8);
GENERIC_ARRAY(i16, I16);
GENERIC_ARRAY(i32, I32);
GENERIC_ARRAY(i64, I64);
GENERIC_ARRAY(u8, U8);
GENERIC_ARRAY(u16, U16);
GENERIC_ARRAY(u32, U32);
GENERIC_ARRAY(u64, U64);
GENERIC_ARRAY(f32, F32);
GENERIC_ARRAY(f64, F64);

#define RWLJ_GROW_FORMULA(capacity) (8 + capacity * 2)

#define rwlj_array_init_reserve_with_kind(array, arena_, capacity_, kind_)     \
        do {                                                                   \
                (array)->arena = (arena_);                                     \
                (array)->count = 0;                                            \
                (array)->capacity = capacity_;                                 \
                (array)->kind = kind_;                                         \
                (array)->data = rwlj_arena_alloc(                              \
                        arena_, rwlj_cast(usize)(array)->capacity              \
                );                                                             \
        } while (false)

#define rwlj_array_init(array, arena)                                          \
        rwlj_array_init_reserve_with_kind(                                     \
                array, arena, RWLJ_GROW_FORMULA(0), RWLJ_ARRAY_GROWING         \
        )

#define rwlj_array_init_reserve(array, arena, capacity)                        \
        rwlj_array_init_reserve_with_kind(                                     \
                array, arena, capacity, RWLJ_ARRAY_GROWING                     \
        )

#define rwlj_array_init_fixed(array, arena, capacity)                          \
        rwlj_array_init_reserve_with_kind(                                     \
                array, arena, capacity, RWLJ_ARRAY_FIXED                       \
        )

#define rwlj_array_get(array, index)        rwlj_slice_get(array, index)
#define rwlj_array_set(array, index, value) rwlj_slice_set(array, index, value)

#define rwlj_array_append(array, value)                                        \
        do {                                                                   \
                if ((array)->count < (array)->capacity) {                      \
                        (array)->data[(array)->count++] = (value);             \
                }                                                              \
        } while (false)

#define rwlj_array_preppend(array, value)                                      \
        do {                                                                   \
                if ((array)->count >= (array)->capacity) {                     \
                        break;                                                 \
                }                                                              \
                                                                               \
                memmove(&(array)->data[1],                                     \
                        &(array)->data[0],                                     \
                        rwlj_cast(usize)(                                      \
                                (array)->count *                               \
                                rwlj_size_of((array)->data[0])                 \
                        ));                                                    \
                (array)->data[0] = (value);                                    \
                (array)->count += 1;                                           \
        } while (false)

#define rwlj_array_pop(array)                                                  \
        ((array)->count > 0 ? (array)->data[--(array)->count] : 0)

#define rwlj_array_remove_unordered(array, index)                              \
        rwlj_gslice_remove_unordered(array, index)
#define rwlj_array_remove_ordered(array, index)                                \
        rwlj_gslice_remove_ordered(array, index)

#define rwlj_array_reserve(array, size)                                        \
        do {                                                                   \
                (array)->data = rwlj_arena_realloc(                            \
                        (array)->arena,                                        \
                        (array)->data,                                         \
                        rwlj_cast(usize)(array)->capacity,                     \
                        size                                                   \
                );                                                             \
                                                                               \
                if ((array)->data == NULL) {                                   \
                        (array)->count = 0;                                    \
                        (array)->capacity = 0;                                 \
                } else {                                                       \
                        (array)->capacity = (size);                            \
                }                                                              \
        } while (false)

#define rwlj_array_trim(array)                                                 \
        do {                                                                   \
                (array)->data = rwlj_arena_realloc(                            \
                        (array)->arena,                                        \
                        (array)->data,                                         \
                        (array)->capacity,                                     \
                        (array)->count                                         \
                );                                                             \
                                                                               \
                if ((array)->data == NULL) {                                   \
                        (array)->count = 0;                                    \
                        (array)->capacity = 0;                                 \
                } else {                                                       \
                        (array)->capacity = (array)->count;                    \
                }                                                              \
        } while (false)

/* hashmaps */

/*
 *  Strings
 */

typedef struct rwljString {
        char *data;
        isize len;
} rwljString;

#define rwlj_str(string) { string, rwlj_size_of(string) }

typedef struct rwljStringBuilder {
        char *buf;
        isize count;
        isize capacity;
} rwljStringBuilder;

isize rwlj_wprintf_va(char *buf, isize len, rwljString fmt, va_list args);
isize rwlj_wprintf(char *buf, isize len, rwljString fmt, ...);

/*
 *  Algorithms
 */

void rwlj_quick_sort(void *data, isize len, isize elem_size);
void rwlj_insertion_sort(void *data, isize len, isize elem_size);

/*
 *
 *  IMPLEMENTATION
 *
 */

/*
 *  Memory Allocators
 */

/* arena */

void
rwlj_arena_init(
        rwljArena *arena,
        void *backing_buf,
        usize size,
        rwlj_arena_kind kind
)
{
        arena->allocated_size = 0;
        arena->kind = kind;
        switch (arena->kind) {
        case RWLJ_ARENA_STATIC:
                arena->total_size = size > 0 ? size : rwlj_mb(1);
                arena->backing_buf = malloc(arena->total_size);
                assert(arena->backing_buf != NULL);
                break;
        case RWLJ_ARENA_GROWING:
                arena->total_size = size > 0 ? size : rwlj_mb(8);
                arena->backing_buf =
                        mmap(NULL,
                             arena->total_size,
                             PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANON,
                             -1,
                             0);
                assert(arena->backing_buf != NULL);
                break;
        case RWLJ_ARENA_BUFFER:
                assert(backing_buf != NULL);
                arena->backing_buf = backing_buf;
                arena->total_size = size;
                break;
        }
}

void *
rwlj_arena_alloc_aligned(rwljArena *arena, usize size, usize alignment)
{
        usize allocation_size = rwlj_align_pow2(size, alignment);
        usize allocation_offset = arena->allocated_size + allocation_size;
        if (allocation_offset > arena->total_size) {
                return NULL;
        }

        void *allocation = &arena->backing_buf[arena->allocated_size];
        memset(allocation, 0, allocation_offset);
        arena->allocated_size = allocation_offset;

        return allocation;
}

void *
rwlj_arena_realloc_aligned(
        rwljArena *arena,
        void *old_mem,
        usize old_size,
        usize new_size,
        usize alignment
)
{
        if (old_mem == NULL || old_size == 0) {
                return rwlj_arena_alloc_aligned(arena, new_size, alignment);
        }

        // Out of bounds
        void *arena_buf = rwlj_cast(void *) arena->backing_buf;
        if (old_mem < arena_buf || old_mem >= &arena_buf[arena->total_size] ||
            old_size > arena->allocated_size) {
                return NULL;
        }

        usize offset = arena->allocated_size - old_size;
        void *mem = &arena_buf[offset];
        if (new_size > 0 && new_size <= arena->total_size && mem == old_mem) {
                arena->allocated_size += new_size - old_size;
        }

        return old_mem;
}

void
rwlj_arena_free_all(rwljArena *arena)
{
        arena->allocated_size = 0;
}

void
rwlj_arena_destroy(rwljArena *arena)
{
        switch (arena->kind) {
        case RWLJ_ARENA_STATIC:
                free(arena->backing_buf);
                break;
        case RWLJ_ARENA_GROWING:
                munmap(arena->backing_buf, arena->total_size);
                break;
        case RWLJ_ARENA_BUFFER:
                // no-op
                return;
        }
}

void
rwlj_arena_temp_init(rwljArenaTemp *temp_arena, rwljArena *arena)
{
        temp_arena->arena = arena;
        temp_arena->allocated_size = temp_arena->arena->allocated_size;
}

void
rwlj_arena_temp_free_all(rwljArenaTemp *temp_arena)
{
        temp_arena->arena->allocated_size = temp_arena->allocated_size;
}

/*
 *  Strings
 */

isize
rwlj_wprintf_va(char *buf, isize len, rwljString fmt, va_list ap)
{
        typedef enum rwlj_fmt_flags {
                // Flags
                RWLJ_ALTERNATE = rwlj_bit(1),
                RWLJ_ZERO = rwlj_bit(2),
                RWLJ_MINUS = rwlj_bit(3),
                RWLJ_BLANK = rwlj_bit(4),
                RWLJ_SIGN = rwlj_bit(5),
                RWLJ_THOUSANDS = rwlj_bit(6),

                // Conversion specifier
                RWLJ_SIGNED = rwlj_bit(7),
                RWLJ_OCTAL = rwlj_bit(8),
                RWLJ_HEX = rwlj_bit(9),
                RWLJ_FLOAT = rwlj_bit(10),
                RWLJ_EXPONENT = rwlj_bit(11),
                RWLJ_CHAR = rwlj_bit(12),
                RWLJ_STRING = rwlj_bit(13),
        } rwlj_fmt_flags;

        typedef struct rwljConvState {
                rwlj_fmt_flags flags;
                isize field_width;
                isize precision;
        } rwljConvState;
        rwljConvState state = { 0 };

        char *string = buf;
        isize bytes_written = 0;
        for (isize i = 0; i < len;) {
                if (string[i] != '%') {
                        buf[bytes_written] = string[i];
                        i += 1;
                        bytes_written += 1;
                        continue;
                }
                if (string[i + 1] == '%') {
                        i += 1;
                        buf[bytes_written] = string[i];
                        i += 1;
                        bytes_written += 1;
                        continue;
                }

        parsing:
                i += 1;
                switch (string[i]) {
                case '#':
                        state.flags |= RWLJ_ALTERNATE;
                        goto parsing;
                case '0':
                        if (RWLJ_MINUS & ~state.flags) {
                                state.flags |= RWLJ_ZERO;
                        }
                        goto parsing;
                case '-':
                        state.flags |= RWLJ_MINUS;
                        state.flags = RWLJ_ZERO & ~state.flags;
                        goto parsing;
                case ' ':
                        if (RWLJ_SIGN & ~state.flags) {
                                state.flags |= RWLJ_BLANK;
                        }
                        goto parsing;
                case '+':
                        state.flags |= RWLJ_SIGN;
                        state.flags = RWLJ_BLANK & ~state.flags;
                        goto parsing;
                case '\'':
                        state.flags |= RWLJ_THOUSANDS;
                        goto parsing;
                case 'h':
                case 'l':
                case 'L':
                case 'j':
                case 'z':
                case 't':
                        goto parsing;
                case 'd':
                case 'i':
                        state.flags |= RWLJ_SIGNED;
                        break;
                case 'x':
                case 'X':
                        state.flags |= RWLJ_HEX;
                case 'o':
                        state.flags |= RWLJ_OCTAL;
                        state.flags = RWLJ_HEX & ~state.flags;
                case 'u':
                        state.flags = RWLJ_SIGNED & ~state.flags;
                        break;
                case 'a':
                case 'A':
                        // We'll just ignore the exponent flag for this one
                        state.flags |= RWLJ_HEX;
                case 'g':
                case 'G':
                case 'e':
                case 'E':
                        state.flags |= RWLJ_EXPONENT;
                case 'f':
                case 'F':
                        state.flags |= RWLJ_FLOAT;
                        break;
                }
        }

        // Actual stuff now
}

isize
rwlj_wprintf(char *buf, isize len, rwljString fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        isize count = rwlj_wprintf_va(buf, len, fmt, ap);
        va_end(ap);

        return count;
}

#define RWLJ_IMPLEMENTATION

#if defined(RWLJ_IMPLEMENTATION) && !defined(RWLJ_IMPLEMENTATION_DONE)
#define RWLJ_IMPLEMENTATION_DONE
#endif

#endif
