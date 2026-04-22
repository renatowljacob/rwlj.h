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

#include <stdio.h>
#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __linux__

#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#elif defined(_WIN64)

#endif

// TODO: pthreads

/*
 *
 *  DECLARATION
 *
 */

/*
 *  Types, Utility Functions and Macros
 */

#if defined(__clang__) || defined(__GNUC__)
#define RWLJ_PRINTF_ARGS(num) __attribute__((format(printf, num, (num + 1))))
#endif

#define rwlj_global     static
#define rwlj_internal   static
#define rwlj_persistent static

#define rwlj_concat(x, y)  x##y
#define rwlj_concat_(x, y) x##_##y

#define RWLJ_STDIN  STDIN_FILENO
#define RWLJ_STDOUT STDOUT_FILENO
#define RWLJ_STDERR STDERR_FILENO

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#ifdef __SIZEOF_INT128__
typedef __int128_t i128;
typedef __uint128_t u128;
#endif

typedef intptr_t intptr;
typedef uintptr_t uintptr;
typedef ptrdiff_t isize;
typedef size_t usize;

typedef bool b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;
typedef isize bsize;

#ifdef __SSE__
typedef _Float16 f16;
#endif
typedef float f32;
typedef double f64;

#define RWLJ_U8_MIN 0u
#define RWLJ_U8_MAX 0xffu
#define RWLJ_I8_MIN (-0x7f - 1)
#define RWLJ_I8_MAX 0x7f

#define RWLJ_U16_MIN 0u
#define RWLJ_U16_MAX 0xffffu
#define RWLJ_I16_MIN (-0x7fff - 1)
#define RWLJ_I16_MAX 0x7fff

#define RWLJ_U32_MIN 0u
#define RWLJ_U32_MAX 0xffffffffu
#define RWLJ_I32_MIN (-0x7fffffff - 1)
#define RWLJ_I32_MAX 0x7fffffff

#define RWLJ_U64_MIN 0ull
#define RWLJ_U64_MAX 0xffffffffffffffffull
#define RWLJ_I64_MIN (-0x7fffffffffffffffll - 1)
#define RWLJ_I64_MAX 0x7fffffffffffffffll

#define RWLJ_F16_MAX 65504.0
#define RWLJ_F16_MIN 6.10351562e-5

#define RWLJ_F32_MAX 3.40282347e+38F
#define RWLJ_F32_MIN 1.17549435e-38F

#define RWLJ_F64_MIN 2.2250738585072014e-308
#define RWLJ_F64_MAX 1.7976931348623158e+308

#define cast(T) (T)

#define RWLJ_TRAP() __builtin_trap()

#define rwlj_no_op()     ((void)0)
#define rwlj_unused(var) ((void)var)

#define __rwlj_assert(cond)                                                    \
    do {                                                                       \
        if (cond) {                                                            \
            break;                                                             \
        }                                                                      \
                                                                               \
        rwlj_printfln(                                                         \
            "Assertion failure at %s:%d:%s()",                                 \
            RWLJ_FILE,                                                         \
            RWLJ_LINE,                                                         \
            RWLJ_FUNCTION                                                      \
        );                                                                     \
    } while (false)

#ifdef DEBUG
#define rwlj_assert(cond)                                                      \
    do {                                                                       \
        __rwlj_assert(cond);                                                   \
        RWLJ_TRAP();                                                           \
    } while (false)

#define rwlj_assert_msg(cond, ...)                                             \
    do {                                                                       \
        __rwlj_assert(cond);                                                   \
        rwlj_printfln(__VA_ARGS__);                                            \
        RWLJ_TRAP();                                                           \
    } while (false)
#else
#define rwlj_assert(cond) rwlj_no_op()

#define rwlj_assert_msg(cond, ...) rwlj_no_op()
#endif

#define __rwlj_static_assert(cond, msg)                                        \
    rwlj_global u8 rwlj_concat(msg, RWLJ_LINE)[!!(cond) ? 1 : -1]
#define rwlj_static_assert(cond)                                               \
    __rwlj_static_assert(cond, static_assertion_at_line_)

#define rwlj_size_of(x)           (isize)(sizeof(x))
#define rwlj_offset_of(T, member) ((isize) & (((T *)0)->member))
#define rwlj_align_of(T)                                                       \
    ((isize)rwlj_offset_of(                                                    \
        struct {                                                               \
            char c;                                                            \
            T member;                                                          \
        },                                                                     \
        member                                                                 \
    ))
#define rwlj_container_of(T, object, member)                                   \
    cast(void *)(cast(intptr)(object) - rwlj_offset_of(T, member))

#define rwlj_size_of_array(a) (rwlj_size_of(a) / rwlj_size_of(0 [a]))

#define rwlj_bit(n) (1 << n)
#define rwlj_kb(n)  (n << 10)
#define rwlj_mb(n)  (n << 20)
#define rwlj_gb(n)  (n << 30)

#define rwlj_swap(T, x, y)                                                     \
    do {                                                                       \
        T __temp = (x);                                                        \
        (x) = (y);                                                             \
        (y) = __temp;                                                          \
    } while (false)

#define rwlj_max(x, y)              (x > y ? x : y)
#define rwlj_min(x, y)              (x < y ? x : y)
#define rwlj_clamp(x, lower, upper) (rwlj_max(rwlj_min(x, upper), lower))

#define rwlj_align_pow2(x, b) ((x + b - 1) & (~(b - 1)))
#define rwlj_is_pow2(x)       ((x & (x - 1)) == 0)

// #define rwlj_is_nil(ptr, nil) (ptr == NULL || ptr == nil)

#define rwlj_is_space(c)                                                       \
    (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#define rwlj_is_upper(c)        (c >= 'A' && c <= 'Z')
#define rwlj_is_lower(c)        (c >= 'a' && c <= 'z')
#define rwlj_to_upper(c)        (rwlj_is_lower(c) ? c - 32 : c)
#define rwlj_to_lower(c)        (rwlj_is_upper(c) ? c + 32 : c)
#define rwlj_is_alpha(c)        (rwlj_is_upper(c) || rwlj_is_lower(c))
#define rwlj_is_digit(c)        (c >= '0' && c <= '9')
#define rwlj_is_alphanumeric(c) (rwlj_is_alpha(c) || rwlj_is_digit(c))
#define rwlj_is_digit_hex(c)                                                   \
    (rwlj_is_digit(c) || (rwlj_to_upper(c) >= 'A' && rwlj_to_upper(c) <= 'F'))

#define RWLJ_LINE     __LINE__
#define RWLJ_FUNCTION __func__
#define RWLJ_FILE     __FILE__
#define RWLJ_COUNTER  (__COUNTER__ + 1)

/*
 *  Math
 */

#define rwlj_abs(x) (x >= 0 ? x : -(x))

#define rwlj_truncate(x) (cast(i64) x)
#define rwlj_floor(x)    (rwlj_truncate(x > 0.0f ? x : x - 1.0f))
#define rwlj_ceil(x)     (rwlj_truncate(x < 0.0f ? x : x + 1.0f))
#define rwlj_round(x) (x >= 0.0f ? rwlj_ceil(x - 0.5f) : rwlj_floor(x + 0.5f))

#define RWLJ_F16_SHIFT (16 - 6)
#define RWLJ_F16_MASK  0x1fll
#define RWLJ_F16_BIAS  0xfll

#define RWLJ_F32_SHIFT (32 - 9)
#define RWLJ_F32_MASK  0xffll
#define RWLJ_F32_BIAS  0x7fll

#define RWLJ_F64_SHIFT (64 - 12)
#define RWLJ_F64_MASK  0x7ffll
#define RWLJ_F64_BIAS  0x3ffll

enum rwljFloat_Class {
    RWLJ_NORMAL,
    RWLJ_SUBNORMAL,
    RWLJ_INFINITY,
    RWLJ_NEGATIVE_INFINITY,
    RWLJ_NAN
};
typedef isize rwljFloat_Class;

rwljFloat_Class rwlj_is_inf(f64 f);
rwljFloat_Class rwlj_is_nan(f64 f);
rwljFloat_Class rwlj_classify(f64 f);
f64 rwlj_normalize(f64 f, i64 *exponent);
f64 rwlj_frexp(f64 f, i64 *exponent);

/*
 *  Memory Allocators
 */

/* arena */

#define RWLJ_DEFAULT_ALIGNMENT sizeof(void *)

enum rwljArena_Kind {
    RWLJ_ARENA_STATIC,
    RWLJ_ARENA_GROWING,
    RWLJ_ARENA_BUFFER,
};
typedef isize rwljArena_Kind;

typedef struct rwljArena {
    u8 *backing_buf;
    usize total_size;
    usize allocated_size;
    rwljArena_Kind kind;
} rwljArena;

void
rwlj_arena_init(rwljArena *arena, void *mem, usize size, rwljArena_Kind kind);

#define rwlj_arena_init_static(arena)                                          \
    rwlj_arena_init(arena, NULL, 0, RWLJ_ARENA_STATIC)
#define rwlj_arena_init_static_size(arena, size)                               \
    rwlj_arena_init(arena, NULL, size, RWLJ_ARENA_STATIC)

#define rwlj_arena_init_growing(arena)                                         \
    rwlj_arena_init(arena, NULL, 0, RWLJ_ARENA_GROWING)
#define rwlj_arena_init_growing_size(arena, size)                              \
    rwlj_arena_init(arena, NULL, size, RWLJ_ARENA_GROWING)

#define rwlj_arena_init_from_buffer(arena, backing_buf, size)                  \
    rwlj_arena_init(arena, backing_buf, size, RWLJ_ARENA_BUFFER)

void *rwlj_arena_alloc_aligned(rwljArena *arena, usize size, usize alignment);
#define rwlj_arena_alloc(arena, size)                                          \
    rwlj_arena_alloc_aligned(arena, size, RWLJ_DEFAULT_ALIGNMENT)

void *rwlj_arena_resize_aligned(
    rwljArena *arena,
    void *old_mem,
    usize old_size,
    usize new_size,
    usize alignment
);
#define rwlj_arena_resize(arena, old_mem, old_size, new_size)                  \
    rwlj_arena_resize_aligned(                                                 \
        arena, old_mem, old_size, new_size, RWLJ_DEFAULT_ALIGNMENT             \
    )

void rwlj_arena_free_all(rwljArena *arena);
void rwlj_arena_destroy(rwljArena *arena);

typedef struct rwljArena_Temp {
    rwljArena *arena;
    usize allocated_size;
} rwljArena_Temp;

void rwlj_arena_temp_init(rwljArena_Temp *arena_temp, rwljArena *arena);
void rwlj_arena_temp_free_all(rwljArena_Temp *arena_temp);

/*
 *  Data Structures
 */

/* slices */

#define GENERIC_SLICE(T, name)                                                 \
    typedef struct rwlj_concat_(rwljSlice, name) {                             \
        T *data;                                                               \
        isize len;                                                             \
    } rwlj_concat_(rwljSlice, name)

GENERIC_SLICE(i8, I8);
GENERIC_SLICE(i16, I16);
GENERIC_SLICE(i32, I32);
GENERIC_SLICE(i64, I64);
GENERIC_SLICE(isize, Isize);
GENERIC_SLICE(u8, U8);
GENERIC_SLICE(u16, U16);
GENERIC_SLICE(u32, U32);
GENERIC_SLICE(u64, U64);
GENERIC_SLICE(usize, Usize);
GENERIC_SLICE(f16, F16);
GENERIC_SLICE(f32, F32);
GENERIC_SLICE(f64, F64);
GENERIC_SLICE(char, String);

#define rwlj_slice_get(slice, index)                                           \
    (index >= 0 && index < (slice)->len ? (slice)->data[index] : 0)
#define rwlj_slice_set(slice, index, value)                                    \
    (index >= 0 && index < (slice)->len ? ((slice)->data[index] = value, true) \
                                        : false)

#define rwlj_slice_reverse(slice, item_T)                                      \
    do {                                                                       \
        for (isize __i = 0; __i < (slice)->len / 2; __i += 1) {                \
            rwlj_swap(                                                         \
                item_T,                                                        \
                (slice)->data[__i],                                            \
                (slice)->data[((slice)->len - __i - 1)]                        \
            );                                                                 \
        }                                                                      \
    } while (false)

#define rwlj_slice_clear(slice)                                                \
    do {                                                                       \
        memset(                                                                \
            &(slice)->data[0],                                                 \
            0,                                                                 \
            cast(usize)((slice)->len * rwlj_size_of((slice)->data[0]))         \
        );                                                                     \
    } while (false)

/* dynamic arrays */

enum rwljArray_Kind {
    RWLJ_ARRAY_GROWING,
    RWLJ_ARRAY_FIXED,
};
typedef isize rwljArray_Kind;

#define GENERIC_ARRAY(T, name)                                                 \
    typedef struct rwlj_concat(rwljArray, name) {                              \
        T *data;                                                               \
        isize len;                                                             \
        isize capacity;                                                        \
        rwljArray_Kind kind;                                                   \
        rwljArena *arena;                                                      \
        usize item_size;                                                       \
    } rwlj_concat_(rwljArray, name)

GENERIC_ARRAY(i8, I8);
GENERIC_ARRAY(i16, I16);
GENERIC_ARRAY(i32, I32);
GENERIC_ARRAY(i64, I64);
GENERIC_ARRAY(isize, Isize);
GENERIC_ARRAY(u8, U8);
GENERIC_ARRAY(u16, U16);
GENERIC_ARRAY(u32, U32);
GENERIC_ARRAY(u64, U64);
GENERIC_ARRAY(usize, Usize);
GENERIC_ARRAY(f16, F16);
GENERIC_ARRAY(f32, F32);
GENERIC_ARRAY(f64, F64);

#define RWLJ_GROW_FORMULA(capacity) (8 + capacity * 2)

#define rwlj_array_init_reserve_with_kind(array, capacity_, kind_, arena_)     \
    do {                                                                       \
        (array)->arena = (arena_);                                             \
        (array)->len = 0;                                                      \
        (array)->capacity = (capacity_);                                       \
        (array)->kind = (kind_);                                               \
        (array)->item_size = rwlj_size_of((array)->data[0]);                   \
        (array)->data = rwlj_arena_alloc(                                      \
            (array)->arena, cast(usize)(array)->capacity * (array)->item_size  \
        );                                                                     \
    } while (false)

#define rwlj_array_init_dynamic(array, arena)                                  \
    rwlj_array_init_reserve_with_kind(                                         \
        array, RWLJ_GROW_FORMULA(0), RWLJ_ARRAY_GROWING, arena                 \
    )

#define rwlj_array_init_dynamic_reserve(array, capacity, arena)                \
    rwlj_array_init_reserve_with_kind(                                         \
        array,                                                                 \
        (capacity > RWLJ_GROW_FORMULA(0) ? capacity : RWLJ_GROW_FORMULA(0)),   \
        RWLJ_ARRAY_GROWING,                                                    \
        arena                                                                  \
    )

#define rwlj_array_init_fixed(array, fixed_array)                              \
    do {                                                                       \
        (array)->arena = NULL;                                                 \
        (array)->len = 0;                                                      \
        (array)->capacity = rwlj_size_of_array(fixed_array);                   \
        (array)->kind = RWLJ_ARRAY_FIXED;                                      \
        (array)->item_size = rwlj_size_of((array)->data[0]);                   \
        (array)->data = (fixed_array);                                         \
    } while (false)

#define rwlj_array_get(array, index)        rwlj_slice_get(array, index)
#define rwlj_array_set(array, index, value) rwlj_slice_set(array, index, value)

bool __rwlj_array_resize(rwljArray_I64 *array, isize new_capacity);

#define rwlj_array_resize(array, new_capacity)                                 \
    __rwlj_array_resize((rwljArray_I64 *)array, new_capacity)

#define rwlj_array_grow(array)                                                 \
    rwlj_array_resize(array, RWLJ_GROW_FORMULA((array)->capacity))

#define rwlj_array_reserve(array, new_size)                                    \
    rwlj_array_resize(array, new_size > (array)->capacity ? new_size : 0)

#define rwlj_array_trim(array) rwlj_array_resize(array, (array)->len)

// Abusing short circuiting to resize array (while also being an expression,
// that's the important part)
// Checks to see if the array is a growing one and has enough capacity or can be
// grown
#define rwlj_array_append(array, value)                                        \
    (((array)->kind == RWLJ_ARRAY_GROWING &&                                   \
          ((array)->len < (array)->capacity) ||                                \
      rwlj_array_grow(array)) ||                                               \
             ((array)->kind == RWLJ_ARRAY_FIXED &&                             \
              (array)->len < (array)->capacity)                                \
         ? ((array)->data[((array)->len)] = (value), (array)->len += 1, true)  \
         : false)

#define rwlj_array_inject(array, index, value)                                 \
    (((array)->kind == RWLJ_ARRAY_GROWING &&                                   \
          ((array)->len < (array)->capacity) ||                                \
      rwlj_array_grow(array)) ||                                               \
             ((array)->kind == RWLJ_ARRAY_FIXED &&                             \
              (array)->len < (array)->capacity)                                \
         ? (memmove(                                                           \
                &(array)->data[index + 1],                                     \
                &(array)->data[index],                                         \
                cast(usize)(((array)->len * cast(usize)(array)->item_size))    \
            ),                                                                 \
            (array)->data[index] = (value),                                    \
            ((array)->len += 1),                                               \
            true)                                                              \
         : false)

#define rwlj_array_preppend(array, value) rwlj_array_inject(array, 0, value)

#define rwlj_array_pop(array)                                                  \
    ((array)->len > 0 ? ((array)->len -= 1, (array)->data[(array)->len]) : 0)

#define rwlj_array_remove_unordered(array, index)                              \
    do {                                                                       \
        if ((index) < (array)->len && (index) >= 0 && (array)->len > 0) {      \
            (array)->len -= 1;                                                 \
            (array)->data[index] = (array)->data[(array)->len];                \
        }                                                                      \
    } while (false)

#define rwlj_array_remove_ordered(array, index)                                \
    do {                                                                       \
        if ((index) < (array)->len && (index) >= 0 && (array)->len > 0) {      \
            (array)->data[index] = (array)->data[--(array)->len];              \
            memmove(                                                           \
                &(array)->data[index],                                         \
                &(array)->data[index + 1],                                     \
                cast(usize)(                                                   \
                    ((array)->len - index) * cast(usize)(array)->item_size     \
                )                                                              \
            );                                                                 \
            (array)->len -= 1;                                                 \
        }                                                                      \
    } while (false)

/* hashmaps */

/*
 *  Strings
 */

typedef struct rwljString {
    char *data;
    isize len;
} rwljString;

#define STR_LIT(string) cast(rwljString){ string, rwlj_size_of(string) - 1 }

isize rwlj_string_compare(rwljString a, rwljString b);
bool rwlj_string_are_equal(rwljString a, rwljString b);
rwljString rwlj_string_clone(rwljString s, rwljArena *arena);

isize rwlj_string_cstrlen(const char *string, isize max_len);

isize rwlj_format_i64(rwljSlice_U8 buf, i64 number);
isize rwlj_format_u64(rwljSlice_U8 buf, u64 number, u64 base);
isize rwlj_format_f64(rwljSlice_U8 buf, f64 number, u8 fmt, i64 precision);
isize rwlj_format_string(rwljSlice_String buf, rwljString s);

isize __rwlj_bprintf_va(
    rwljSlice_String buf,
    bool has_new_line,
    const char *fmt,
    va_list args
);
isize rwlj_bprintf(rwljSlice_String buf, const char *fmt, ...);
isize rwlj_bprintfln(rwljSlice_String buf, const char *fmt, ...);

isize __rwlj_fprintf_va(i32 fd, bool has_new_line, char const *fmt, va_list ap);
isize rwlj_fprintf(i32 fd, char const *fmt, ...);
isize rwlj_fprintfln(i32 fd, char const *fmt, ...);

isize rwlj_eprintf(char const *fmt, ...);
isize rwlj_eprintfln(char const *fmt, ...);

isize rwlj_printf(char const *fmt, ...);
isize rwlj_printfln(char const *fmt, ...);

typedef struct rwljString_Builder {
    char *buf;
    isize len;
    isize capacity;
} rwljString_Builder;

void rwlj_string_builder_init(
    rwljString_Builder *sb,
    rwljArena *arena,
    isize capacity
);
rwljString rwlj_string_builder_write_i64(rwljString_Builder *sb, i64 number);
rwljString
rwlj_string_builder_write_u64(rwljString_Builder *sb, u64 number, u64 base);
rwljString rwlj_string_builder_write_f64(
    rwljString_Builder *sb,
    f64 number,
    u8 fmt,
    i64 precision
);
rwljString
rwlj_string_builder_write_string(rwljString_Builder *sb, rwljString s);
rwljString rwlj_string_builder_to_string(rwljString_Builder *sb);
rwljString rwlj_string_builder_clone(rwljString_Builder *sb, rwljArena *arena);
void rwlj_string_builder_clear(rwljString_Builder *sb);

rwljString rwlj_sbprintf(rwljString_Builder *sb, char const *fmt, ...);
rwljString rwlj_sbprintfln(rwljString_Builder *sb, char const *fmt, ...);

/*
 *  Algorithms
 */

void rwlj_quick_sort(void *data, isize len, isize elem_size);
void rwlj_insertion_sort(void *data, isize len, isize elem_size);

/*
 *  Testing
 */

#define DESCRIBE(string)                                                       \
    do {                                                                       \
        rwlj_printfln(                                                         \
            "----------------------------------------------------------------" \
            "----------------"                                                 \
        );                                                                     \
        rwlj_printfln(string);                                                 \
        rwlj_printfln(                                                         \
            "----------------------------------------------------------------" \
            "----------------"                                                 \
        );                                                                     \
    } while (false)

#define IT(string)                                                             \
    for (bool has_ended = false, success = true;                               \
         !has_ended && rwlj_printfln("[%d] - " string "...", RWLJ_COUNTER);    \
         has_ended = true,                                                     \
              success ? rwlj_printfln("\tOK") : rwlj_printfln("\tfailed"))

#define __rwlj_testing_printf_info()                                           \
    rwlj_printf("[%s:%d:%s()]: ", RWLJ_FILE, RWLJ_LINE, RWLJ_FUNCTION)

#define rwlj_testing_printf(...)                                               \
    do {                                                                       \
        __rwlj_testing_printf_info();                                          \
        rwlj_printfln(__VA_ARGS__);                                            \
    } while (false)

#define rwlj_testing_expect(condition)                                         \
    do {                                                                       \
        if (!(condition)) {                                                    \
            rwlj_testing_printf("Expected %s to be true", #condition);         \
            success = false;                                                   \
        }                                                                      \
    } while (false)

#define rwlj_testing_expect_value(a, b)                                        \
    do {                                                                       \
        if (a != b) {                                                          \
            rwlj_testing_printf("Expected %s, got %s", #b, #a);                \
            success = false;                                                   \
        }                                                                      \
    } while (false)

#define rwlj_testing_expect_string(a, b)                                       \
    do {                                                                       \
        if (!rwlj_string_are_equal(a, b)) {                                    \
            rwlj_testing_printf(                                               \
                "Expected %.*s, got %.*s",                                     \
                cast(i32) a.len,                                               \
                a.data,                                                        \
                cast(i32) b.len,                                               \
                b.data                                                         \
            );                                                                 \
            success = false;                                                   \
        }                                                                      \
    } while (false)

/*
 *
 *  IMPLEMENTATION
 *
 */

/*
 *  Math
 */

// Snatched most of things from Odin. Holy moly I wanna get better at
// floating-point stuff

rwljFloat_Class
rwlj_is_inf(f64 f)
{
    return f * 0.5 == f;
}

rwljFloat_Class
rwlj_is_nan(f64 f)
{
    return f != f;
}

rwljFloat_Class
rwlj_classify(f64 f)
{
    (void)f;
    return 1;
}

f64
rwlj_normalize(f64 f, i64 *exponent)
{
    if (rwlj_abs(f) < RWLJ_F64_MIN) {
        *exponent = -(RWLJ_F64_SHIFT);
        return f * (1l << RWLJ_F64_SHIFT);
    }
    return f;
}

f64
rwlj_frexp(f64 f, i64 *exponent)
{
    union {
        f64 f;
        i64 i;
    } y = { .f = f };

    if (f == 0) {
        *exponent = 0;
        return 0;
    } else if (rwlj_is_inf(f) || rwlj_is_nan(f)) {
        *exponent = 0;
        return y.f;
    }

    y.f = rwlj_normalize(y.f, exponent);
    *exponent += ((y.i >> RWLJ_F64_SHIFT) & RWLJ_F64_MASK) - RWLJ_F64_BIAS + 1;
    y.i &= ~(RWLJ_F64_MASK << RWLJ_F64_SHIFT);
    y.i |= (-1 + RWLJ_F64_BIAS) << RWLJ_F64_SHIFT;
    return y.f;
}

f64
rwlj_ldexp(f64 mantissa, i64 exponent)
{
    union {
        f64 f;
        i64 i;
    } f = { 0 };

    if (rwlj_is_inf(mantissa) || rwlj_is_nan(mantissa) || mantissa == 0) {
        return mantissa;
    }

    i64 exp = exponent;
    f64 man = rwlj_normalize(mantissa, &exp);

    f.f = man;
    exp += ((f.i >> RWLJ_F64_SHIFT) & RWLJ_F64_MASK) - RWLJ_F64_BIAS;
    if (exp < -(RWLJ_F64_BIAS + RWLJ_F64_SHIFT)) {
        // Underflow
        return (man < 0) ? -0.0 : +0.0;
    } else if (exp > RWLJ_F64_BIAS) {
        // Overflow
        return (f.f < 0) ? -RWLJ_INFINITY : RWLJ_INFINITY;
    }

    f64 m = 1;
    if (exp < -(RWLJ_F64_BIAS - 1)) {
        exp += 53;
        m /= (1l << (RWLJ_F64_SHIFT + 1));
    }
    f.i &= ~(RWLJ_F64_MASK << RWLJ_F64_SHIFT);
    f.i |= (exp + RWLJ_F64_BIAS) << RWLJ_F64_SHIFT;

    return m * f.f;
}

/*
 *  Memory Allocators
 */

/* arena */

void
rwlj_arena_init(
    rwljArena *arena,
    void *backing_buf,
    usize size,
    rwljArena_Kind kind
)
{
    arena->allocated_size = 0;
    arena->kind = kind;
    switch (arena->kind) {
    case RWLJ_ARENA_STATIC:
        arena->total_size = size > 0 ? size : rwlj_mb(1);
        arena->backing_buf = malloc(arena->total_size);
        break;
    case RWLJ_ARENA_GROWING:
        arena->total_size = size > 0 ? size : rwlj_mb(8);
        arena->backing_buf = mmap(
            NULL,
            arena->total_size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANON,
            -1,
            0
        );
        break;
    case RWLJ_ARENA_BUFFER:
        arena->total_size = size;
        arena->backing_buf = backing_buf;
        break;
    }
    rwlj_assert(arena->backing_buf != NULL);
}

void *
rwlj_arena_alloc_aligned(rwljArena *arena, usize size, usize alignment)
{
    if (size == 0) {
        return NULL;
    }

    usize allocation_size = rwlj_align_pow2(size, alignment);
    usize allocation_offset = arena->allocated_size + allocation_size;
    if (allocation_offset > arena->total_size) {
        return NULL;
    }

    void *allocation = &arena->backing_buf[arena->allocated_size];
    memset(allocation, 0, allocation_size);
    arena->allocated_size = allocation_offset;

    return allocation;
}

void *
rwlj_arena_resize_aligned(
    rwljArena *arena,
    void *old_memory,
    usize old_size,
    usize new_size,
    usize alignment
)
{
    u8 *old_mem = old_memory;
    if (old_mem == NULL || old_size == 0) {
        return rwlj_arena_alloc_aligned(arena, new_size, alignment);
    }

    // Out of bounds
    if (old_mem < arena->backing_buf ||
        old_mem >= &arena->backing_buf[arena->total_size] ||
        old_size > arena->allocated_size) {
        return NULL;
    }

    usize offset = arena->allocated_size - old_size;
    void *mem = &arena->backing_buf[offset];
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
        rwlj_no_op();
        return;
    }
}

void
rwlj_arena_temp_init(rwljArena_Temp *temp_arena, rwljArena *arena)
{
    temp_arena->arena = arena;
    temp_arena->allocated_size = temp_arena->arena->allocated_size;
}

void
rwlj_arena_temp_free_all(rwljArena_Temp *temp_arena)
{
    temp_arena->arena->allocated_size = temp_arena->allocated_size;
}

/*
 *  Array Resizing
 */

bool
__rwlj_array_resize(rwljArray_I64 *array, isize new_capacity)
{
    if (new_capacity == 0) {
        return false;
    }

    if (array->kind == RWLJ_ARRAY_FIXED) {
        // Reserve is a no op
        if (new_capacity > array->capacity || new_capacity < 0) {
            return false;
        }
        array->capacity = new_capacity;
        return true;
    }

    void *curr_data = array->data;
    isize array_new_cap = new_capacity;
    usize array_new_size = cast(usize) array_new_cap * array->item_size;

    usize arena_new_allocated_size =
        array->arena->allocated_size -
        (cast(usize) array->capacity * array->item_size) + array_new_size;
    if (arena_new_allocated_size > array->arena->total_size) {
        usize rest = array->arena->total_size - array->arena->allocated_size;
        array_new_cap = array->capacity + cast(isize)(rest / array->item_size);

        // Array doesn't get resized
        if (array_new_cap == array->capacity) {
            return false;
        }
        array_new_size = cast(usize) array_new_cap * array->item_size;
    }

    array->data = rwlj_arena_resize(
        array->arena, array->data, cast(usize) array->capacity, array_new_size
    );
    array->capacity = array_new_cap;
    if (array->data != curr_data) {
        memmove(
            array->data, curr_data, cast(usize) array->len * array->item_size
        );
    }
    rwlj_assert(array->data != NULL);

    return true;
}

/*
 *  Strings
 */

isize
rwlj_string_compare(rwljString a, rwljString b)
{
    for (isize i = 0; i < a.len && i < b.len; i += 1) {
        if (a.data[i] != b.data[i]) {
            return a.data[i] - b.data[i];
        }
    }

    return 0;
}

bool
rwlj_string_are_equal(rwljString a, rwljString b)
{
    if (a.len != b.len || rwlj_string_compare(a, b)) {
        return false;
    }

    return true;
}

rwljString
rwlj_string_clone(rwljString s, rwljArena *arena)
{
    rwljString cloned = { 0 };
    cloned.data = rwlj_arena_alloc(arena, cast(usize) s.len);
    if (cloned.data == NULL || s.data == NULL) {
        return cloned;
    }
    cloned.len = s.len;
    memcpy(cloned.data, s.data, cast(usize) s.len);

    return cloned;
}

isize
rwlj_string_cstrlen(char const *string, isize max_len)
{
    char *s = cast(char *) string;
    isize len = 0;

    while (*s && len < max_len) {
        len += 1;
        s = &s[1];
    }

    return len;
}

// One can only wish for generics
isize
rwlj_format_i64(rwljSlice_U8 buf, i64 number)
{
    i64 base = 10;
    isize bytes_written = 0;

    if (number < 0) {
        if (rwlj_slice_set(&buf, bytes_written, '-')) {
            bytes_written += 1;
        }
    }
    isize start = bytes_written;

    i64 n = rwlj_abs(number);
    do {
        buf.data[bytes_written] = cast(u8)(n % base + '0');
        n /= base;
        bytes_written += 1;
    } while (bytes_written < buf.len && n > 0);

    rwljSlice_U8 slice = { .data = &buf.data[start],
                           .len = bytes_written - start };
    rwlj_slice_reverse(&slice, u8);

    return bytes_written;
}

isize
rwlj_format_u64(rwljSlice_U8 buf, u64 number, u64 base)
{
    isize bytes_written = 0;
    u8 fmt = 0;
    switch (base) {
    case 2:
        fmt = 'b';
        break;
    case 8:
        fmt = 'o';
        break;
    case 10:
        break;
    case 16:
        fmt = 'x';
        break;
    default:
        rwlj_assert_msg(false, "Base not implemented for unsigned integers");
    }
    if (base != 10) {
        if (rwlj_slice_set(&buf, bytes_written, '0')) {
            bytes_written += 1;
        }
        if (rwlj_slice_set(&buf, bytes_written, fmt)) {
            bytes_written += 1;
        }
    }
    isize start = bytes_written;

    char *digit_table =
        rwlj_is_upper(fmt) ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        buf.data[bytes_written] = cast(u8) digit_table[number % base];
        number /= base;
        bytes_written += 1;
    } while (bytes_written < buf.len && number > 0);
    rwljSlice_U8 slice = { .data = &buf.data[start],
                           .len = bytes_written - start };
    rwlj_slice_reverse(&slice, u8);

    return bytes_written;
}

// From https://research.swtch.com/ftoa
// Someday I'll use a better algorithm for this, but for now it's good enough
isize
rwlj_format_f64(rwljSlice_U8 buf, f64 number, u8 fmt, i64 precision)
{
    isize bytes_written = 0;
    isize base = 0;
    switch (fmt) {
    case 'i':
    case 'd':
        base = 10;
        break;
    case 'A':
    case 'a':
        base = 16;
        break;
    default:
        rwlj_assert_msg(
            false, "Base not implemented for floating-point numbers"
        );
    }
    if (base != 10) {
        if (rwlj_slice_set(&buf, bytes_written, '0')) {
            bytes_written += 1;
        }
        if (rwlj_slice_set(&buf, bytes_written, fmt)) {
            bytes_written += 1;
        }
    }

    if (rwlj_is_inf(number)) {
        if (rwlj_slice_set(&buf, bytes_written, number > 0 ? '+' : '-')) {
            bytes_written += 1;
        }

        rwljString inf = STR_LIT("inf");
        for (isize i = 0; i < inf.len; i += 1) {
            if (rwlj_slice_set(&buf, bytes_written, cast(u8) inf.data[i])) {
                bytes_written += 1;
            }
        }

        return bytes_written;
    } else if (rwlj_is_nan(number)) {
        rwljString nan = STR_LIT("nan");
        for (isize i = 0; i < nan.len; i += 1) {
            if (rwlj_slice_set(&buf, bytes_written, cast(u8) nan.data[i])) {
                bytes_written += 1;
            }
        }

        return bytes_written;
    }

    char *digit_table =
        rwlj_is_upper(fmt) ? "0123456789ABCDEF" : "0123456789abcdef";

    i64 exp = 0;
    f64 fr = rwlj_frexp(number, &exp);
    i64 man = (i64)(fr * (1l << (RWLJ_F64_SHIFT + 1)));
    exp -= (RWLJ_F64_SHIFT + 1);

    bytes_written = rwlj_format_i64(buf, man);
    for (; exp > 0; exp -= 1) {
        i64 delta = 0;
        if (buf.data[0] >= '5') {
            delta = 1;
        }

        u8 x = 0;
        for (i64 i = bytes_written - 1; i >= 0; i -= 1) {
            x = cast(u8)(x + 2 * (buf.data[i] - '0'));
            buf.data[i + delta] = cast(u8) digit_table[x % base];
            x /= cast(u8) base;
        }

        if (delta == 1) {
            buf.data[0] = '1';
            bytes_written += 1;
        }
    }
    i64 decimal_point = bytes_written;
    for (; exp < 0; exp += 1) {
        if (buf.data[bytes_written - 1] % 2 != 0) {
            buf.data[bytes_written] = '0';
            bytes_written += 1;
        }

        i64 delta = 0;
        u8 x = 0;
        if (buf.data[0] < '2') {
            delta = 1;
            x = buf.data[0] - '0';
            bytes_written -= 1;
            decimal_point -= 1;
        }
        for (i64 i = 0; i < bytes_written; i += 1) {
            x = cast(u8)(x * cast(u8) base + buf.data[i + delta] - '0');
            buf.data[i] = x / 2 + '0';
            x %= 2;
        }
    }
    if (bytes_written > precision) {
        bool zeroed = true;
        for (isize i = precision + 1; i < bytes_written; i += 1) {
            if (buf.data[i] != '0') {
                zeroed = false;
                break;
            }
        }
        if (buf.data[precision] >= '5' &&
            (!zeroed || buf.data[precision - 1] % 2 == 1)) {
            i64 i = precision - 1;
            for (; i >= 0 && buf.data[i] == '9'; i -= 1) {
                buf.data[i] = '0';
            }
            if (i >= 0) {
                buf.data[i] += 1;
            } else {
                buf.data[0] = '1';
                decimal_point += 1;
            }
        }
        bytes_written = precision;
    }
    for (; bytes_written < precision; bytes_written += 1) {
        buf.data[bytes_written] = '0';
    }

    switch (fmt) {
    case 'F':
    case 'f': {
        isize offset = decimal_point;
        memmove(
            &buf.data[offset + 1], &buf.data[offset], cast(usize) precision
        );
        buf.data[offset] = '.';
        isize end = (offset + 1 + precision);
        memset(
            &buf.data[offset + 1 + precision],
            0,
            cast(usize)(bytes_written - end)
        );
        break;
    }
    case 'E':
    case 'e': {
        isize offset = decimal_point;
        memmove(
            &buf.data[offset + 1], &buf.data[offset], cast(usize) precision
        );
        buf.data[offset] = '.';
        isize end = (offset + 1 + precision);
        memset(
            &buf.data[offset + 1 + precision],
            0,
            cast(usize)(bytes_written - end)
        );
        break;
    }
    // TODO: to be implemented
    case 'G':
    case 'g': {
    }
    }

    return bytes_written;
}

isize
rwlj_format_string(rwljSlice_String buf, rwljString str)
{
    isize len = rwlj_min(buf.len, str.len);
    if (len == 0 || buf.data == NULL || str.data == NULL) {
        return 0;
    }
    memcpy(buf.data, str.data, cast(usize) len);

    return len;
}

isize
__rwlj_bprintf_va(
    rwljSlice_String buf,
    bool has_new_line,
    char const *fmt,
    va_list ap
)
{
    enum rwljLength_Modifier { RWLJ_INT, RWLJ_CHAR, RWLJ_SHORT, RWLJ_LONG };
    typedef i8 rwljLength_Modifier;
    typedef struct rwljConv_State {
        isize field_width;
        isize precision;
        isize base;
        struct {
            bool alternate;
            bool zero;
            bool minus;
            bool blank;
            bool sign;
            bool thousands;
        } flags;
        rwljLength_Modifier length_modifier;
    } rwljConv_State;
    rwljConv_State state = { .base = 10 };

    if (fmt == NULL || buf.data == NULL) {
        return 0;
    }

    char *string = cast(char *) fmt;
    isize bytes_written = 0;
    for (isize i = 0; i < buf.len && bytes_written < buf.len && string[i];
         i += 1) {
        if (string[i] != '%') {
            buf.data[bytes_written] = string[i];
            bytes_written += 1;
            continue;
        }
        if (string[i + 1] == '%') {
            i += 1;
            buf.data[bytes_written] = string[i];
            bytes_written += 1;
            continue;
        }
        i += 1;

        // Parse flags
        for (; i < buf.len; i += 1) {
            switch (string[i]) {
            case '#':
                state.flags.alternate = true;
                continue;
            case '0':
                state.flags.zero = !state.flags.minus;
                continue;
            case '-':
                state.flags.minus = true;
                state.flags.zero = false;
                continue;
            case ' ':
                state.flags.blank = !state.flags.sign;
                continue;
            case '+':
                state.flags.sign = true;
                state.flags.blank = false;
                continue;
            case '\'':
                state.flags.thousands = true;
                continue;
            }
            break;
        }

        // Parse length modifiers
        for (; i < buf.len && rwlj_is_alpha(string[i]); i += 1) {
            switch (string[i]) {
            case 'h':
                state.length_modifier =
                    state.length_modifier == RWLJ_CHAR ? RWLJ_SHORT : RWLJ_CHAR;
                continue;
            case 'l':
                state.length_modifier = RWLJ_LONG;
                continue;
            }
            break;
        }

        // Parse field width
        for (; i < buf.len && rwlj_is_digit(string[i]); i += 1) {
            if (string[i] == '0' && state.field_width <= 0) {
                continue;
            }
            // Also skip extra zeroes before non-zero value
            state.field_width *= 10;
            state.field_width += string[i] - 48;
        }

        // Parse precision
        if (string[i] == '.') {
            i += 1;

            bool ignore_precision = false;
            if (string[i] == '-') {
                ignore_precision = true;
                i += 1;
            }

            for (; i < buf.len && rwlj_is_digit(string[i]); i += 1) {
                if (!ignore_precision) {
                    state.precision *= 10;
                    state.precision += string[i] - 48;
                }
            }
        }

        u8 conv_buf[1024] = { 0 };
        rwljSlice_U8 conv_slice = { .data = conv_buf,
                                    .len = rwlj_size_of_array(conv_buf) };

        // Parse conversion specifiers
        isize conv_bytes_written = 0;
        union {
            i64 i;
            u64 u;
        } value = { 0 };

#pragma GCC diagnostic ignored "-Wanalyzer-va-arg-type-mismatch"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-va-list-exhausted"
#pragma GCC diagnostic push
        switch (string[i]) {
        case 'd':
        case 'i':
            switch (cast(enum rwljLength_Modifier) state.length_modifier) {
            case RWLJ_INT:
            case RWLJ_CHAR:
            case RWLJ_SHORT:
                value.i = cast(i64) va_arg(ap, int);
                break;
            case RWLJ_LONG:
                value.i = cast(i64) va_arg(ap, long);
                break;
            }

            conv_bytes_written = rwlj_format_i64(conv_slice, value.i);
            break;
        case 'x':
        case 'X':
        case 'o':
        case 'u':
            state.base = string[i] == 'o' ? 8 : string[i] == 'u' ? 10 : 16;

            switch (cast(enum rwljLength_Modifier) state.length_modifier) {
            case RWLJ_INT:
            case RWLJ_CHAR:
            case RWLJ_SHORT:
                value.u = cast(u64) va_arg(ap, int);
                break;
            case RWLJ_LONG:
                value.u = cast(u64) va_arg(ap, long);
                break;
            }

            conv_bytes_written =
                rwlj_format_u64(conv_slice, value.u, cast(u64) state.base);
            break;
        case 'a':
        case 'A':
        case 'g':
        case 'G':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
            conv_bytes_written = rwlj_format_f64(
                conv_slice, va_arg(ap, f64), cast(u8) string[i], state.precision
            );
            break;
        case 'p': {
            void *addr = va_arg(ap, void *);
            value.u = addr == NULL ? 0 : cast(uintptr) & addr;

            conv_bytes_written = rwlj_format_u64(conv_slice, value.u, 16);
            break;
        }
        case 's': {
            char *s = va_arg(ap, char *);
            if (s == NULL) {
                continue;
            }
            rwljString str = {
                .data = s,
                .len = rwlj_string_cstrlen(s, buf.len - bytes_written)
            };
            bytes_written += rwlj_format_string(
                (rwljSlice_String){ &buf.data[bytes_written],
                                    buf.len - bytes_written },
                str
            );
            continue;
        }
        case 'S': {
            rwljString *s = va_arg(ap, void *);
            if (s == NULL) {
                continue;
            }
            bytes_written += rwlj_format_string(
                (rwljSlice_String){ &buf.data[bytes_written],
                                    buf.len - bytes_written },
                *s
            );
            continue;
        }
        case 't': {
            rwljString boolean =
                va_arg(ap, int) ? STR_LIT("true") : STR_LIT("false");
            bytes_written += rwlj_format_string(
                (rwljSlice_String){ &buf.data[bytes_written],
                                    buf.len - bytes_written },
                boolean
            );
            continue;
        }
        case 'c':
            value.i = va_arg(ap, int);
            if (rwlj_slice_set(&buf, bytes_written, cast(char) value.i)) {
                bytes_written += 1;
            }
            continue;
        }
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

        if (state.flags.blank && conv_slice.data[0] != '-') {
            if (rwlj_slice_set(&buf, bytes_written, ' ')) {
                bytes_written += 1;
                state.field_width -= 1;
            }
        }

        if (state.flags.sign && conv_slice.data[0] != '-') {
            if (rwlj_slice_set(&buf, bytes_written, '+')) {
                bytes_written += 1;
                state.field_width -= 1;
            }
        }

        for (; state.flags.zero && state.field_width - conv_bytes_written > 0 &&
               bytes_written < buf.len;
             bytes_written += 1, state.field_width -= 1) {
            buf.data[bytes_written] = '0';
        }

        isize offset = conv_bytes_written + bytes_written;
        isize bytes_to_copy = rwlj_min(conv_bytes_written, buf.len - offset);
        memcpy(
            &buf.data[bytes_written], conv_slice.data, cast(usize) bytes_to_copy
        );
        bytes_written += bytes_to_copy;

        for (;
             state.flags.minus && state.field_width - conv_bytes_written > 0 &&
             bytes_written < buf.len;
             bytes_written += 1, state.field_width -= 1) {
            buf.data[bytes_written] = ' ';
        }

        // TODO: alternate flag
    }

    if (has_new_line && rwlj_slice_set(&buf, bytes_written, '\n')) {
        bytes_written += 1;
    }

    return bytes_written;
}

isize
rwlj_bprintf(rwljSlice_String buf, char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_bprintf_va(buf, false, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_bprintfln(rwljSlice_String buf, char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_bprintf_va(buf, true, fmt, ap);
    va_end(ap);

    return len;
}

// TODO: make this thread-safe
isize
__rwlj_fprintf_va(i32 fd, bool has_new_line, char const *fmt, va_list ap)
{
    rwlj_persistent char buf[4096] = { 0 };
    rwljSlice_String buf_slice = { .data = buf,
                                   .len = rwlj_size_of_array(buf) };
    isize len = __rwlj_bprintf_va(buf_slice, has_new_line, fmt, ap);

    return write(fd, buf_slice.data, cast(usize) len);
}

isize
rwlj_fprintf(i32 fd, char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(fd, false, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_fprintfln(i32 fd, char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(fd, true, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_eprintf(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(RWLJ_STDERR, false, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_eprintfln(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(RWLJ_STDERR, true, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_printf(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(RWLJ_STDOUT, false, fmt, ap);
    va_end(ap);

    return len;
}

isize
rwlj_printfln(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    isize len = __rwlj_fprintf_va(RWLJ_STDOUT, true, fmt, ap);
    va_end(ap);

    return len;
}

void
rwlj_string_builder_init(
    rwljString_Builder *sb,
    rwljArena *arena,
    isize capacity
)
{
    sb->buf = rwlj_arena_alloc(arena, cast(usize) capacity);
    sb->capacity = sb->buf != NULL ? capacity : 0;
    sb->len = 0;
}

rwljString
rwlj_string_builder_write_i64(rwljString_Builder *sb, i64 number)
{
    isize start = sb->len;
    sb->len += rwlj_format_i64(
        (rwljSlice_U8){ cast(u8 *) & sb->buf[start], sb->capacity - start },
        number
    );

    return (rwljString){ &sb->buf[start], sb->len - start };
}

rwljString
rwlj_string_builder_write_u64(rwljString_Builder *sb, u64 number, u64 base)
{
    isize start = sb->len;
    sb->len += rwlj_format_u64(
        (rwljSlice_U8){ cast(u8 *) & sb->buf[start], sb->capacity - start },
        number,
        base
    );

    return (rwljString){ &sb->buf[start], sb->len - start };
}

rwljString
rwlj_string_builder_write_f64(
    rwljString_Builder *sb,
    f64 number,
    u8 fmt,
    i64 precision
)
{
    isize start = sb->len;
    sb->len += rwlj_format_f64(
        (rwljSlice_U8){ cast(u8 *) & sb->buf[start], sb->capacity - start },
        number,
        fmt,
        precision
    );

    return (rwljString){ &sb->buf[start], sb->len - start };
}

rwljString
rwlj_string_builder_write_string(rwljString_Builder *sb, rwljString s)
{
    isize start = sb->len;
    sb->len += rwlj_format_string(
        (rwljSlice_String){ &sb->buf[start], sb->capacity - start }, s
    );

    return (rwljString){ &sb->buf[start], sb->len - start };
}

rwljString
rwlj_string_builder_to_string(rwljString_Builder *sb)
{
    return (rwljString){ sb->buf, sb->len };
}

rwljString
rwlj_string_builder_clone(rwljString_Builder *sb, rwljArena *arena)
{
    return rwlj_string_clone(rwlj_string_builder_to_string(sb), arena);
}

void
rwlj_string_builder_clear(rwljString_Builder *sb)
{
    sb->capacity = 0;
    sb->len = 0;
}

rwljString
rwlj_sbprintf(rwljString_Builder *sb, char const *fmt, ...)
{
    isize start = sb->len;
    va_list ap;

    va_start(ap, fmt);
    sb->len += __rwlj_bprintf_va(
        (rwljSlice_String){ &sb->buf[start], sb->capacity - start },
        false,
        fmt,
        ap
    );
    va_end(ap);

    return (rwljString){ &sb->buf[start], sb->len - start };
}

rwljString
rwlj_sbprintfln(rwljString_Builder *sb, char const *fmt, ...)
{
    isize start = sb->len;
    va_list ap;

    va_start(ap, fmt);
    sb->len += __rwlj_bprintf_va(
        (rwljSlice_String){ &sb->buf[start], sb->capacity - start },
        true,
        fmt,
        ap
    );
    va_end(ap);

    return (rwljString){ &sb->buf[start], sb->len - start };
}

#define RWLJ_IMPLEMENTATION

#if defined(RWLJ_IMPLEMENTATION) && !defined(RWLJ_IMPLEMENTATION_DONE)
#define RWLJ_IMPLEMENTATION_DONE
#endif

#endif
