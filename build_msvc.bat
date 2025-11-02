@echo off
REM Build script for Kerr Black Hole Visualizer - Visual Studio
REM Requires: Visual Studio 2017+, SDL2, GLEW

echo ========================================
echo Kerr Black Hole Visualizer - MSVC Build
echo ========================================
echo.

REM Check if main.cpp exists
if not exist main.cpp (
    echo ERROR: main.cpp not found in current directory
    pause
    exit /b 1
)

REM Set library paths (modify these to match your installation)
set SDL2_DIR=C:\SDL2
set GLEW_DIR=C:\glew

REM Check if cl.exe is available
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: cl.exe not found in PATH
    echo Please run this script from Visual Studio Developer Command Prompt
    echo Or run: "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    pause
    exit /b 1
)

echo [1/3] Checking compiler...
echo OK: MSVC compiler found

echo.
echo [2/3] Compiling with optimizations...
echo Include paths:
echo   SDL2: %SDL2_DIR%\include
echo   GLEW: %GLEW_DIR%\include
echo.

cl.exe main.cpp /EHsc /std:c++17 /O2 /Fe:KerrBlackHole.exe ^
    /I"%SDL2_DIR%\include" ^
    /I"%GLEW_DIR%\include" ^
    /link ^
    /LIBPATH:"%SDL2_DIR%\lib\x64" ^
    /LIBPATH:"%GLEW_DIR%\lib\Release\x64" ^
    SDL2.lib SDL2main.lib glew32.lib opengl32.lib ^
    /SUBSYSTEM:CONSOLE

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    echo.
    echo Troubleshooting:
    echo 1. Download SDL2 development libraries:
    echo    https://www.libsdl.org/download-2.0.php
    echo    Extract to: %SDL2_DIR%
    echo.
    echo 2. Download GLEW:
    echo    http://glew.sourceforge.net/
    echo    Extract to: %GLEW_DIR%
    echo.
    echo 3. Verify paths in this script match your installation
    echo.
    echo 4. Run from Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

REM Clean up intermediate files
if exist main.obj del main.obj

echo.
echo [3/3] Build successful!
echo.
echo Output: KerrBlackHole.exe
echo.
echo Required files in same directory:
echo   - shader.vert
echo   - shader.frag
echo   - blackhole.comp
echo   - SDL2.dll
echo   - glew32.dll
echo.
echo Copy DLLs from:
echo   %SDL2_DIR%\lib\x64\SDL2.dll
echo   %GLEW_DIR%\bin\Release\x64\glew32.dll
echo.
echo To run: KerrBlackHole.exe
echo ========================================
pause
