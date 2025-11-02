# Kerr Black Hole Gravitational Lensing Visualizer

A physically accurate, real-time visualization of a rotating (Kerr) black hole with gravitational lensing effects, inspired by *Thorne et al. (2015)* and the movie *Interstellar*.

![Kerr Black Hole](https://img.shields.io/badge/Physics-General_Relativity-blue) ![OpenGL](https://img.shields.io/badge/OpenGL-4.5-green) ![C++](https://img.shields.io/badge/C%2B%2B-17-orange)

---

## 🌌 Features

### Physics Implementation
- **Kerr Metric**: Full implementation in Boyer-Lindquist coordinates with spin parameter `a ∈ [0, 0.998]`
- **Null Geodesic Integration**: 4th-order Runge-Kutta (RK4) method for backward ray tracing
- **Frame Dragging**: Rotational effects on spacetime around spinning black hole
- **Event Horizon**: Dynamic horizon radius: `r_h = M + √(M² - a²)`
- **Photon Sphere**: Natural emergence at `r ≈ 1.5M` creating bright ring
- **Gravitational Redshift**: Full relativistic Doppler and gravitational frequency shift
- **Doppler Beaming**: Intensity boosting formula `I ∝ g³` where `g = ν_obs/ν_emit`

### Visual Effects
- **Accretion Disk**: Shakura-Sunyaev inspired disk with temperature gradient
  - Inner radius (ISCO): 3M
  - Outer radius: 12M
  - Color gradient: blue (hot inner) → orange (cool outer)
- **Self-Lensing**: Secondary images from strongly bent light paths
- **Asymmetric Lensing**: "Interstellar-style" disk arcs above and below black hole
- **Starfield**: Procedurally generated background stars warped by gravity
- **Temporal Dynamics**: Rotating disk with visible turbulence and spiral patterns
- **Cinematic Rendering**: ACES tone mapping with gamma correction

### Rendering Technology
- **OpenGL Compute Shader**: GPU-accelerated ray tracing (16×16 work groups)
- **Adaptive Step Size**: Smaller steps near photon sphere for accuracy
- **Real-time Performance**: Optimized for 60 FPS @ 1920×1080

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| **ESC** | Quit application |
| **SPACE** | Pause/resume animation |
| **↑ / ↓** | Increase/decrease exposure |
| **A / D** | Decrease/increase spin parameter `a` |
| **W / S** | Increase/decrease observer inclination |
| **Q / E** | Decrease/increase camera distance |

---

## 🔧 Compilation

### Prerequisites

**Windows (MinGW-w64):**
- MinGW-w64 compiler (GCC 7.3+)
- SDL2 development libraries
- GLEW library
- OpenGL 4.5 capable GPU

**Windows (Visual Studio):**
- Visual Studio 2017+ with C++17 support
- SDL2, GLEW libraries

### Installation of Dependencies

#### Option 1: MSYS2/MinGW (Recommended for Windows)
```bash
# Install MSYS2 from https://www.msys2.org/
# Then in MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-SDL2
pacman -S mingw-w64-x86_64-glew
```

#### Option 2: Manual Installation
1. Download SDL2 development libraries: https://www.libsdl.org/download-2.0.php
   - Choose "SDL2-devel-2.X.X-mingw.tar.gz" for MinGW
   - Extract to `C:\SDL2`

2. Download GLEW: http://glew.sourceforge.net/
   - Choose Windows binaries
   - Extract to `C:\glew`

3. Add to PATH or specify paths in compilation command

### Building the Application

#### MinGW Compilation:
```bash
# If libraries are in system PATH:
g++ main.cpp -o KerrBlackHole.exe -std=c++17 -O3 \
    -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm

# With explicit library paths:
g++ main.cpp -o KerrBlackHole.exe -std=c++17 -O3 \
    -IC:/SDL2/include -IC:/glew/include \
    -LC:/SDL2/lib -LC:/glew/lib \
    -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lgdi32 -lm
```

#### Visual Studio (Command Line):
```bash
cl.exe main.cpp /EHsc /std:c++17 /O2 /Fe:KerrBlackHole.exe \
    /I"C:\SDL2\include" /I"C:\glew\include" \
    /link /LIBPATH:"C:\SDL2\lib\x64" /LIBPATH:"C:\glew\lib\Release\x64" \
    SDL2.lib SDL2main.lib glew32.lib opengl32.lib /SUBSYSTEM:CONSOLE
```

#### Using Provided Build Script:
```bash
# For MinGW:
build_mingw.bat

# For Visual Studio:
build_msvc.bat
```

### Running the Application

```bash
# Make sure shader files are in the same directory as executable:
# - shader.vert
# - shader.frag
# - blackhole.comp

./KerrBlackHole.exe
```

---

## 📐 Physics Background

### Kerr Metric (Boyer-Lindquist Coordinates)

The line element for a rotating black hole:

```
ds² = -(1 - 2Mr/Σ) dt² - (4Mar sin²θ/Σ) dt dφ 
      + (Σ/Δ) dr² + Σ dθ² 
      + [(r² + a²)² - a²Δ sin²θ] sin²θ/Σ dφ²
```

Where:
- `Σ = r² + a² cos²θ`
- `Δ = r² - 2Mr + a²`
- `M` = black hole mass (set to 1 in geometric units)
- `a` = spin parameter (0 = Schwarzschild, 1 = extremal Kerr)

### Geodesic Equation

Photon paths follow null geodesics (`ds² = 0`):

```
d²x^μ/dλ² = -Γ^μ_νρ (dx^ν/dλ)(dx^ρ/dλ)
```

Integrated using 4th-order Runge-Kutta with adaptive step size.

### Doppler & Gravitational Redshift

The observed frequency shift:

```
g = ν_obs/ν_emit = (k_μ u^μ_obs) / (k_μ u^μ_emit)
```

Where:
- `k_μ` = photon 4-momentum
- `u^μ_obs` = observer 4-velocity (at infinity)
- `u^μ_emit` = emitter 4-velocity (disk material in Keplerian orbit)

Intensity boosting: `I_obs = g³ I_emit`

### Accretion Disk Model

- **Innermost Stable Circular Orbit (ISCO)**: `r_ISCO ≈ 3M` (prograde)
- **Temperature profile**: `T ∝ r^(-3/4)` (thin disk approximation)
- **Emission**: `I ∝ r^(-3)` (simplified Shakura-Sunyaev)
- **Rotation**: Keplerian angular velocity `Ω = √(M/r³)`

---

## 🎨 Expected Visual Output

When running, you should see:

1. **Black Shadow**: Central dark region (event horizon)
2. **Photon Ring**: Bright, thin ring at `r ≈ 1.5M` from strong lensing
3. **Asymmetric Disk**: 
   - Primary disk in equatorial plane
   - Secondary lensed arc above black hole
   - Tertiary lensed arc below black hole
4. **Doppler Shift**:
   - Brighter approaching side (blue-shifted)
   - Dimmer receding side (red-shifted)
5. **Warped Starfield**: Background stars bent around black hole
6. **Frame Dragging**: Visible asymmetry due to black hole rotation

### Parameter Effects

- **Low spin (a ≈ 0)**: Symmetric Schwarzschild-like appearance
- **High spin (a ≈ 0.9)**: Strong asymmetry, pronounced frame dragging
- **High inclination (85°)**: Edge-on view shows dramatic lensing arcs
- **Low inclination (10°)**: Face-on view shows circular disk structure

---

## 🚀 Performance Optimization

- **Adaptive Integration**: Step size decreases near photon sphere
- **Early Termination**: Ray tracing stops at horizon or escape
- **Float Precision**: Single precision with periodic renormalization
- **Local Memory**: Performance-critical data kept in compute shader locals
- **Work Group Size**: 16×16 threads optimized for modern GPUs

Target: **60 FPS @ 1920×1080** on NVIDIA GTX 1060 or equivalent

---

## 📚 References

1. **Thorne, K. S. et al.** (2015). "Gravitational Lensing by Spinning Black Holes in Astrophysics, and in the Movie Interstellar". *Classical and Quantum Gravity*, 32(6).

2. **Kerr, R. P.** (1963). "Gravitational Field of a Spinning Mass as an Example of Algebraically Special Metrics". *Physical Review Letters*, 11(5).

3. **Shakura, N. I. & Sunyaev, R. A.** (1973). "Black Holes in Binary Systems. Observational Appearance". *Astronomy and Astrophysics*, 24.

4. **James, O. et al.** (2015). "Visualizing Interstellar's Wormhole". *American Journal of Physics*, 83(6).

5. **Chandrasekhar, S.** (1983). *The Mathematical Theory of Black Holes*. Oxford University Press.

---

## 🔬 Extensions & Future Work

- **Kerr-Newman Metric**: Add electric charge parameter
- **Polarization**: Simulate polarized emission from disk
- **Spectral Mapping**: Multi-wavelength rendering (X-ray, optical, radio)
- **Temporal Denoising**: Path tracing accumulation for smoother visuals
- **Volumetric Corona**: Optically thin hot gas near horizon
- **Interactive VR**: Stereoscopic rendering for VR headsets
- **Newman-Penrose Formalism**: Alternative coordinate system for numerics

---

## 📄 License

This project is released under the **MIT License**.

```
Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
```

---

## 🙏 Acknowledgments

- Inspired by the visual effects work in Christopher Nolan's *Interstellar*
- Physics implementation based on Kip Thorne's research
- OpenGL community for excellent documentation and resources

---

## 🐛 Troubleshooting

### "Failed to load shader files"
- Ensure `shader.vert`, `shader.frag`, and `blackhole.comp` are in the same directory as the executable

### Black screen / No output
- Check GPU supports OpenGL 4.5+ with compute shaders
- Update graphics drivers
- Try reducing resolution in `main.cpp` (WINDOW_WIDTH/HEIGHT)

### Poor performance
- Lower resolution
- Reduce MAX_STEPS in `blackhole.comp` (default: 512)
- Increase initial step size `dlambda`

### Compilation errors
- Verify SDL2 and GLEW paths are correct
- Check compiler supports C++17
- Ensure all library files (.lib/.a) are for correct architecture (x86/x64)

---

**Enjoy exploring the warped spacetime around a rotating black hole!** 🌌✨
