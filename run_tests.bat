@echo off
setlocal
cd /D "%~dp0"

set default_flags=/Od /Ob1 /std:c11 /W4 /Z7 /nologo /FC
set analyzer_flags=%default_flags% /analyze /c
set sanitizer_flags=%default_flags% /fsanitize=address /WX

set default_file=main
set output_file=main.exe
set files=tests.c rwlj.h

python .\test_gen || exit /b 1

if "%~1"=="analyzer" (
    cl %analyzer_flags% %files% || exit /b 1
    echo No analyzer errors reported
) else if "%~1"=="sanitizer" (
    cl %sanitizer_flags% /Fe:%output_file% %files% || exit /b 1
    \.main || exit /b 1
) else (
    cl %default_flags% /Fe:%output_file% %files% || exit /b 1
    \.main || exit /b 1
)
