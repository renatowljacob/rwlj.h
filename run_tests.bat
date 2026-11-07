@echo off
setlocal EnableDelayedExpansion
cd /D "%~dp0"

set default_flags=/Od /Ob1 /std:c11 /Z7 /nologo /FC /experimental:c11atomics
set analyzer_flags=%default_flags% /analyze /c
set sanitizer_flags=%default_flags% /fsanitize=address /WX

set default_file=main
set output_file=main.exe
set files=tests.c

if /I "%~1"=="clean" (
    call :cleanup
    exit /B 0
)

python .\test_gen || exit /b 1

if "%~1"=="analyzer" (
    cl %analyzer_flags% %files% || exit /b 1
    echo No analyzer errors reported
    call :cleanup
    exit /B 0
) else if "%~1"=="sanitizer" (
    cl %sanitizer_flags% /Fe:%output_file% %files% || exit /b 1
    .\%output_file%
    set test_result=%ERRORLEVEL%
    call :cleanup
    exit /B !test_result!
) else (
    cl %default_flags% /Fe:%output_file% %files% || exit /b 1
    .\%output_file%
    set test_result=%ERRORLEVEL%
    call :cleanup
    exit /B !test_result!
)

:cleanup
del /Q /F tests.obj tests.ilk tests.nativecodeanalysis.xml >nul 2>&1
del /Q /F main.obj main.ilk main.nativecodeanalysis.xml >nul 2>&1
del /Q /F "rwlj_file_tests.tmp*" >nul 2>&1
for /D %%D in ("rwlj_file_tests.tmp*") do rmdir /S /Q "%%~fD"
exit /B 0
