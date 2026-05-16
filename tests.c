#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "vendor/stb_sprintf.h"

#define RWLJ_IMPLEMENTATION
#include "rwlj.h"

typedef void (*rwljTest_Proc)(void);

void
test_math(void)
{
    DESCRIBE("math procedures tests");

    IT("Identifies infinity")
    {
        rwlj_testing_expect(rwlj_is_inf(RWLJ_INFINITY));
        rwlj_testing_expect(rwlj_is_inf(-RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_inf(0.0));
        rwlj_testing_expect(!rwlj_is_inf(-0.0));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_F64_MAX));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_F64_MIN));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_NAN));
    }

    IT("Identifies NaNs")
    {
        rwlj_testing_expect(rwlj_is_nan(RWLJ_NAN));
        rwlj_testing_expect(!rwlj_is_nan(RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_nan(-RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_nan(0.0));
        rwlj_testing_expect(!rwlj_is_nan(-0.0));
        rwlj_testing_expect(!rwlj_is_nan(RWLJ_F64_MAX));
        rwlj_testing_expect(!rwlj_is_nan(RWLJ_F64_MIN));
    }

    f64 min_subnormal = 4.94065645841246544177e-324;
    f64 max_subnormal = 2.22507385850720088902e-308;
    IT("Identifies subnormals")
    {
        rwlj_testing_expect(rwlj_is_subnormal(0.0));
        rwlj_testing_expect(rwlj_is_subnormal(-0.0));
        rwlj_testing_expect(rwlj_is_subnormal(min_subnormal));
        rwlj_testing_expect(rwlj_is_subnormal(max_subnormal));
        rwlj_testing_expect(rwlj_is_subnormal(-min_subnormal));
        rwlj_testing_expect(rwlj_is_subnormal(-max_subnormal));
        rwlj_testing_expect(!rwlj_is_subnormal(RWLJ_NAN));
        rwlj_testing_expect(!rwlj_is_subnormal(RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_subnormal(-RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_subnormal(RWLJ_F64_MAX));
        rwlj_testing_expect(!rwlj_is_subnormal(RWLJ_F64_MIN));
        rwlj_testing_expect(!rwlj_is_subnormal(3.14159265));
    }

    IT("Classifies floats correctly")
    {
        rwlj_testing_expect_value(
            rwlj_classify(RWLJ_NAN), RWLJ_FLOAT_CLASS_NAN
        );
        rwlj_testing_expect_value(
            rwlj_classify(RWLJ_INFINITY), RWLJ_FLOAT_CLASS_INFINITY
        );
        rwlj_testing_expect_value(
            rwlj_classify(-RWLJ_INFINITY), RWLJ_FLOAT_CLASS_INFINITY
        );
        rwlj_testing_expect_value(
            rwlj_classify(min_subnormal), RWLJ_FLOAT_CLASS_SUBNORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(max_subnormal), RWLJ_FLOAT_CLASS_SUBNORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(-min_subnormal), RWLJ_FLOAT_CLASS_SUBNORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(-max_subnormal), RWLJ_FLOAT_CLASS_SUBNORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(RWLJ_F64_MIN), RWLJ_FLOAT_CLASS_NORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(3.14159265), RWLJ_FLOAT_CLASS_NORMAL
        );
        rwlj_testing_expect_value(
            rwlj_classify(RWLJ_F64_MAX), RWLJ_FLOAT_CLASS_NORMAL
        );
    }
}

void
test_arena(void)
{
    DESCRIBE("rwljArena tests");

    IT("Sets growing arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(8);
        rwlj_arena_init_growing(&arena, arena_total_size);

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_GROWING);
        rwlj_testing_expect_value(arena.total_size, arena_total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Sets static arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(1);
        rwlj_arena_init_static(&arena, arena_total_size);

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_STATIC);
        rwlj_testing_expect_value(arena.total_size, arena_total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Sets buffer arena")
    {
#define ARENA_TOTAL_SIZE rwlj_mb(1)
        u8 buf[ARENA_TOTAL_SIZE] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_BUFFER);
        rwlj_testing_expect_value(arena.total_size, ARENA_TOTAL_SIZE);

        rwlj_arena_destroy(&arena);
#undef ARENA_TOTAL_SIZE
    }

    IT("Allocates all memory")
    {
        u8 buf[rwlj_mb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        u8 *allocation = rwlj_arena_alloc(&arena, arena.total_size);

        rwlj_testing_expect(allocation != NULL);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Allocates no memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        u8 *allocation = rwlj_arena_alloc(&arena, 0);

        rwlj_testing_expect(allocation == NULL);
        rwlj_testing_expect_value(arena.allocated_size, 0);

        rwlj_arena_destroy(&arena);
    }

    IT("Arbitrarily allocates memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        usize allocation_num = 8;
        usize total_allocation_size = rwlj_size_of_buf(buf) / allocation_num;
        for (usize i = 0; i < allocation_num * 2; i += 1) {
            u8 *allocation = rwlj_arena_alloc(&arena, total_allocation_size);
            usize iter_allocation_size = total_allocation_size * (i + 1);

            if (i < allocation_num) {
                rwlj_testing_expect(allocation != NULL);
                rwlj_testing_expect_value(
                    arena.allocated_size, iter_allocation_size
                );
            } else {
                rwlj_testing_expect(allocation == NULL);
                rwlj_testing_expect_value(
                    arena.allocated_size, arena.total_size
                );
            }
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Enlarges last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        usize allocation_size = arena.total_size / 4;
        usize allocated_memory = 0;

        u8 *allocation = NULL;
        for (usize i = 0, allocation_num = 3; i < allocation_num; i += 1) {
            allocation = rwlj_arena_alloc(&arena, allocation_size);
            allocated_memory += allocation_size;
        }

        rwlj_testing_expect(allocation != NULL);
        rwlj_testing_expect_value(arena.allocated_size, allocated_memory);

        u8 *resized_allocation = rwlj_arena_resize(
            &arena, allocation, allocation_size, allocation_size * 2
        );

        rwlj_testing_expect(resized_allocation != NULL);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Shrinks last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        usize allocation_size = arena.total_size / 4;
        usize allocated_memory = 0;

        u8 *allocation = NULL;
        for (usize i = 0, allocation_num = 3; i < allocation_num; i += 1) {
            allocation = rwlj_arena_alloc(&arena, allocation_size);
            allocated_memory += allocation_size;
        }

        rwlj_testing_expect(allocation != NULL);
        rwlj_testing_expect_value(arena.allocated_size, allocated_memory);

        u8 *resized_mem = rwlj_arena_resize(
            &arena, allocation, allocation_size, allocation_size / 2
        );
        allocated_memory -= (allocation_size / 2);

        rwlj_testing_expect(resized_mem != NULL);
        rwlj_testing_expect_value(arena.allocated_size, allocated_memory);

        rwlj_arena_destroy(&arena);
    }
}

void
test_slice(void)
{
    DESCRIBE("rwljSlice tests");

    // Continue refactoring from here
    IT("Doesn't get value before index 0 (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(!rwlj_slice_get(&slice, -1));
    }

    IT("Doesn't get value beyond last index (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(!rwlj_slice_get(&slice, slice.len));
    }

    IT("Gets value at index 0")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(rwlj_slice_get(&slice, 0));
    }

    IT("Gets value at last index")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(rwlj_slice_get(&slice, slice.len - 1));
    }

    IT("Doesn't set value before at index 0 (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(!rwlj_slice_set(&slice, -1, 22));
    }

    IT("Doesn't set value beyond last index (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(!rwlj_slice_set(&slice, slice.len, 22));
    }

    IT("Sets value at index 0")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(rwlj_slice_set(&slice, 0, 22));
        rwlj_testing_expect_value(rwlj_slice_get(&slice, 0), 22);
    }

    IT("Sets value at last index")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_testing_expect(rwlj_slice_set(&slice, slice.len - 1, 22));
        rwlj_testing_expect_value(rwlj_slice_get(&slice, slice.len - 1), 22);
    }

    IT("Reverses slice")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        isize reversed_buf[] = {
            8, 7, 6, 5, 4, 3, 2, 1,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };
        rwljSlice_Isize reversed_slice = { reversed_buf,
                                           rwlj_size_of_array(reversed_buf) };

        rwlj_slice_reverse(&slice, isize);
        for (isize i = 0; i < slice.len; i += 1) {
            rwlj_testing_expect_value(slice.data[i], reversed_slice.data[i]);
        }
    }

    IT("Clears slice")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = { buf, rwlj_size_of_array(buf) };

        rwlj_slice_clear(&slice);
        for (isize i = 0; i < slice.len; i += 1) {
            rwlj_testing_expect_value(slice.data[i], 0);
        }
    }

    IT("Sorts slice")
    {
        // To be implemented
    }
}

void
test_array(void)
{
    DESCRIBE("rwljArray tests");

    IT("Creates dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, RWLJ_GROW_FORMULA(0));

        rwlj_arena_destroy(&arena);
    }

    IT("Creates fixed array")
    {
        isize backing_array[8] = { 0 };
        rwljArray_Isize array = { 0 };
        rwlj_array_init_fixed(&array, backing_array);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_FIXED);
        rwlj_testing_expect_value(
            array.capacity, rwlj_size_of_array(backing_array)
        );
    }

    IT("Creates and reserves dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, array_cap);

        rwlj_arena_destroy(&arena);
    }

    IT("Reserves at least N elements according to grow formula")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic_reserve(&array, 0, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, RWLJ_GROW_FORMULA(0));

        rwlj_arena_destroy(&arena);
    }

    IT("Grows dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(rwlj_array_grow(&array));

        isize array_new_len = RWLJ_GROW_FORMULA(array_cap);
        rwlj_testing_expect_value(array.capacity, array_new_len);

        rwlj_arena_destroy(&arena);
    }

    IT("Grows dynamic array up to arena's total size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        isize max_len = rwlj_size_of_buf(buf) / rwlj_size_of(isize);
        rwlj_testing_expect(rwlj_array_resize(&array, max_len));
        rwlj_testing_expect_value(array.capacity, max_len);

        rwlj_arena_destroy(&arena);
    }

    // TODO: clean this up
    IT("Doesn't resize dynamic array past arena's total size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        isize max_len = cast(isize)(arena.total_size - arena.allocated_size) /
                        rwlj_size_of(isize);
        rwlj_testing_expect(rwlj_array_resize(&array, max_len * max_len));
        rwlj_testing_expect_value(
            array.capacity, cast(isize)(arena.total_size / array.item_size)
        );

        rwlj_arena_destroy(&arena);
    }

    // TODO: clean this up
    IT("Doesn't resize dynamic array past arena's total size after previous "
       "allocations")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        u8 *_ = rwlj_arena_alloc(&arena, 128);
        _ = rwlj_arena_alloc(&arena, 128);
        usize allocated = arena.allocated_size;
        rwlj_unused(_);

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        isize max_len = cast(isize)(arena.total_size - arena.allocated_size) /
                        rwlj_size_of(isize);
        rwlj_testing_expect(rwlj_array_resize(&array, max_len * max_len));
        rwlj_testing_expect_value(
            array.capacity,
            cast(isize)((arena.total_size - allocated) / array.item_size)
        );

        rwlj_arena_destroy(&arena);
    }

    IT("Shrinks dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect_value(array.capacity, array_cap);

        isize array_new_size = RWLJ_GROW_FORMULA(0);
        rwlj_testing_expect(rwlj_array_resize(&array, array_new_size));
        rwlj_testing_expect_value(array.capacity, array_new_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Appends to dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0; i < array.capacity; i += 1) {
            rwlj_testing_expect(rwlj_array_append(&array, i));
        }
        rwlj_testing_expect_value(array.len, array.capacity);

        isize expected_values[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Appends to dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize old_capacity = array.capacity;
        isize new_capacity = RWLJ_GROW_FORMULA(array.capacity);
        for (isize i = 0, n = array.capacity; i < n + 1; i += 1) {
            rwlj_testing_expect(rwlj_array_append(&array, i));
        }
        rwlj_testing_expect_value(array.len, old_capacity + 1);
        rwlj_testing_expect_value(array.capacity, new_capacity);

        isize expected_values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Arbitrarily appends to dynamic array and grows up to arena's total "
       "size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize new_capacity = cast(isize)(arena.total_size / array.item_size);
        isize arbitrary_limit = new_capacity * new_capacity;
        for (isize i = 0; i < arbitrary_limit; i += 1) {
            if (i < new_capacity) {
                rwlj_testing_expect(rwlj_array_append(&array, i));
            } else {
                rwlj_testing_expect(!rwlj_array_append(&array, i));
            }
        }
        rwlj_testing_expect_value(array.len, new_capacity);
        rwlj_testing_expect_value(array.capacity, new_capacity);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Preppends to dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0; i < array.capacity; i += 1) {
            rwlj_testing_expect(rwlj_array_insert(&array, 0, i));
        }
        rwlj_testing_expect_value(array.len, array.capacity);

        isize expected_values[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Preppends to dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize old_capacity = array.capacity;
        isize new_capacity = RWLJ_GROW_FORMULA(array.capacity);
        for (isize i = 0, n = array.capacity; i < n + 1; i += 1) {
            rwlj_testing_expect(rwlj_array_insert(&array, 0, i));
        }
        rwlj_testing_expect_value(array.len, old_capacity + 1);
        rwlj_testing_expect_value(array.capacity, new_capacity);

        isize expected_values[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Arbitrarily preppends to dynamic array and grows up to arena's total "
       "size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize new_capacity = cast(isize)(arena.total_size / array.item_size);
        isize arbitrary_limit = new_capacity * new_capacity;
        for (isize i = 0; i < arbitrary_limit; i += 1) {
            if (i < new_capacity) {
                rwlj_testing_expect(rwlj_array_insert(&array, 0, i));
            } else {
                rwlj_testing_expect(!rwlj_array_insert(&array, 0, i));
            }
        }
        rwlj_testing_expect_value(array.len, new_capacity);
        rwlj_testing_expect_value(array.capacity, new_capacity);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Inserts into dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0; i < array.capacity; i += 1) {
            rwlj_testing_expect(rwlj_array_insert(&array, array.len / 2, i));
        }
        rwlj_testing_expect_value(array.len, array.capacity);

        isize expected_values[] = { 1, 3, 5, 7, 6, 4, 2, 0 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Inserts into dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize old_capacity = array.capacity;
        isize new_capacity = RWLJ_GROW_FORMULA(array.capacity);
        for (isize i = 0, n = array.capacity; i < n + 1; i += 1) {
            rwlj_testing_expect(rwlj_array_insert(&array, array.len / 2, i));
        }
        rwlj_testing_expect_value(array.len, old_capacity + 1);
        rwlj_testing_expect_value(array.capacity, new_capacity);

        isize expected_values[] = { 1, 3, 5, 7, 8, 6, 4, 2, 0 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Arbitrarily inserts into dynamic array and grows up to arena's total "
       "size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        isize new_capacity = cast(isize)(arena.total_size / array.item_size);
        isize arbitrary_limit = new_capacity * new_capacity;
        for (isize i = 0; i < arbitrary_limit; i += 1) {
            if (i < new_capacity) {
                rwlj_testing_expect(rwlj_array_insert(&array, 0, i));
            } else {
                rwlj_testing_expect(!rwlj_array_insert(&array, 0, i));
            }
        }
        rwlj_testing_expect_value(array.len, new_capacity);
        rwlj_testing_expect_value(array.capacity, new_capacity);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Dynamic array works as a stack")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0, n = array.capacity; i < n; i += 1) {
            rwlj_testing_expect(rwlj_array_append(&array, i + 1));
        }

        isize expected_values[] = { 8, 7, 6, 5, 4, 3, 2, 1 };
        for (isize i = 0, n = array.len; i < n; i += 1) {
            rwlj_testing_expect_value(
                rwlj_array_pop(&array), expected_values[i]
            );
        }
        rwlj_testing_expect_value(array.len, 0);
        rwlj_testing_expect(!rwlj_array_pop(&array));

        rwlj_arena_destroy(&arena);
    }

    IT("Removes elements in an unordered fashion")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0, n = array.capacity; i < n; i += 1) {
            rwlj_testing_expect(rwlj_array_append(&array, i + 1));
        }

        for (isize i = 0, n = array.len; i < n; i += 2) {
            rwlj_array_remove_unordered(&array, i);
        }

        isize expected_values[] = { 8, 2, 7, 4, 6 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }
        rwlj_testing_expect_value(array.len, 5);

        rwlj_arena_destroy(&arena);
    }

    IT("Removes elements in an ordered fashion")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_buf(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        for (isize i = 0, n = array.capacity; i < n; i += 1) {
            rwlj_testing_expect(rwlj_array_append(&array, i + 1));
        }

        for (isize i = 0, n = array.len; i < n; i += 2) {
            rwlj_array_remove_ordered(&array, i);
        }

        isize expected_values[] = { 2, 3, 5, 6, 8 };
        for (isize i = 0; i < array.len; i += 1) {
            rwlj_testing_expect_value(array.data[i], expected_values[i]);
        }
        rwlj_testing_expect_value(array.len, 5);

        rwlj_arena_destroy(&arena);
    }
}

void
test_formatting(void)
{
    DESCRIBE("Formatting tests");

    IT("Prints text")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        rwljString string = { cast(char *) buf_slice.data,
                              rwlj_bprintf(buf_slice, "Hello, World!") };
        rwljString expected = STR_LIT("Hello, World!");
        rwlj_testing_expect_value(string, expected);
    }

    IT("Prints nothing")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        rwljString string = { cast(char *) buf_slice.data,
                              rwlj_bprintf(buf_slice, "") };
        rwljString expected = STR_LIT("");
        rwlj_testing_expect_value(string, expected);
    }

    IT("Prints the whole buffer")
    {
#define STRING                                                                 \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

        u8 buf[rwlj_kb(1) / 8] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        for (isize i = 0; i < buf_slice.len; i += 1) {
            buf_slice.data[i] = 'a';
        }

        rwljString string = { cast(char *) buf_slice.data,
                              rwlj_bprintf(buf_slice, STRING) };
        rwljString expected = STR_LIT(STRING);
        rwlj_testing_expect_value(string, expected);
#undef STRING
    }

    IT("Prints and doesn't go beyong buffer")
    {
#define STRING                                                                 \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define BUF_LEN (rwlj_kb(1) / 16)

        u8 buf[BUF_LEN] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        for (isize i = 0; i < buf_slice.len; i += 1) {
            buf_slice.data[i] = 'a';
        }

        rwljString string = { cast(char *) buf_slice.data,
                              rwlj_bprintf(buf_slice, STRING) };
        rwljString expected = { STRING, BUF_LEN };
        rwlj_testing_expect_value(string, expected);
#undef BUF_LEN
#undef STRING
    }

    IT("Formats signed integer")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        {
            isize value = RWLJ_I64_MIN;
            rwljString string = { cast(char *) buf_slice.data,
                                  rwlj_bprintf(buf_slice, "|%ld|", value) };
            rwljString expected = STR_LIT("|-9223372036854775808|");
            rwlj_testing_expect_value(string, expected);
        }
        {
            isize value = RWLJ_I64_MAX;
            rwljString string = { cast(char *) buf_slice.data,
                                  rwlj_bprintf(buf_slice, "|%ld|", value) };
            rwljString expected = STR_LIT("|9223372036854775807|");
            rwlj_testing_expect_value(string, expected);
        }
    }

    IT("Formats unsigned integer")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = { buf, rwlj_size_of_buf(buf) };

        {
            usize value = RWLJ_U64_MIN;
            rwljString string = { cast(char *) buf_slice.data,
                                  rwlj_bprintf(buf_slice, "|%lu|", value) };
            rwljString expected = STR_LIT("|0|");
            rwlj_testing_expect_value(string, expected);
        }
        {
            usize value = RWLJ_U64_MAX;
            rwljString string = { cast(char *) buf_slice.data,
                                  rwlj_bprintf(buf_slice, "|%lu|", value) };
            rwljString expected = STR_LIT("|18446744073709551615|");
            rwlj_testing_expect_value(string, expected);
        }
    }

    typedef struct rwljF64_Test {
        char *fmt;
        f64 value;
    } rwljF64_Test;

    // Table 3: Stress Inputs for Converting 53-bit Binary to Decimal, <
    // 1/2 ULP
    // Table 4: Stress Inputs for Converting 53-bit Binary to Decimal, >
    // 1/2 ULP
#define F64_TEST_TABLE(fmt)                                                    \
    { "%.0" #fmt, ldexp(8511030020275656.0, -342) },                           \
        { "%.1" #fmt, ldexp(5201988407066741.0, -824) },                       \
        { "%.2" #fmt, ldexp(6406892948269899.0, +237) },                       \
        { "%.3" #fmt, ldexp(8431154198732492.0, +72) },                        \
        { "%.4" #fmt, ldexp(6475049196144587.0, +99) },                        \
        { "%.5" #fmt, ldexp(8274307542972842.0, +726) },                       \
        { "%.6" #fmt, ldexp(5381065484265332.0, -456) },                       \
        { "%.7" #fmt, ldexp(6761728585499734.0, -1057) },                      \
        { "%.8" #fmt, ldexp(7976538478610756.0, +376) },                       \
        { "%.9" #fmt, ldexp(5982403858958067.0, +377) },                       \
        { "%.10" #fmt, ldexp(5536995190630837.0, +93) },                       \
        { "%.11" #fmt, ldexp(7225450889282194.0, +710) },                      \
        { "%.12" #fmt, ldexp(7225450889282194.0, +709) },                      \
        { "%.13" #fmt, ldexp(8703372741147379.0, +117) },                      \
        { "%.14" #fmt, ldexp(8944262675275217.0, -1001) },                     \
        { "%.15" #fmt, ldexp(7459803696087692.0, -707) },                      \
        { "%.16" #fmt, ldexp(6080469016670379.0, -381) },                      \
        { "%.17" #fmt, ldexp(8385515147034757.0, +721) },                      \
        { "%.18" #fmt, ldexp(7514216811389786.0, -828) },                      \
        { "%.19" #fmt, ldexp(8397297803260511.0, -345) },                      \
        { "%.20" #fmt, ldexp(6733459239310543.0, +202) },                      \
        { "%.21" #fmt, ldexp(8091450587292794.0, -473) },                      \
        { "%.0" #fmt, ldexp(6567258882077402.0, +952) },                       \
        { "%.1" #fmt, ldexp(6712731423444934.0, +535) },                       \
        { "%.2" #fmt, ldexp(6712731423444934.0, +534) },                       \
        { "%.3" #fmt, ldexp(5298405411573037.0, -957) },                       \
        { "%.4" #fmt, ldexp(5137311167659507.0, -144) },                       \
        { "%.5" #fmt, ldexp(6722280709661868.0, +363) },                       \
        { "%.6" #fmt, ldexp(5344436398034927.0, -169) },                       \
        { "%.7" #fmt, ldexp(8369123604277281.0, -853) },                       \
        { "%.8" #fmt, ldexp(8995822108487663.0, -780) },                       \
        { "%.9" #fmt, ldexp(8942832835564782.0, -383) },                       \
        { "%.10" #fmt, ldexp(8942832835564782.0, -384) },                      \
        { "%.11" #fmt, ldexp(8942832835564782.0, -385) },                      \
        { "%.12" #fmt, ldexp(6965949469487146.0, -249) },                      \
        { "%.13" #fmt, ldexp(6965949469487146.0, -250) },                      \
        { "%.14" #fmt, ldexp(6965949469487146.0, -251) },                      \
        { "%.15" #fmt, ldexp(7487252720986826.0, +548) },                      \
        { "%.16" #fmt, ldexp(5592117679628511.0, +164) },                      \
        { "%.17" #fmt, ldexp(8887055249355788.0, +665) },                      \
        { "%.18" #fmt, ldexp(6994187472632449.0, +690) },                      \
        { "%.19" #fmt, ldexp(8797576579012143.0, +588) },                      \
        { "%.20" #fmt, ldexp(7363326733505337.0, +272) },                      \
        { "%.21" #fmt, ldexp(8549497411294502.0, -448) },

    IT("Formats floats with f")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(f) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = { ftoa_buf, rwlj_size_of_buf(ftoa_buf) };

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = { printf_buf,
                                      rwlj_size_of_buf(printf_buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljF64_Test test = tests[i];

            isize rwlj_len = rwlj_bprintf(ftoa_slice, test.fmt, test.value);

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString rwlj_string = { cast(char *) ftoa_slice.data, rwlj_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    rwlj_string,
                    rwlj_len
                );
                success = false;
            }
        }
    }

    IT("Formats floats with g")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(g) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = { ftoa_buf, rwlj_size_of_buf(ftoa_buf) };

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = { printf_buf,
                                      rwlj_size_of_buf(printf_buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljF64_Test test = tests[i];

            isize rwlj_len = rwlj_bprintf(ftoa_slice, test.fmt, test.value);

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString rwlj_string = { cast(char *) ftoa_slice.data, rwlj_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    rwlj_string,
                    rwlj_len
                );
                success = false;
            }
        }
    }

    IT("Formats floats with e")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(e) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = { ftoa_buf, rwlj_size_of_buf(ftoa_buf) };

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = { printf_buf,
                                      rwlj_size_of_buf(printf_buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljF64_Test test = tests[i];

            isize rwlj_len = rwlj_bprintf(ftoa_slice, test.fmt, test.value);

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString rwlj_string = { cast(char *) ftoa_slice.data, rwlj_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    rwlj_string,
                    rwlj_len
                );
                success = false;
            }
        }
    }

    IT("Formats floats with a")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(a) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = { ftoa_buf, rwlj_size_of_buf(ftoa_buf) };

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = { printf_buf,
                                      rwlj_size_of_buf(printf_buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljF64_Test test = tests[i];

            isize rwlj_len = rwlj_bprintf(ftoa_slice, test.fmt, test.value);

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString rwlj_string = { cast(char *) ftoa_slice.data, rwlj_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    rwlj_string,
                    rwlj_len
                );
                success = false;
            }
        }
    }
#undef F64_TEST_TABLE

    IT("Formats floats")
    {
        typedef struct rwljF64_Single_Test {
            f64 value;
            i64 prec;
            char fmt;
            char *printf_fmt;
        } rwljF64_Single_Test;

        typedef struct rwljF64_Double_Test {
            f64 value[2];
            i64 prec;
            char fmt;
            char *printf_fmt;
        } rwljF64_Double_Test;

        rwljF64_Single_Test tests_single[] = {
            { -3.0, -1, 'f', "%f" },
            { -8.88888888, 10, 'f', "%.10f" },
            { -880.88888888, 10, 'f', "%.10f" },
            { 4.1, 1, 'f', "%.1f" },
            { 0.1, 0, 'f', "%.0f" },
            { 1e-4, 2, 'f', "%.2f" },
            { -5.2, 2, 'f', "%.2f" },
            { 0., 1, 'f', "%.1f" },
            { -0., -1, 'f', "%f" },
            { 9.09834e-07, -1, 'f', "%f" },
            { 38685626227668133590597632.0, 1, 'f', "%.1f" }, // 10
            { 5e-7, 24, 'f', "%.24f" },
            { 1e-8, 10, 'f', "%.10f" },
            { 100056789.0, 1, 'f', "%.1f" },
            { 1.23, 2, 'f', "%.2f" },
            { -3.0, -1, 'e', "%e" },
            { 4.1, 1, 'E', "%.1E" },
            { -5.2, 2, 'e', "%.2e" },
            { 3.14159265, -1, 'g', "%g" },
            { 4.1, 1, 'G', "%.1G" },
            { 3e-300, -1, 'g', "%g" },
            { 1.2, 0, 'g', "%.0g" },
        };

        rwljF64_Double_Test tests_double[] = {
            { { 0.3, -3.0 }, -1, 'g', "%g %g" },
            { { 3.704, 3.706 }, 3, 'g', "%.3g %.3g" },
        };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = { ftoa_buf, rwlj_size_of_buf(ftoa_buf) };

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = { printf_buf,
                                      rwlj_size_of_buf(printf_buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests_single); i += 1) {
            rwljF64_Single_Test test = tests_single[i];

            isize rwlj_len =
                rwlj_bprintf(ftoa_slice, test.printf_fmt, test.value);

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.printf_fmt,
                test.value
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString rwlj_string = { cast(char *) ftoa_slice.data, rwlj_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    rwlj_string,
                    rwlj_len
                );
                success = false;
            }
        }

        for (isize i = 0; i < rwlj_size_of_array(tests_double); i += 1) {
            rwljF64_Double_Test test = tests_double[i];

            isize ftoa_len = rwlj_bprintf(
                ftoa_slice, test.printf_fmt, test.value[0], test.value[1]
            );

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(usize) printf_slice.len,
                test.printf_fmt,
                test.value[0],
                test.value[1]
            );
            isize printf_len =
                cast(isize) strlen(cast(char *) printf_slice.data);

            rwljString ftoa_string = { cast(char *) ftoa_slice.data, ftoa_len };
            rwljString printf_string = { cast(char *) printf_slice.data,
                                         printf_len };

            if (!rwlj_string_are_equal(ftoa_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_len,
                    ftoa_string,
                    ftoa_len
                );
                success = false;
            }
        }
    }
}

void
test_string_builder(void)
{
    DESCRIBE("String Builder tests");
}

// clang-format off
rwljTest_Proc test_arr[] = {
    test_math,
    test_arena,
    test_slice,
    test_array,
    test_formatting,
    test_string_builder,
};
// clang-format on

int
main(void)
{
    for (isize i = 0; i < rwlj_size_of_array(test_arr); i += 1) {
        (test_arr[i])();
        rwlj_printfln("");
    }
}
