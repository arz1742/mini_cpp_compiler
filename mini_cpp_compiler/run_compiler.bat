@echo off
title Mini C++ Compiler - Interactive Run
color 0A
chcp 65001 >nul

:menu
cls
echo =============================================
echo          Mini C++ Compiler - Single Run
echo =============================================
echo.
echo Available test files:
echo ---------------------------------------------
setlocal enabledelayedexpansion
set i=0
for %%f in (tests\*.txt) do (
    set /a i+=1
    echo [!i!] %%f
    set file!i!=%%f
)
echo ---------------------------------------------
echo [0] Exit
echo.
set /p choice=Enter test number (or type file path manually): 

if "%choice%"=="0" exit
if defined file%choice% (
    set filename=!file%choice%!
) else (
    set filename=%choice%
)
echo.
echo Running: %filename%
echo ---------------------------------------------
echo.
mini_compiler.exe %filename%
echo.
pause
goto menu
