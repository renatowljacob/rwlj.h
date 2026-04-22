#include <assert.h>

#include "rwlj.h"
#define RWLJ_IMPLEMENTATION

typedef void (*rwljTest_Proc)(void);

void
test_arena(void)
{
    DESCRIBE("Arena tests");

    IT("Sets growing arena")
    {
        rwljArena arena = { 0 };
        rwlj_arena_init_growing_size(&arena, rwlj_mb(8));

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_GROWING);
        rwlj_testing_expect_value(arena.total_size, rwlj_mb(8));

        rwlj_arena_destroy(&arena);
    }

    IT("Sets static arena")
    {
        rwljArena arena = { 0 };
        rwlj_arena_init_static_size(&arena, rwlj_mb(1));

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_STATIC);
        rwlj_testing_expect_value(arena.total_size, rwlj_mb(1));

        rwlj_arena_destroy(&arena);
    }

    IT("Sets buffer arena")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        rwlj_testing_expect(arena.backing_buf != NULL);
        rwlj_testing_expect_value(arena.kind, RWLJ_ARENA_BUFFER);
        rwlj_testing_expect_value(arena.total_size, rwlj_kb(1));

        rwlj_arena_destroy(&arena);
    }

    IT("Allocates all memory with arena")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        u8 *mem = rwlj_arena_alloc(&arena, arena.total_size);

        rwlj_testing_expect(mem != NULL);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Allocates no memory with arena")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        u8 *mem = rwlj_arena_alloc(&arena, 0);

        rwlj_testing_expect(mem == NULL);
        rwlj_testing_expect_value(arena.allocated_size, 0);

        rwlj_arena_destroy(&arena);
    }

    IT("Makes arbitrary allocations with arena")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        isize allocations = 8;
        isize allocation_size = rwlj_size_of_array(buf) / allocations;
        for (isize i = 0; i < allocations; i += 1) {
            u8 *mem = rwlj_arena_alloc(&arena, cast(usize) allocation_size);

            rwlj_testing_expect(mem != NULL);
            rwlj_testing_expect_value(
                arena.allocated_size, cast(usize)(allocation_size * (i + 1))
            );
        }

        rwlj_arena_destroy(&arena);
    }

    IT("Enlarges last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        usize allocation_size = arena.total_size / 4;

        u8 *_mem1 = rwlj_arena_alloc(&arena, allocation_size);
        rwlj_unused(_mem1);

        u8 *_mem2 = rwlj_arena_alloc(&arena, allocation_size);
        rwlj_unused(_mem2);

        u8 *mem3 = rwlj_arena_alloc(&arena, allocation_size);

        rwlj_testing_expect(mem3 != NULL);
        rwlj_testing_expect_value(arena.allocated_size, allocation_size * 3);

        u8 *resized_mem = rwlj_arena_resize(
            &arena, mem3, allocation_size, allocation_size * 2
        );

        rwlj_testing_expect(resized_mem != NULL);
        rwlj_testing_expect_value(arena.allocated_size, arena.total_size);

        rwlj_arena_destroy(&arena);
    }

    IT("Shrinks last arena allocation")
    {
        u8 buf[rwlj_kb(1)] = { 0 };
        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        usize allocation_size = arena.total_size / 4;
        usize total_allocated = 0;

        u8 *_mem1 = rwlj_arena_alloc(&arena, allocation_size);
        rwlj_unused(_mem1);
        total_allocated += allocation_size;

        u8 *_mem2 = rwlj_arena_alloc(&arena, allocation_size);
        rwlj_unused(_mem2);
        total_allocated += allocation_size;

        u8 *mem3 = rwlj_arena_alloc(&arena, allocation_size);
        total_allocated += allocation_size;

        rwlj_testing_expect(mem3 != NULL);
        rwlj_testing_expect_value(arena.allocated_size, total_allocated);

        u8 *resized_mem = rwlj_arena_resize(
            &arena, mem3, allocation_size, allocation_size / 2
        );
        total_allocated -= (allocation_size / 2);

        rwlj_testing_expect(resized_mem != NULL);
        rwlj_testing_expect_value(arena.allocated_size, total_allocated);

        rwlj_arena_destroy(&arena);
    }
}

void
test_slice(void)
{
    DESCRIBE("Slice tests");

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
    DESCRIBE("Dynamic Array tests");

    IT("Creates dynamic array")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

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
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(array.data != NULL);
        rwlj_testing_expect_value(array.kind, RWLJ_ARRAY_GROWING);
        rwlj_testing_expect_value(array.capacity, array_cap);

        rwlj_arena_destroy(&arena);
    }

    IT("Reserves at least 8 elements worth of memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

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
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect(rwlj_array_grow(&array));

        isize array_new_len = RWLJ_GROW_FORMULA(array_cap);
        rwlj_testing_expect_value(array.capacity, array_new_len);

        rwlj_arena_destroy(&arena);
    }

    IT("Grows dynamic array up to arena's total allocated memory")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        isize max_len = rwlj_size_of_array(buf) / rwlj_size_of(isize);
        rwlj_testing_expect(rwlj_array_resize(&array, max_len));
        rwlj_testing_expect_value(array.capacity, max_len);

        rwlj_arena_destroy(&arena);
    }

    // TODO: clean this up
    IT("Doesn't allocate more than arena's total size")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

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
    IT("Doesn't allocate more than arena's total size considering previous "
       "allocations")
    {
        u8 buf[rwlj_kb(1)] = { 0 };

        rwljArena arena = { 0 };
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

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
        rwlj_arena_init_from_buffer(&arena, buf, rwlj_size_of_array(buf));

        rwljArray_Isize array = { 0 };
        isize array_cap = 32;
        rwlj_array_init_dynamic_reserve(&array, array_cap, &arena);

        rwlj_testing_expect_value(array.capacity, array_cap);

        isize array_new_size = RWLJ_GROW_FORMULA(0);
        rwlj_testing_expect(rwlj_array_resize(&array, array_new_size));
        rwlj_testing_expect_value(array.capacity, array_new_size);

        rwlj_arena_destroy(&arena);
    }
}

// void
// test_formatting(void)
// {
//     char buf[1024] = { 0 };
//     rwljSlice_String buf_slice = { .data = buf,
//                                    .len = rwlj_size_of_array(buf) };
//     rwljString s = { .data = &buf_slice.data[0],
//                      .len = rwlj_bprintf(buf_slice, "Hello, %s", "World") };
//
//     rwlj_testing_expect_string(s, STR_LIT("Hello, World"));
// }

// void
// test_float_formatting(void)
// {
//     typedef struct rwljF64_Test {
//         i64 precision;
//         f64 f;
//         rwljString expected;
//     } rwljF64_Test;
//
//     // clang-format off
//     rwljF64_Test tests[] = {
//         {0,  rwlj_ldexp(8511030020275656, -342),  STR_LIT("9.e-88")},
//         {1,  rwlj_ldexp(5201988407066741, -824),  STR_LIT("4.6e-233")},
//         {2,  rwlj_ldexp(6406892948269899, +237),  STR_LIT("1.41e+87")},
//         {3,  rwlj_ldexp(8431154198732492, +72),   STR_LIT("3.981e+37")},
//         {4,  rwlj_ldexp(6475049196144587, +99),   STR_LIT("4.1040e+45")},
//         {5,  rwlj_ldexp(8274307542972842, +726),  STR_LIT("2.92084e+234")},
//         {6,  rwlj_ldexp(5381065484265332, -456),  STR_LIT("2.891946e-122")},
//         {7,  rwlj_ldexp(6761728585499734, -1057), STR_LIT("4.3787718e-303")},
//         {8,  rwlj_ldexp(7976538478610756, +376), STR_LIT("1.22770163e+129")},
//         {9,  rwlj_ldexp(5982403858958067, +377),
//         STR_LIT("1.841552452e+129")}, {10, rwlj_ldexp(5536995190630837, +93),
//         STR_LIT("5.4835744350e+43")}, {11, rwlj_ldexp(7225450889282194,
//         +710),  STR_LIT("3.89190181146e+229")}, {12,
//         rwlj_ldexp(7225450889282194, +709),  STR_LIT("1.945950905732e+229")},
//         {13, rwlj_ldexp(8703372741147379, +117),
//         STR_LIT("1.4460958381605e+51")}, {14, rwlj_ldexp(8944262675275217,
//         -1001), STR_LIT("4.17367747458531e-286")}, {15,
//         rwlj_ldexp(7459803696087692, -707),
//         STR_LIT("1.107950772878888e-197")}, {16, rwlj_ldexp(6080469016670379,
//         -381),  STR_LIT("1.2345501366327440e-99")}, {17,
//         rwlj_ldexp(8385515147034757, +721),
//         STR_LIT("9.25031711960365024e+232")}, {18,
//         rwlj_ldexp(7514216811389786, -828),
//         STR_LIT("4.198047150284889840e-234")}, {19,
//         rwlj_ldexp(8397297803260511, -345),
//         STR_LIT("1.1716315319786511046e-88")}, {20,
//         rwlj_ldexp(6733459239310543, +202),
//         STR_LIT("4.32810072844612493629e+76")}, {21,
//         rwlj_ldexp(8091450587292794, -473),
//         STR_LIT("3.317710118160031081518e-127")},
//
//         {0,  rwlj_ldexp(6567258882077402, +952), STR_LIT("3.e+302")},
//         {1,  rwlj_ldexp(6712731423444934, +535), STR_LIT("7.6e+176")},
//         {2,  rwlj_ldexp(6712731423444934, +534), STR_LIT("3.78e+176")},
//         {3,  rwlj_ldexp(5298405411573037, -957), STR_LIT("4.350e-273")},
//         {4,  rwlj_ldexp(5137311167659507, -144), STR_LIT("2.3037e-28")},
//         {5,  rwlj_ldexp(6722280709661868, +363), STR_LIT("1.26301e+125")},
//         {6,  rwlj_ldexp(5344436398034927, -169), STR_LIT("7.142211e-36")},
//         {7,  rwlj_ldexp(8369123604277281, -853), STR_LIT("1.3934574e-241")},
//         {8,  rwlj_ldexp(8995822108487663, -780), STR_LIT("1.41463449e-219")},
//         {9,  rwlj_ldexp(8942832835564782, -383),
//         STR_LIT("4.539277920e-100")}, {10, rwlj_ldexp(8942832835564782,
//         -384), STR_LIT("2.2696389598e-100")}, {11,
//         rwlj_ldexp(8942832835564782, -385), STR_LIT("1.13481947988e-100")},
//         {12, rwlj_ldexp(6965949469487146, -249),
//         STR_LIT("7.700366561890e-60")}, {13, rwlj_ldexp(6965949469487146,
//         -250), STR_LIT("3.8501832809448e-60")}, {14,
//         rwlj_ldexp(6965949469487146, -251), STR_LIT("1.92509164047238e-60")},
//         {15, rwlj_ldexp(7487252720986826, +548),
//         STR_LIT("6.898586531774201e+180")}, {16, rwlj_ldexp(5592117679628511,
//         +164), STR_LIT("1.3076622631878654e+65")}, {17,
//         rwlj_ldexp(8887055249355788, +665),
//         STR_LIT("1.36052020756121240e+216")}, {18,
//         rwlj_ldexp(6994187472632449, +690),
//         STR_LIT("3.592810217475959676e+223")}, {19,
//         rwlj_ldexp(8797576579012143, +588),
//         STR_LIT("8.9125197712484551899e+192")}, {20,
//         rwlj_ldexp(7363326733505337, +272),
//         STR_LIT("5.58769757362301140950e+97")}, {21,
//         rwlj_ldexp(8549497411294502, -448),
//         STR_LIT("1.176257830728540379990e-119")},
//
//         {3, 12345000, STR_LIT("1.234e+7")},
//     };
//     // clang-format on
//
//     u8 buf[1024] = { 0 };
//     rwljSlice_U8 buf_slice = { .data = buf, .len = rwlj_size_of_array(buf) };
//     for (isize i = 0; i < rwlj_size_of_array(tests); i += 1) {
//         isize len =
//             rwlj_format_f64(buf_slice, tests[i].f, 'f', tests[i].precision);
//         rwljString formatted_float = { .data = cast(char *) buf_slice.data,
//                                        .len = len };
//         if (!rwlj_string_are_equal(formatted_float, tests[i].expected)) {
//             printf(
//                 "%ld: expected %.*s, got %*s\n",
//                 i,
//                 cast(i32) tests[i].expected.len,
//                 tests[i].expected.data,
//                 cast(i32) formatted_float.len,
//                 formatted_float.data
//             );
//         }
//         rwlj_slice_clear(&buf_slice);
//     }
// }

rwljTest_Proc test_arr[] = { test_arena, test_slice, test_array };

int
main(void)
{
    for (isize i = 0; i < rwlj_size_of_array(test_arr); i += 1) {
        (test_arr[i])();
        rwlj_printf("\n");
    }
}
