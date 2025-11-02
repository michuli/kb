# Physics Implementation Notes - Kerr Black Hole Visualization

## Mathematical Foundation

### 1. Kerr Metric in Boyer-Lindquist Coordinates

The complete line element for a rotating black hole in geometric units (G = c = 1):

```
ds² = -(1 - 2Mr/Σ) dt² - (4Mar sin²θ/Σ) dt dφ 
      + (Σ/Δ) dr² + Σ dθ² 
      + [(r² + a²)² - a²Δ sin²θ] sin²θ/Σ dφ²
```

**Auxiliary functions:**
- Σ(r,θ) = r² + a² cos²θ
- Δ(r) = r² - 2Mr + a²
- A(r,θ) = (r² + a²)² - a² Δ sin²θ

**Key properties:**
- When a = 0: reduces to Schwarzschild metric
- When a → M: approaches extremal Kerr black hole
- Metric signature: (-,+,+,+)

### 2. Event Horizons

The event horizon occurs where g^rr → ∞, or equivalently where Δ = 0:

```
r± = M ± √(M² - a²)
```

- **Outer horizon**: r₊ = M + √(M² - a²)  [event horizon]
- **Inner horizon**: r₋ = M - √(M² - a²)  [Cauchy horizon]

For our simulation:
- a = 0.0 (Schwarzschild): r₊ = 2M
- a = 0.9M: r₊ ≈ 1.44M
- a = 0.998M: r₊ ≈ 1.06M

### 3. Ergosphere

The ergosphere is the region where:

```
r < r_ergo = M + √(M² - a² cos²θ)
```

Inside the ergosphere, all observers must rotate with the black hole (frame dragging).

### 4. Photon Sphere and Circular Orbits

For null geodesics (photons), circular orbits exist at:

**Prograde orbit** (co-rotating):
```
r_ph⁺ = 2M[1 + cos(2/3 · arccos(-a/M))]
```

**Retrograde orbit** (counter-rotating):
```
r_ph⁻ = 2M[1 + cos(2/3 · arccos(a/M))]
```

For Schwarzschild (a=0): r_ph = 3M

The photon sphere creates the bright "photon ring" visible in images.

### 5. Innermost Stable Circular Orbit (ISCO)

The ISCO determines the inner edge of the accretion disk:

**Prograde (equatorial, co-rotating):**
```
r_ISCO = M[3 + Z₂ - √((3-Z₁)(3+Z₁+2Z₂))]
```

where:
```
Z₁ = 1 + (1-a²/M²)^(1/3)[(1+a/M)^(1/3) + (1-a/M)^(1/3)]
Z₂ = √(3a²/M² + Z₁²)
```

Approximate values:
- a = 0.0: r_ISCO ≈ 6M
- a = 0.9M: r_ISCO ≈ 2.3M
- a = 0.998M: r_ISCO ≈ 1.45M

In our simulation, we use r_ISCO ≈ 3M for a = 0.9.

---

## Geodesic Equations

### Null Geodesics (Photon Paths)

For massless particles (photons), the geodesic equation is:

```
d²x^μ/dλ² + Γ^μ_νρ (dx^ν/dλ)(dx^ρ/dλ) = 0
```

where Γ^μ_νρ are the Christoffel symbols (connection coefficients).

### Christoffel Symbols

For the Kerr metric, the non-zero Christoffel symbols are (partial list):

```
Γ^t_tr = Mr(r²-a²cos²θ) / [Σ²(r²-2Mr+a²)]
Γ^t_tθ = -Ma²r sin(2θ) / (2Σ²)
Γ^r_tt = (r²-2Mr+a²)(r²-a²cos²θ) / Σ³
Γ^r_rr = (Mr-r²-a²cos²θ) / [Σ(r²-2Mr+a²)]
Γ^θ_θθ = -a²sin(2θ) / (2Σ)
Γ^φ_tφ = a·Γ^t_tr
... (36 non-zero components total)
```

### Conserved Quantities

Due to time and azimuthal symmetries:

1. **Energy** (per unit mass):
   ```
   E = -p_t = -(g_tt ṫ + g_tφ φ̇)
   ```

2. **Angular momentum** (per unit mass, z-component):
   ```
   L_z = p_φ = g_φt ṫ + g_φφ φ̇
   ```

3. **Carter constant** (generalization of Lz²):
   ```
   Q = p_θ² + cos²θ[a²(m² - E²) + L_z²/sin²θ]
   ```

These reduce the 8D phase space (4 position + 4 momentum) to effectively 2D.

---

## Numerical Integration (RK4)

### Method

We use the 4th-order Runge-Kutta method to integrate the geodesic equations:

```
State: y = (t, r, θ, φ, ṫ, ṙ, θ̇, φ̇)

k₁ = f(yₙ)
k₂ = f(yₙ + Δλ·k₁/2)
k₃ = f(yₙ + Δλ·k₂/2)
k₄ = f(yₙ + Δλ·k₃)

yₙ₊₁ = yₙ + Δλ/6 · (k₁ + 2k₂ + 2k₃ + k₄)
```

where f(y) computes the derivatives using geodesic equations.

### Adaptive Step Size

Near the photon sphere (r ≈ 1.5M), curvature is extreme. We use:

```cpp
float distToPhotonSphere = abs(r - r_photon);
dlambda = 0.02 + 0.08 * smoothstep(0.0, 2.0, distToPhotonSphere);
```

This gives:
- dlambda ≈ 0.02 near photon sphere (high accuracy)
- dlambda ≈ 0.10 far from black hole (faster computation)

### Null Geodesic Condition

Must maintain ds² = 0 throughout integration:

```
g_μν (dx^μ/dλ)(dx^ν/dλ) = 0
```

Small numerical errors accumulate, but renormalization can correct:

```cpp
float norm = sqrt(-g_tt·ṫ² - 2·g_tφ·ṫ·φ̇ + g_rr·ṙ² + g_θθ·θ̇² + g_φφ·φ̇²);
vel /= norm;  // Renormalize to null geodesic
```

---

## Relativistic Effects

### 1. Gravitational Redshift

Frequency shift due to gravitational time dilation:

```
ν_obs/ν_emit = √(-g_tt(emit)) / √(-g_tt(obs))
```

At infinity: g_tt → -1, so:
```
ν_obs/ν_emit = √(-g_tt(emit))
```

### 2. Doppler Shift (Special Relativity)

For moving source with velocity v:

```
ν_obs/ν_emit = γ(1 - v·n/c) / (1 - v_obs·n/c)
```

where n is the direction vector to observer.

### 3. Combined Redshift Factor

The full relativistic calculation uses 4-velocities:

```
g = (k_μ u^μ_obs) / (k_μ u^μ_emit)
```

where:
- k^μ = photon 4-momentum
- u^μ = 4-velocity of observer/emitter

For the accretion disk in Keplerian orbit:

```
u^t_disk = 1 / √(-g_tt - 2Ω·g_tφ - Ω²·g_φφ)
u^φ_disk = Ω · u^t_disk
```

where Ω = dφ/dt = √(M/r³) is the Keplerian angular velocity.

### 4. Doppler Beaming

The observed intensity is boosted by:

```
I_obs = g³ · I_emit
```

This g³ factor comes from:
- g¹ from frequency shift (energy per photon)
- g¹ from photon arrival rate
- g¹ from aberration (solid angle change)

**Visual effect**: The approaching side of the disk appears much brighter than the receding side.

---

## Accretion Disk Physics

### Shakura-Sunyaev Model

The standard thin disk model gives:

**Temperature profile:**
```
T(r) ∝ (Ṁ/r³)^(1/4) ≈ r^(-3/4)
```

**Surface brightness:**
```
I(r) ∝ T⁴ ∝ r^(-3)
```

### Emission Color

Blackbody approximation:
- **Inner disk** (r ≈ 3M, T ~ 10⁷ K): Blue-white / UV
- **Middle disk** (r ≈ 6M, T ~ 10⁶ K): White / yellow
- **Outer disk** (r ≈ 12M, T ~ 10⁵ K): Orange / red

In our shader:
```glsl
float temp = pow(DISK_INNER / r, 0.75);
if (temp > 0.8) color = vec3(0.6, 0.7, 1.0);     // Blue
else if (temp > 0.5) color = vec3(1.0, 0.9, 0.7); // Yellow
else color = vec3(1.0, 0.6, 0.3);                 // Orange
```

### Keplerian Velocity

Orbital velocity at radius r:

```
v_φ = √(M/r)
Ω = v_φ/r = √(M/r³)
```

At ISCO (r ≈ 3M):
```
v_φ ≈ 0.58c  (58% speed of light!)
```

---

## Visual Phenomena

### 1. Einstein Ring / Photon Ring

Light completing multiple orbits around the black hole before escaping creates bright rings at:

- **Primary image**: r ≈ 1.5M (photon sphere)
- **Secondary images**: Fainter rings from 1.5, 2.5, 3.5+ orbits

### 2. Gravitational Lensing

Light paths bend, creating:
- **Primary disk image**: Direct view of equatorial disk
- **Secondary image**: Lensed view of far side of disk (arc above hole)
- **Tertiary image**: Double-lensed view (arc below hole)

### 3. Frame Dragging Asymmetry

For spinning black holes:
- **Co-rotating side**: Photons pulled along, stay longer, brighter
- **Counter-rotating side**: Photons escape faster, dimmer

This creates left-right asymmetry in the disk appearance.

### 4. Shadow Size

The apparent angular size of the black shadow:

```
θ_shadow ≈ (2.6 ± 0.1)√(M²/D²)
```

where D is the observer distance.

For M = 1, D = 25M:
```
θ_shadow ≈ 10.4° (in our simulation)
```

---

## Coordinate Systems

### Boyer-Lindquist Coordinates (r, θ, φ)

**Pros:**
- Metric components time-independent
- Asymptotically flat (→ Minkowski as r → ∞)
- Separates Hamilton-Jacobi equation

**Cons:**
- Coordinate singularity at horizon
- Not good for numerical integration across horizon

### Kerr-Schild Coordinates

Alternative coordinates that are non-singular at horizon:
```
t_KS = t_BL + ∫ 2Mr/(r²-2Mr+a²) dr
```

Used in simulations that track matter falling into black hole.

---

## Computational Optimizations

### 1. Metric Precomputation

Store metric components g_μν instead of recomputing:

```glsl
float gtt, gtphi, grr, gthth, gphiphi;
computeMetric(r, theta, a, gtt, gtphi, grr, gthth, gphiphi);
```

### 2. Christoffel Symbol Simplification

Use explicit derivatives instead of full symbolic computation:

```glsl
float dsig_dr = 2.0 * r;
float dsig_dtheta = -2.0 * a * a * cos(theta) * sin(theta);
float ddlt_dr = 2.0 * r - 2.0 * M;
```

### 3. Early Termination

Stop ray tracing when:
- r < r_horizon → black
- r > 100M → background stars
- Intersect disk → disk color
- Steps > MAX_STEPS → black (absorbed)

### 4. Float vs. Double Precision

Single precision (float) sufficient because:
- Renormalization corrects null geodesic condition
- Errors accumulate slowly compared to step count
- ~4× faster on most GPUs

---

## Validation Tests

### Expected Results

1. **Schwarzschild limit (a=0)**:
   - Perfect circular symmetry
   - Shadow radius ≈ 2.6M
   - No frame dragging

2. **High spin (a=0.9)**:
   - Visible asymmetry
   - Smaller shadow on co-rotating side
   - Brighter primary arc

3. **Energy conservation**:
   - E = -p_t should remain constant
   - Verify: |E_final - E_initial| / E_initial < 10⁻⁴

4. **Null geodesic condition**:
   - ds² = 0 throughout path
   - Verify: |g_μν dx^μ dx^ν| < 10⁻⁶

---

## References for Implementation

1. **Numerical Methods:**
   - Levin & Perez-Giz (2008): "A Periodic Table for Black Hole Orbits"
   - Chan et al. (2018): "GRay2: A General Purpose Geodesic Integrator"

2. **Visualization:**
   - Luminet (1979): First black hole image simulation
   - James et al. (2015): Interstellar visual effects technical paper
   - EHT Collaboration (2019): M87 black hole imaging

3. **Relativity:**
   - Misner, Thorne, Wheeler: "Gravitation" (1973)
   - Chandrasekhar: "Mathematical Theory of Black Holes" (1983)
   - Carroll: "Spacetime and Geometry" (2004)

---

## Extension Ideas

### 1. Radiative Transfer

Include photon absorption/emission along path:

```
dI/dλ = η(ν) - α(ν)·I
```

where η = emissivity, α = absorption coefficient.

### 2. Polarization

Track Stokes parameters (I, Q, U, V) along geodesics for polarized emission.

### 3. Magnetohydrodynamics

Couple to GRMHD simulation data (e.g., from HARM, Athena++).

### 4. Gravitational Waves

Add time-dependent metric perturbations for binary mergers.

### 5. Wormholes

Modify topology to connect two asymptotically flat regions.

---

**This implementation captures the essential physics of Kerr black holes while remaining computationally tractable for real-time rendering.**
