#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "vendor/stb_sprintf.h"

#define RWLJ_IMPLEMENTATION
#include "rwlj.h"

// The dumbest tests you'll ever gonna see

typedef void (*rwljTest_Proc)(void);

void
test_math(void)
{
    DESCRIBE("math procedures tests");

    TEST("Identifies infinity")
    {
        rwlj_testing_expect(rwlj_is_inf(RWLJ_INFINITY));
        rwlj_testing_expect(rwlj_is_inf(-RWLJ_INFINITY));
        rwlj_testing_expect(!rwlj_is_inf(0.0));
        rwlj_testing_expect(!rwlj_is_inf(-0.0));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_F64_MAX));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_F64_MIN));
        rwlj_testing_expect(!rwlj_is_inf(RWLJ_NAN));
    }

    TEST("Identifies NaNs")
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
    TEST("Identifies subnormals")
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

    TEST("Classifies floats correctly")
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

    TEST("Sets growing arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(8);
        rwlj_arena_init_growing(&arena, arena_total_size);

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_GROWING);
        rwlj_testing_expect_value(arena.total_size, arena_total_size);

        rwlj_arena_destroy(&arena);
    }

    TEST("Sets static arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(1);
        rwlj_arena_init_static(&arena, arena_total_size);

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_STATIC);
        rwlj_testing_expect_value(arena.total_size, arena_total_size);

        rwlj_arena_destroy(&arena);
    }

    TEST("Sets buffer arena")
    {
#define ARENA_TOTAL_SIZE rwlj_mb(1)
        u8 buf[ARENA_TOTAL_SIZE] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_BUFFER);
        rwlj_testing_expect_value(arena.total_size, ARENA_TOTAL_SIZE);

        rwlj_arena_destroy(&arena);
#undef ARENA_TOTAL_SIZE
    }

    TEST("Allocates all memory")
    {
        u8 buf[rwlj_mb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        u8 *allocation = rwlj_arena_alloc(&arena, arena.total_size);

        rwlj_testing_expect(allocation != NULL);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    TEST("Allocates no memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        u8 *allocation = rwlj_arena_alloc(&arena, 0);

        rwlj_testing_expect(allocation == NULL);
        rwlj_testing_expect_value(arena.allocated_size, 0);

        rwlj_arena_destroy(&arena);
    }

    TEST("Arbitrarily allocates memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        usize allocation_num = 8;
        usize total_allocation_size = rwlj_count_of(buf) / allocation_num;
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

    TEST("Enlarges last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Shrinks last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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
    TEST("Doesn't get value before index 0 (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(!rwlj_slice_get(&slice, -1));
    }

    TEST("Doesn't get value beyond last index (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(!rwlj_slice_get(&slice, slice.len));
    }

    TEST("Gets value at index 0")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(rwlj_slice_get(&slice, 0));
    }

    TEST("Gets value at last index")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(rwlj_slice_get(&slice, slice.len - 1));
    }

    TEST("Doesn't set value before at index 0 (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(!rwlj_slice_set(&slice, -1, 22));
    }

    TEST("Doesn't set value beyond last index (out of bounds)")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(!rwlj_slice_set(&slice, slice.len, 22));
    }

    TEST("Sets value at index 0")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(rwlj_slice_set(&slice, 0, 22));
        rwlj_testing_expect_value(rwlj_slice_get(&slice, 0), 22);
    }

    TEST("Sets value at last index")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_testing_expect(rwlj_slice_set(&slice, slice.len - 1, 22));
        rwlj_testing_expect_value(rwlj_slice_get(&slice, slice.len - 1), 22);
    }

    TEST("Reverses slice")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        isize reversed_buf[] = {
            8, 7, 6, 5, 4, 3, 2, 1,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);
        rwljSlice_Isize reversed_slice = rwlj_slice_from_array(reversed_buf);

        rwlj_slice_reverse(&slice, isize);
        for (isize i = 0; i < slice.len; i += 1) {
            rwlj_testing_expect_value(slice.data[i], reversed_slice.data[i]);
        }
    }

    TEST("Clears slice")
    {
        isize buf[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
        };
        rwljSlice_Isize slice = rwlj_slice_from_array(buf);

        rwlj_slice_clear(&slice);
        for (isize i = 0; i < slice.len; i += 1) {
            rwlj_testing_expect_value(slice.data[i], 0);
        }
    }

    TEST("Sorts slice")
    {
        // To be implemented
    }
}

void
test_array(void)
{
    DESCRIBE("rwljArray tests");

    TEST("Creates dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic(&array, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, RWLJ_GROW_FORMULA(0));

        rwlj_arena_destroy(&arena);
    }

    TEST("Creates fixed array")
    {
        isize backing_array[8] = { 0 };
        rwljArray_Isize array = { 0 };
        rwlj_array_init_fixed(&array, backing_array);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_FIXED);
        rwlj_testing_expect_value(array.capacity, rwlj_count_of(backing_array));
    }

    TEST("Creates and reserves dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, array_cap);

        rwlj_arena_destroy(&arena);
    }

    TEST("Reserves at least N elements according to grow formula")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        rwlj_array_init_dynamic_reserve(&array, 0, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, RWLJ_GROW_FORMULA(0));

        rwlj_arena_destroy(&arena);
    }

    TEST("Grows dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(rwlj_array_grow(&array));

        isize array_new_len = RWLJ_GROW_FORMULA(array_cap);
        rwlj_testing_expect_value(array.capacity, array_new_len);

        rwlj_arena_destroy(&arena);
    }

    TEST("Grows dynamic array up to arena's total size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        isize max_len = rwlj_count_of(buf) / rwlj_size_of(isize);
        rwlj_testing_expect(rwlj_array_resize(&array, max_len));
        rwlj_testing_expect_value(array.capacity, max_len);

        rwlj_arena_destroy(&arena);
    }

    // TODO: clean this up
    TEST("Doesn't resize dynamic array past arena's total size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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
    TEST(
        "Doesn't resize dynamic array past arena's total size after previous "
        "allocations"
    )
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Shrinks dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect_value(array.capacity, array_cap);

        isize array_new_size = RWLJ_GROW_FORMULA(0);
        rwlj_testing_expect(rwlj_array_resize(&array, array_new_size));
        rwlj_testing_expect_value(array.capacity, array_new_size);

        rwlj_arena_destroy(&arena);
    }

    TEST("Appends to dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Appends to dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST(
        "Arbitrarily appends to dynamic array and grows up to arena's total "
        "size"
    )
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Preppends to dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Preppends to dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST(
        "Arbitrarily preppends to dynamic array and grows up to arena's total "
        "size"
    )
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Inserts into dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Inserts into dynamic array and grows")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST(
        "Arbitrarily inserts into dynamic array and grows up to arena's total "
        "size"
    )
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Dynamic array works as a stack")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Removes elements in an unordered fashion")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Removes elements in an ordered fashion")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

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

    TEST("Prints text")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        rwljString s = rwlj_string_from_fmt(&buf_slice, "Hello, World!");
        rwljString expected = STRING("Hello, World!");
        rwlj_testing_expect_value(s, expected);
    }

    TEST("Prints nothing")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        rwljString s = rwlj_string_from_fmt(&buf_slice, "");
        rwljString expected = STRING("");
        rwlj_testing_expect_value(s, expected);
    }

#define TEST_STRING                                                            \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    TEST("Prints the whole buffer")
    {

        u8 buf[rwlj_kb(1) / 8] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        for (isize i = 0; i < buf_slice.len; i += 1) {
            buf_slice.data[i] = 'a';
        }

        rwljString s = rwlj_string_from_fmt(&buf_slice, TEST_STRING);
        rwljString expected = STRING(TEST_STRING);
        rwlj_testing_expect_value(s, expected);
    }

    TEST("Prints and doesn't go beyong buffer")
    {
#define BUF_LEN (rwlj_kb(1) / 16)

        u8 buf[BUF_LEN] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        for (isize i = 0; i < buf_slice.len; i += 1) {
            buf_slice.data[i] = 'a';
        }

        rwljString s = rwlj_string_from_fmt(&buf_slice, TEST_STRING);
        rwljString expected = { TEST_STRING, BUF_LEN };
        rwlj_testing_expect_value(s, expected);
#undef BUF_LEN
    }
#undef TEST_STRING

    TEST("Formats signed integer")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = RWLJ_I64_MIN;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%ld|", value);
            rwljString expected = STRING("|-9223372036854775808|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = RWLJ_I64_MAX;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%ld|", value);
            rwljString expected = STRING("|9223372036854775807|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Formats unsigned integer")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            usize value = RWLJ_U64_MIN;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%lu|", value);
            rwljString expected = STRING("|0|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            usize value = RWLJ_U64_MAX;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%lu|", value);
            rwljString expected = STRING("|18446744073709551615|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Formats base 2")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%lb|", value);
            rwljString expected =
                STRING("|1101101001101000101110010010111001001001|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%#lb|", value);
            rwljString expected =
                STRING("|0b1101101001101000101110010010111001001001|");
            rwlj_testing_expect_value(s, expected);
        }
    }
    TEST("Formats base 8")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%lo|", value);
            rwljString expected = STRING("|15515056227111|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%#lo|", value);
            rwljString expected = STRING("|0o15515056227111|");
            rwlj_testing_expect_value(s, expected);
        }
    }
    TEST("Formats base 16")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%lx|", value);
            rwljString expected = STRING("|da68b92e49|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%#lx|", value);
            rwljString expected = STRING("|0xda68b92e49|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            usize value = 938059837001llu;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%#lX|", value);
            rwljString expected = STRING("|0XDA68B92E49|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Prints pointer")
    {
        u8 buf1[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf1_slice = rwlj_slice_from_buf(buf1);

        u8 buf2[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf2_slice = rwlj_slice_from_buf(buf2);

        {
            void *addr = buf1;
            rwljString s = rwlj_string_from_fmt(&buf1_slice, "|%p|", addr);
            rwljString expected = {
                cast(char *) buf2_slice.data,
                snprintf(
                    cast(char *) buf2_slice.data, buf2_slice.len, "|%p|", addr
                )
            };
            rwlj_testing_expect_value(s, expected);
        }
        {
            void *addr = NULL;
            rwljString s = rwlj_string_from_fmt(&buf1_slice, "|%p|", addr);
            rwljString expected = STRING("|<nil>|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Prints boolean")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%t|", true);
            rwljString expected = STRING("|true|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%t|", false);
            rwljString expected = STRING("|false|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Prints sign")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = 22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%+ld|", value);
            rwljString expected = STRING("|+22|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = -22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%+ld|", value);
            rwljString expected = STRING("|-22|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Prints leading blank")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = 22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|% ld|", value);
            rwljString expected = STRING("| 22|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = -22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|% ld|", value);
            rwljString expected = STRING("|-22|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Ignore certain combinations of flags")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = 22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|% +ld|", value);
            rwljString expected = STRING("|+22|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = -22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|% +ld|", value);
            rwljString expected = STRING("|-22|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Pads with 0s")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = 22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05ld|", value);
            rwljString expected = STRING("|00022|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = -22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05ld|", value);
            rwljString expected = STRING("|-0022|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 0;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05ld|", value);
            rwljString expected = STRING("|00000|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 12345;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05ld|", value);
            rwljString expected = STRING("|12345|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 1234567;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05ld|", value);
            rwljString expected = STRING("|1234567|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            f64 value = 2.5;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05g|", value);
            rwljString expected = STRING("|002.5|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            f64 value = -2.5;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%05g|", value);
            rwljString expected = STRING("|-02.5|");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Left justifies")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        {
            isize value = 22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5ld|", value);
            rwljString expected = STRING("|22   |");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = -22;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5ld|", value);
            rwljString expected = STRING("|-22  |");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 0;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5ld|", value);
            rwljString expected = STRING("|0    |");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 12345;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5ld|", value);
            rwljString expected = STRING("|12345|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            isize value = 1234567;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5ld|", value);
            rwljString expected = STRING("|1234567|");
            rwlj_testing_expect_value(s, expected);
        }
        {
            f64 value = 2.5;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5g|", value);
            rwljString expected = STRING("|2.5  |");
            rwlj_testing_expect_value(s, expected);
        }
        {
            f64 value = -2.5;
            rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-5g|", value);
            rwljString expected = STRING("|-2.5 |");
            rwlj_testing_expect_value(s, expected);
        }
    }

    TEST("Left justify overrides zero pads")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 buf_slice = rwlj_slice_from_buf(buf);

        isize value = 22;
        rwljString s = rwlj_string_from_fmt(&buf_slice, "|%-05ld|", value);
        rwljString expected = STRING("|22   |");
        rwlj_testing_expect_value(s, expected);
    }

    TEST("") {}

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

    TEST("Formats floats with f")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(f) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = rwlj_slice_from_buf(ftoa_buf);

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = rwlj_slice_from_buf(printf_buf);

        for (isize i = 0; i < rwlj_count_of(tests); i += 1) {
            rwljF64_Test test = tests[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            rwljString printf_string = rwlj_string(
                printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString rwlj_string =
                rwlj_string_from_fmt(&ftoa_slice, test.fmt, test.value);

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    rwlj_string,
                    rwlj_string.len
                );
                success = false;
            }
        }
    }

    TEST("Formats floats with e")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(e) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = rwlj_slice_from_buf(ftoa_buf);

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = rwlj_slice_from_buf(printf_buf);

        for (isize i = 0; i < rwlj_count_of(tests); i += 1) {
            rwljF64_Test test = tests[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            rwljString printf_string = rwlj_string(
                printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString rwlj_string =
                rwlj_string_from_fmt(&ftoa_slice, test.fmt, test.value);

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    rwlj_string,
                    rwlj_string.len
                );
                success = false;
            }
        }
    }

    TEST("Formats floats with g")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(g) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = rwlj_slice_from_buf(ftoa_buf);

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = rwlj_slice_from_buf(printf_buf);

        for (isize i = 0; i < rwlj_count_of(tests); i += 1) {
            rwljF64_Test test = tests[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            rwljString printf_string = rwlj_string(
                cast(char *) printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString rwlj_string =
                rwlj_string_from_fmt(&ftoa_slice, test.fmt, test.value);

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    rwlj_string,
                    rwlj_string.len
                );
                success = false;
            }
        }
    }

    TEST("Formats floats with a")
    {
        rwljF64_Test tests[] = { F64_TEST_TABLE(a) };

        u8 ftoa_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 ftoa_slice = rwlj_slice_from_buf(ftoa_buf);

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = rwlj_slice_from_buf(printf_buf);

        for (isize i = 0; i < rwlj_count_of(tests); i += 1) {
            rwljF64_Test test = tests[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.fmt,
                test.value
            );
            rwljString printf_string = rwlj_string(
                cast(char *) printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString rwlj_string =
                rwlj_string_from_fmt(&ftoa_slice, test.fmt, test.value);

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    rwlj_string,
                    rwlj_string.len
                );
                success = false;
            }
        }
    }
#undef F64_TEST_TABLE

    TEST("Formats floats")
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
        rwljSlice_U8 ftoa_slice = rwlj_slice_from_buf(ftoa_buf);

        u8 printf_buf[rwlj_kb(1)] = { 0 };
        rwljSlice_U8 printf_slice = rwlj_slice_from_buf(printf_buf);

        for (isize i = 0; i < rwlj_count_of(tests_single); i += 1) {
            rwljF64_Single_Test test = tests_single[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(i32) printf_slice.len,
                test.printf_fmt,
                test.value
            );
            rwljString printf_string = rwlj_string(
                cast(char *) printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString rwlj_string =
                rwlj_string_from_fmt(&ftoa_slice, test.printf_fmt, test.value);

            if (!rwlj_string_are_equal(rwlj_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    rwlj_string,
                    rwlj_string.len
                );
                success = false;
            }
        }

        for (isize i = 0; i < rwlj_count_of(tests_double); i += 1) {
            rwljF64_Double_Test test = tests_double[i];

            stbsp_snprintf(
                cast(char *) printf_slice.data,
                cast(usize) printf_slice.len,
                test.printf_fmt,
                test.value[0],
                test.value[1]
            );
            rwljString printf_string = rwlj_string(
                cast(char *) printf_slice.data,
                0,
                cast(isize) strlen(cast(char *) printf_slice.data)
            );

            rwljString ftoa_string = rwlj_string_from_fmt(
                &ftoa_slice, test.printf_fmt, test.value[0], test.value[1]
            );

            if (!rwlj_string_are_equal(ftoa_string, printf_string)) {
                rwlj_printfln(
                    "%ld - expected { %S, %ld }, got { %S, %ld }",
                    i,
                    printf_string,
                    printf_string.len,
                    ftoa_string,
                    ftoa_string.len
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

    TEST("Creates string builder")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        isize capacity = 256;
        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, capacity);

        rwlj_testing_expect(sb.buf != NULL);
        rwlj_testing_expect_value(sb.len, 0);
        rwlj_testing_expect_value(sb.capacity, capacity);
    }

    TEST("Writes i64")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, 256);

        rwljString s = rwlj_string_builder_write_i64(&sb, -123456);
        rwljString expected = STRING("-123456");

        rwlj_testing_expect_value(s.len, expected.len);
        rwlj_testing_expect_value_string(s, expected);
    }

    TEST("Writes u64")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, 256);

        rwljString s = rwlj_string_builder_write_u64(&sb, 123456, 'u');
        rwljString expected = STRING("123456");

        rwlj_testing_expect_value(s.len, expected.len);
        rwlj_testing_expect_value_string(s, expected);
    }

    TEST("Writes f64")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, 256);

        rwljString expected = STRING("3.14159");
        rwljString s =
            rwlj_string_builder_write_f64(&sb, 3.14159, 'f', expected.len - 2);

        rwlj_testing_expect_value(s.len, expected.len);
        rwlj_testing_expect_value_string(s, expected);
    }

    TEST("Writes string")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, 256);

        rwljString expected = STRING("Hello, World!\n");
        rwljString s = rwlj_string_builder_write_string(&sb, expected);

        rwlj_testing_expect_value(s.len, expected.len);
        rwlj_testing_expect_value_string(s, expected);
    }

#define TEST_STRING                                                            \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"       \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"       \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"       \
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

    TEST("Doesnt' write past buffer")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_count_of(buf));

        rwljString longer_s = STRING(TEST_STRING TEST_STRING);
        rwljString expected = STRING(TEST_STRING);

        rwljString_Builder sb = { 0 };
        rwlj_string_builder_init(&sb, &arena, expected.len);

        rwljString s = rwlj_string_builder_write_string(&sb, longer_s);

        rwlj_testing_expect_value(s.len, expected.len);
        rwlj_testing_expect_value_string(s, expected);
    }
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
    for (isize i = 0; i < rwlj_count_of(test_arr); i += 1) {
        (test_arr[i])();
        rwlj_println(STRING(""));
    }
}
