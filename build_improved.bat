@echo off
REM Build script for IMPROVED Kerr Black Hole Visualizer v2.0

echo ========================================
echo Kerr Black Hole v2.0 - Enhanced Build
echo ========================================
echo.

REM Check files
if not exist main_improved.cpp (
    echo ERROR: main_improved.cpp not found
    pause
    exit /b 1
)

if not exist blackhole_improved.comp (
    echo ERROR: blackhole_improved.comp not found
    pause
    exit /b 1
)

echo [1/2] Compiling improved version...
echo.

g++ main_improved.cpp -o KerrBlackHole_v2.exe -std=c++17 -O3 ^
    -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    echo Try: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew
    pause
    exit /b 1
)

echo.
echo [2/2] Build successful!
echo.
echo Output: KerrBlackHole_v2.exe
echo.
echo NEW FEATURES:
echo  - RK5 Cash-Karp integration (higher accuracy)
echo  - Multiple ray bounces (up to 5)
echo  - Bloom post-processing
echo  - Enhanced disk physics
echo  - Better starfield with galaxies
echo  - Chromatic aberration
echo  - Improved performance
echo.
echo CONTROLS:
echo  H - Show full help
echo  1/2 - Adjust ray bounces
echo  3/4 - Adjust bloom strength
echo  B - Toggle bloom on/off
echo  R - Reset to defaults
echo.
echo To run: KerrBlackHole_v2.exe
echo ========================================
pause
