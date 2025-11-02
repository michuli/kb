@echo off
REM Build script for Kerr Black Hole v3.0 - CINEMATIC INTERSTELLAR STYLE

echo ========================================
echo Kerr Black Hole v3.0 - CINEMATIC BUILD
echo ========================================
echo.

if not exist main_improved.cpp (
    echo ERROR: main_improved.cpp not found
    pause
    exit /b 1
)

if not exist blackhole_cinematic.comp (
    echo ERROR: blackhole_cinematic.comp not found
    pause
    exit /b 1
)

echo [1/2] Compiling CINEMATIC version...
echo Visual Style: Interstellar-inspired volumetric rendering
echo.

g++ main_improved.cpp -o KerrBlackHole_Cinematic.exe -std=c++17 -O3 ^
    -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed
    pause
    exit /b 1
)

echo.
echo [2/2] Build successful!
echo.
echo Output: KerrBlackHole_Cinematic.exe
echo.
echo CINEMATIC FEATURES v3.0:
echo ========================
echo  [√] Volumetric disk emission (layered glow)
echo  [√] Intense photon ring (4x brighter)
echo  [√] Motion blur starfield (dynamic streaks)
echo  [√] Rich color palette (orange-white-blue)
echo  [√] Atmospheric haze and scattering
echo  [√] Subsurface scattering in disk
echo  [√] Enhanced Doppler beaming (g^3.5)
echo  [√] Multiple volumetric samples
echo  [√] Cinematic tone mapping
echo  [√] Color grading (cool darks, warm highlights)
echo.
echo VISUAL QUALITY:
echo  - Matches Interstellar aesthetic
echo  - Brilliant white-blue photon ring
echo  - Glowing orange-white accretion disk
echo  - Volumetric depth and layering
echo  - Dynamic motion effects
echo.
echo REQUIRED FILES:
echo  - blackhole_cinematic.comp (must be in same folder!)
echo  - shader.vert
echo  - shader_improved.frag (or shader.frag)
echo.
echo To run: KerrBlackHole_Cinematic.exe
echo ========================================
pause
