@echo off
title Mini C++ Compiler - Test Suite Runner
color 0A
chcp 65001 >nul

cls
echo =============================================
echo        Running Mini C++ Compiler Test Suite
echo =============================================
echo.
setlocal enabledelayedexpansion
set count=0

for %%f in (tests\*.txt) do (
    echo ---------------------------------------------
    echo [TEST %%f]
    echo.
    mini_compiler.exe %%f
    echo.
    set /a count+=1
)

echo =============================================
echo ✅  All tests completed! Total tests run: %count%
echo =============================================
echo.
pause
