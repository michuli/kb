@echo off
REM Build script for Kerr Black Hole Visualizer - MinGW/MSYS2
REM Requires: MinGW-w64, SDL2, GLEW

echo ========================================
echo Kerr Black Hole Visualizer - MinGW Build
echo ========================================
echo.

REM Check if main.cpp exists
if not exist main.cpp (
    echo ERROR: main.cpp not found in current directory
    pause
    exit /b 1
)

REM Check if shader files exist
if not exist shader.vert (
    echo ERROR: shader.vert not found
    pause
    exit /b 1
)

if not exist shader.frag (
    echo ERROR: shader.frag not found
    pause
    exit /b 1
)

if not exist blackhole.comp (
    echo ERROR: blackhole.comp not found
    pause
    exit /b 1
)

echo [1/3] Checking compiler...
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: g++ not found in PATH
    echo Please install MinGW-w64 or MSYS2
    pause
    exit /b 1
)
echo OK: g++ found

echo.
echo [2/3] Compiling with optimizations...
echo Command: g++ main.cpp -o KerrBlackHole.exe -std=c++17 -O3 -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm
echo.

g++ main.cpp -o KerrBlackHole.exe -std=c++17 -O3 ^
    -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    echo.
    echo Troubleshooting:
    echo - Ensure SDL2 and GLEW are installed via MSYS2:
    echo   pacman -S mingw-w64-x86_64-SDL2
    echo   pacman -S mingw-w64-x86_64-glew
    echo.
    echo - Or download manually and add to PATH:
    echo   SDL2: https://www.libsdl.org/download-2.0.php
    echo   GLEW: http://glew.sourceforge.net/
    pause
    exit /b 1
)

echo.
echo [3/3] Build successful!
echo.
echo Output: KerrBlackHole.exe
echo.
echo Required files in same directory:
echo   - shader.vert
echo   - shader.frag  
echo   - blackhole.comp
echo   - SDL2.dll (if not in PATH)
echo   - glew32.dll (if not in PATH)
echo.
echo To run: KerrBlackHole.exe
echo ========================================
pause
