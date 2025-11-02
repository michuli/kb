#version 450 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

// Enhanced post-processing options
uniform float uChromatic = 0.002;  // Chromatic aberration strength
uniform float uVignette = 0.3;      // Vignette strength
uniform float uSharpen = 0.15;      // Sharpening amount

void main() {
    vec2 uv = TexCoord;
    vec3 color = vec3(0.0);
    
    // Chromatic aberration (subtle lens effect)
    vec2 distFromCenter = uv - 0.5;
    float dist = length(distFromCenter);
    
    vec2 offset = normalize(distFromCenter) * dist * uChromatic;
    
    float r = texture(screenTexture, uv - offset).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv + offset).b;
    
    color = vec3(r, g, b);
    
    // Subtle sharpening (unsharp mask)
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec3 blur = vec3(0.0);
    blur += texture(screenTexture, uv + vec2(-1, -1) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2( 0, -1) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2( 1, -1) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2(-1,  0) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2( 1,  0) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2(-1,  1) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2( 0,  1) * texelSize).rgb;
    blur += texture(screenTexture, uv + vec2( 1,  1) * texelSize).rgb;
    blur /= 8.0;
    
    color += (color - blur) * uSharpen;
    
    // Enhanced vignette (smooth falloff)
    float vignetteFactor = smoothstep(0.9, 0.3, dist * 1.4);
    vignetteFactor = mix(1.0 - uVignette, 1.0, vignetteFactor);
    color *= vignetteFactor;
    
    // Subtle color grading (cooler shadows, warmer highlights)
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    vec3 shadowTint = vec3(0.95, 0.97, 1.0);   // Slightly blue
    vec3 highlightTint = vec3(1.0, 0.98, 0.95); // Slightly warm
    
    float shadowMix = smoothstep(0.3, 0.0, luminance);
    float highlightMix = smoothstep(0.7, 1.0, luminance);
    
    color = mix(color, color * shadowTint, shadowMix * 0.2);
    color = mix(color, color * highlightTint, highlightMix * 0.15);
    
    // Output
    FragColor = vec4(color, 1.0);
}
