#!/bin/sh
# shellcheck disable=SC2086

default_args="-std=gnu11 -Wall -Wextra -Wformat-overflow -Wuse-after-free=1 \
        -Wstrict-prototypes -Wconversion -Wno-override-init -O0 -g3"
analyzer_args="-fanalyzer "
sanitizer_args="-fsanitize=address,undefined -Werror -fmax-errors=1 "
output_file="main"
tests_file="tests.c"

if ! ./test_gen; then
    exit 1
fi
case ${1} in
analyzer)
        gcc ${default_args} ${analyzer_args} \
            -o /dev/null rwlj.h ${tests_file} \
        && printf '%b\n' "No analyzer errors reported"
    ;;
no-analyzer)
        gcc ${default_args} ${sanitizer_args} \
            -o ${output_file} rwlj.h ${tests_file} \
        && ./main
    ;;
*)
        gcc ${default_args} ${analyzer_args} \
            -o /dev/null rwlj.h ${tests_file} \
        && gcc ${default_args} ${sanitizer_args} \
            -o ${output_file} rwlj.h ${tests_file} \
        && ./main
    ;;
esac
