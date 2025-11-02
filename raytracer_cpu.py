#!/usr/bin/env python3
"""
CPU-based Kerr Black Hole Ray Tracer
Implements the same physics as blackhole.comp but on CPU
"""

import numpy as np
from PIL import Image
import sys

# Constants
WIDTH = 800
HEIGHT = 600
M = 1.0
MAX_STEPS = 256
DISK_INNER = 3.0
DISK_OUTER = 12.0
DISK_THICKNESS = 0.15
PI = np.pi

def sigma(r, theta, a):
    return r**2 + a**2 * np.cos(theta)**2

def delta(r, a):
    return r**2 - 2*M*r + a**2

def event_horizon(a):
    return M + np.sqrt(M**2 - a**2)

def geodesic_deriv(pos, vel, a):
    """Compute derivatives for geodesic equation"""
    r, theta = pos[1], pos[2]
    
    sig = sigma(r, theta, a)
    dlt = delta(r, a)
    sin_th = np.sin(theta)
    cos_th = np.cos(theta)
    sin2 = sin_th**2
    
    dt, dr, dtheta, dphi = vel
    
    # Simplified geodesic derivatives
    accel = np.zeros(4)
    
    # d²r/dλ²
    term = M * (r**2 - a**2 * cos_th**2) / (sig**2)
    accel[1] = -term * dt**2
    accel[1] += (r - M) / sig * dr**2
    accel[1] -= r * dtheta**2
    
    # d²θ/dλ²
    accel[2] = a**2 * cos_th * sin_th / sig * dt**2
    accel[2] += cos_th * sin_th * dphi**2
    
    return -accel

def rk4_step(pos, vel, a, dlambda):
    """RK4 integration step"""
    # k1
    k1_vel = geodesic_deriv(pos, vel, a)
    k1_pos = vel.copy()
    
    # k2
    pos2 = pos + 0.5 * dlambda * k1_pos
    vel2 = vel + 0.5 * dlambda * k1_vel
    k2_vel = geodesic_deriv(pos2, vel2, a)
    k2_pos = vel2
    
    # k3  
    pos3 = pos + 0.5 * dlambda * k2_pos
    vel3 = vel + 0.5 * dlambda * k2_vel
    k3_vel = geodesic_deriv(pos3, vel3, a)
    k3_pos = vel3
    
    # k4
    pos4 = pos + dlambda * k3_pos
    vel4 = vel + dlambda * k3_vel
    k4_vel = geodesic_deriv(pos4, vel4, a)
    k4_pos = vel4
    
    # Update
    pos += dlambda / 6.0 * (k1_pos + 2*k2_pos + 2*k3_pos + k4_pos)
    vel += dlambda / 6.0 * (k1_vel + 2*k2_vel + 2*k3_vel + k4_vel)
    
    # Keep theta in range
    pos[2] = np.clip(pos[2], 0.01, PI - 0.01)
    
    return pos, vel

def intersect_disk(pos):
    """Check disk intersection"""
    r, theta = pos[1], pos[2]
    dist = abs(theta - PI/2)
    return dist < DISK_THICKNESS and DISK_INNER <= r <= DISK_OUTER, r

def disk_emission(r, phi):
    """Disk color"""
    temp = (DISK_INNER / r)**0.75
    
    if temp > 0.8:
        color = np.array([0.6, 0.7, 1.0])  # Blue
    elif temp > 0.5:
        color = np.array([1.0, 0.9, 0.7])  # Yellow
    else:
        color = np.array([1.0, 0.6, 0.3])  # Orange
    
    intensity = (DISK_INNER / r)**3
    return color * intensity

def starfield(direction):
    """Simple starfield"""
    # Hash-based stars
    h = abs(np.sin(direction[0] * 12.9898 + direction[1] * 78.233) * 43758.5453) % 1.0
    
    if h > 0.998:
        return np.array([1.0, 0.95, 0.9]) * (h - 0.998) / 0.002
    return np.array([0.05, 0.05, 0.08])  # Dark sky

def trace_ray(origin, direction, a):
    """Main ray tracing function"""
    r0 = np.linalg.norm(origin)
    theta0 = np.arccos(origin[1] / r0)
    phi0 = np.arctan2(origin[2], origin[0])
    
    # Initialize
    pos = np.array([0.0, r0, theta0, phi0])
    
    # Simple initial velocity
    vel = np.array([-1.0, direction[0]*0.1, direction[1]*0.1, direction[2]*0.01])
    
    dlambda = 0.05
    r_horizon = event_horizon(a)
    
    for step in range(MAX_STEPS):
        r = pos[1]
        theta = pos[2]
        
        # Adaptive step
        dlambda = 0.02 + 0.08 * min(1.0, abs(r - 1.5) / 2.0)
        
        # Check termination
        if r < r_horizon:
            return np.array([0.0, 0.0, 0.0])  # Black
        
        if r > 100:
            # Escaped - show starfield
            final_dir = np.array([
                np.sin(theta) * np.cos(pos[3]),
                np.cos(theta),
                np.sin(theta) * np.sin(pos[3])
            ])
            return starfield(final_dir)
        
        # Check disk
        hit_disk, disk_r = intersect_disk(pos)
        if hit_disk:
            color = disk_emission(disk_r, pos[3])
            # Simple redshift approximation
            g = 1.0 + 0.3 * np.cos(pos[3])
            return color * g**3
        
        # Integrate
        pos, vel = rk4_step(pos, vel, a, dlambda)
    
    return np.array([0.0, 0.0, 0.0])

def aces_tonemap(color):
    """ACES tone mapping"""
    a, b, c, d, e = 2.51, 0.03, 2.43, 0.59, 0.14
    return np.clip((color * (a * color + b)) / (color * (c * color + d) + e), 0, 1)

def render(width, height, spin, inclination, distance):
    """Render the black hole"""
    print(f"Rendering {width}x{height} with a={spin:.2f}, incl={inclination:.0f}°...")
    
    img = np.zeros((height, width, 3))
    
    # Camera setup
    orbit_angle = 0.0
    incl_rad = np.radians(inclination)
    
    cam_pos = np.array([
        distance * np.sin(incl_rad) * np.cos(orbit_angle),
        distance * np.cos(incl_rad),
        distance * np.sin(incl_rad) * np.sin(orbit_angle)
    ])
    
    forward = -cam_pos / np.linalg.norm(cam_pos)
    right = np.array([-np.sin(orbit_angle), 0, np.cos(orbit_angle)])
    up = np.cross(right, forward)
    
    fov_scale = np.tan(np.radians(45) / 2)
    
    for y in range(height):
        if y % 50 == 0:
            print(f"  Row {y}/{height}")
        for x in range(width):
            # NDC coordinates
            u = (x + 0.5) / width
            v = (y + 0.5) / height
            ndc_x = (2 * u - 1) * (width / height) * fov_scale
            ndc_y = (2 * v - 1) * fov_scale
            
            # Ray direction
            ray_dir = forward + right * ndc_x + up * ndc_y
            ray_dir = ray_dir / np.linalg.norm(ray_dir)
            
            # Trace
            color = trace_ray(cam_pos, ray_dir, spin)
            
            # Exposure
            color *= 1.0
            
            # Tone map
            color = aces_tonemap(color)
            
            # Gamma
            color = color ** (1.0 / 2.2)
            
            img[y, x] = color
    
    return img

if __name__ == "__main__":
    print("=" * 60)
    print("KERR BLACK HOLE RAY TRACER - CPU VERSION")
    print("=" * 60)
    
    # Render
    img = render(WIDTH, HEIGHT, spin=0.9, inclination=85, distance=25)
    
    # Save
    img_uint8 = (img * 255).astype(np.uint8)
    im = Image.fromarray(img_uint8)
    im.save("kerr_output.png")
    
    print(f"\n✓ Saved to kerr_output.png")
    print("=" * 60)
