#include <assert.h>
#include <stdio.h>

#define RWLJ_IMPLEMENTATION
#include "rwlj.h"

typedef void (*rwljTest_Proc)(void);

void
test_arena(void)
{
    DESCRIBE("rwljArena tests");

    IT("Sets growing arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(8);
        rwlj_arena_init_growing_size(&arena, arena_total_size);

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_GROWING);
        rwlj_testing_expect_value(arena.total_size, arena_total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Sets static arena")
    {
        rwljArena arena = { 0 };
        usize arena_total_size = rwlj_mb(1);
        rwlj_arena_init_static_size(&arena, arena_total_size);

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

    typedef struct rwljF64_Test {
        i64 precision;
        f64 f;
        rwljString expected;
    } rwljF64_Test;

    IT("Formats NaN")
    {
        u8 buf[32] = { 0 };

        rwljSlice_U8 slice = { buf, rwlj_size_of_buf(buf) };
        rwljString result = { .data = cast(char *) slice.data,
                              .len = rwlj_format_f64(slice, RWLJ_NAN, 'g', 0) };

        rwljString expected = STR_LIT("NaN");
        rwlj_testing_expect_value(result, expected);
    }

    IT("Formats Infinity")
    {
        {
            u8 buf[32] = { 0 };

            rwljSlice_U8 slice = { buf, rwlj_size_of_buf(buf) };
            rwljString result = {
                .data = cast(char *) slice.data,
                .len = rwlj_format_f64(slice, RWLJ_INFINITY, 'g', 0)
            };

            rwljString expected = STR_LIT("+inf");
            rwlj_testing_expect_value(result, expected);
        }
        {
            u8 buf[32] = { 0 };

            rwljSlice_U8 slice = { buf, rwlj_size_of_buf(buf) };
            rwljString result = {
                .data = cast(char *) slice.data,
                .len = rwlj_format_f64(slice, -RWLJ_INFINITY, 'g', 0)
            };

            rwljString expected = STR_LIT("-inf");
            rwlj_testing_expect_value(result, expected);
        }
    }

    IT("Formats floats using 'e' specifier")
    {
        // clang-format off
        rwljF64_Test tests[] = {
            {0,  rwlj_ldexp(8511030020275656, -342),  STR_LIT("1.e-87")},
            {1,  rwlj_ldexp(5201988407066741, -824),  STR_LIT("5.e-233")},
            {2,  rwlj_ldexp(6406892948269899, +237),  STR_LIT("1.4e+87")},
            {3,  rwlj_ldexp(8431154198732492, +72),   STR_LIT("3.98e+37")},
            {4,  rwlj_ldexp(6475049196144587, +99),   STR_LIT("4.104e+45")},
            {5,  rwlj_ldexp(8274307542972842, +726),  STR_LIT("2.9208e+234")},
            {6,  rwlj_ldexp(5381065484265332, -456),  STR_LIT("2.89195e-122")},
            {7,  rwlj_ldexp(6761728585499734, -1057), STR_LIT("4.378772e-303")},
            {8,  rwlj_ldexp(7976538478610756, +376),  STR_LIT("1.2277016e+129")},
            {9,  rwlj_ldexp(5982403858958067, +377),  STR_LIT("1.84155245e+129")},
            {10, rwlj_ldexp(5536995190630837, +93),   STR_LIT("5.483574435e+43")},
            {11, rwlj_ldexp(7225450889282194, +710),  STR_LIT("3.8919018115e+229")},
            {12, rwlj_ldexp(7225450889282194, +709),  STR_LIT("1.94595090573e+229")},
            {13, rwlj_ldexp(8703372741147379, +117),  STR_LIT("1.446095838161e+51")},
            {14, rwlj_ldexp(8944262675275217, -1001), STR_LIT("4.1736774745853e-286")},
            {15, rwlj_ldexp(7459803696087692, -707),  STR_LIT("1.10795077287889e-197")},
            {16, rwlj_ldexp(6080469016670379, -381),  STR_LIT("1.234550136632744e-99")},
            {17, rwlj_ldexp(8385515147034757, +721),  STR_LIT("9.2503171196036502e+232")},
            {18, rwlj_ldexp(7514216811389786, -828),  STR_LIT("4.19804715028488984e-234")},
            {19, rwlj_ldexp(8397297803260511, -345),  STR_LIT("1.171631531978651105e-88")},
            {20, rwlj_ldexp(6733459239310543, +202),  STR_LIT("4.3281007284461249363e+76")},
            {21, rwlj_ldexp(8091450587292794, -473),  STR_LIT("3.31771011816003108152e-127")},
            {0,  rwlj_ldexp(6567258882077402, +952),  STR_LIT("2.e+302")},
            {1,  rwlj_ldexp(6712731423444934, +535),  STR_LIT("8.e+176")},
            {2,  rwlj_ldexp(6712731423444934, +534),  STR_LIT("3.8e+176")},
            {3,  rwlj_ldexp(5298405411573037, -957),  STR_LIT("4.35e-273")},
            {4,  rwlj_ldexp(5137311167659507, -144),  STR_LIT("2.304e-28")},
            {5,  rwlj_ldexp(6722280709661868, +363),  STR_LIT("1.2630e+125")},
            {6,  rwlj_ldexp(5344436398034927, -169),  STR_LIT("7.14221e-36")},
            {7,  rwlj_ldexp(8369123604277281, -853),  STR_LIT("1.393457e-241")},
            {8,  rwlj_ldexp(8995822108487663, -780),  STR_LIT("1.4146345e-219")},
            {9,  rwlj_ldexp(8942832835564782, -383),  STR_LIT("4.53927792e-100")},
            {10, rwlj_ldexp(8942832835564782, -384),  STR_LIT("2.269638960e-100")},
            {11, rwlj_ldexp(8942832835564782, -385),  STR_LIT("1.1348194799e-100")},
            {12, rwlj_ldexp(6965949469487146, -249),  STR_LIT("7.70036656189e-60")},
            {13, rwlj_ldexp(6965949469487146, -250),  STR_LIT("3.850183280945e-60")},
            {14, rwlj_ldexp(6965949469487146, -251),  STR_LIT("1.9250916404724e-60")},
            {15, rwlj_ldexp(7487252720986826, +548),  STR_LIT("6.89858653177420e+180")},
            {16, rwlj_ldexp(5592117679628511, +164),  STR_LIT("1.307662263187865e+65")},
            {17, rwlj_ldexp(8887055249355788, +665),  STR_LIT("1.3605202075612124e+216")},
            {18, rwlj_ldexp(6994187472632449, +690),  STR_LIT("3.59281021747595968e+223")},
            {19, rwlj_ldexp(8797576579012143, +588),  STR_LIT("8.912519771248455190e+192")},
            {20, rwlj_ldexp(7363326733505337, +272),  STR_LIT("5.5876975736230114095e+97")},
            {21, rwlj_ldexp(8549497411294502, -448),  STR_LIT("1.17625783072854037999e-119")},
            {3,             12345000,                 STR_LIT("1.23e+07")},
        };
        // clang-format on

        u8 buf[rwlj_kb(4)] = { 0 };
        rwljSlice_U8 slice = { .data = buf, .len = rwlj_size_of_buf(buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljString formatted_float = {
                .data = cast(char *) slice.data,
                .len =
                    rwlj_format_f64(slice, tests[i].f, 'e', tests[i].precision)
            };

            rwlj_testing_expect_value(formatted_float, tests[i].expected);
        }
    }

    IT("Formats floats using 'f' specifier")
    {
        rwljF64_Test tests[] = {
            { 0, +0.0, STR_LIT("0") },
            { 1, -0.0, STR_LIT("-0.0") },
            { 1, 1.14223, STR_LIT("1.1") },
            { 2, 3.14159, STR_LIT("3.14") },
            { 3, 43289423.3123, STR_LIT("43289423.312") },
            { 4, 543.09673859, STR_LIT("543.0967") },
            { 5, 0.8888888888, STR_LIT("0.88889") },
            { 6, 10.98777899, STR_LIT("10.987779") },
            { 7, 453244984239384.10, STR_LIT("453244984239384.1000000") },
            { 8, 42.123, STR_LIT("42.12300000") },
            { 9, 100000.000, STR_LIT("100000.000000000") },
            { 10, 666.666666666666666, STR_LIT("666.6666666667") },
            { 11, 69.696767420420, STR_LIT("69.69676742042") },
            { 12, 0, STR_LIT("0.000000000000") },
        };

        u8 buf[rwlj_kb(4)] = { 0 };
        rwljSlice_U8 slice = { .data = buf, .len = rwlj_size_of_buf(buf) };

        for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
            rwljString formatted_float = {
                .data = cast(char *) slice.data,
                .len =
                    rwlj_format_f64(slice, tests[i].f, 'f', tests[i].precision)
            };

            rwlj_testing_expect_value(formatted_float, tests[i].expected);
        }
    }

    IT("Formats floats using 'g' specifier") {}
}

// clang-format off
rwljTest_Proc test_arr[] = {
    test_arena,
    test_slice,
    test_array,
    test_formatting,
};
// clang-format on

int
main(void)
{
    for (isize i = 0; i < rwlj_size_of_array(test_arr); i += 1) {
        (test_arr[i])();
        rwlj_printf("\n");
    }
}
