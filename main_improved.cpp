/*
 * IMPROVED Kerr Black Hole Visualization v2.0
 * C++17 OpenGL 4.5 - Windows Ready
 * 
 * New Features:
 * - RK5 Cash-Karp integration with adaptive stepping
 * - Multiple ray bounces for self-lensing
 * - Bloom post-processing
 * - Enhanced disk physics
 * - Better performance optimization
 */

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

// Configuration
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Kerr Black Hole v2.0 - Enhanced Ray Tracing";

// Enhanced global state
struct AppState {
    float time = 0.0f;
    float spinParameter = 0.9f;
    float exposure = 1.2f;
    float inclination = 85.0f;
    float cameraDistance = 25.0f;
    int maxBounces = 3;
    float bloomStrength = 0.5f;
    bool enableBloom = true;
    bool paused = false;
    bool running = true;
    bool showHelp = false;
} state;

// Shader utility functions
std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[2048];
        glGetShaderInfoLog(shader, 2048, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint createShaderProgram(const char* vertSource, const char* fragSource) {
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);
    
    if (!vertShader || !fragShader) return 0;
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Program linking error:\n" << infoLog << std::endl;
        return 0;
    }
    
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}

GLuint createComputeShader(const char* source) {
    GLuint shader = compileShader(GL_COMPUTE_SHADER, source);
    if (shader == 0) return 0;
    
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[2048];
        glGetProgramInfoLog(program, 2048, nullptr, infoLog);
        std::cerr << "Compute shader linking error:\n" << infoLog << std::endl;
        return 0;
    }
    
    glDeleteShader(shader);
    return program;
}

// Create fullscreen quad
GLuint createFullscreenQuad() {
    float vertices[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    return VAO;
}

void handleInput(SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        state.running = false;
    }
    else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                state.running = false;
                break;
            case SDLK_SPACE:
                state.paused = !state.paused;
                std::cout << "Animation " << (state.paused ? "paused" : "resumed") << std::endl;
                break;
            case SDLK_h:
                state.showHelp = !state.showHelp;
                if (state.showHelp) {
                    std::cout << "\n=== ENHANCED CONTROLS ===\n"
                              << "ESC:     Quit\n"
                              << "SPACE:   Pause/Resume\n"
                              << "H:       Toggle this help\n"
                              << "↑/↓:     Exposure ±\n"
                              << "A/D:     Spin parameter ±\n"
                              << "W/S:     Inclination ±\n"
                              << "Q/E:     Camera distance ±\n"
                              << "1/2:     Ray bounces ±\n"
                              << "3/4:     Bloom strength ±\n"
                              << "B:       Toggle bloom\n"
                              << "R:       Reset to defaults\n"
                              << "=======================\n" << std::endl;
                }
                break;
            case SDLK_UP:
                state.exposure *= 1.1f;
                std::cout << "Exposure: " << state.exposure << std::endl;
                break;
            case SDLK_DOWN:
                state.exposure /= 1.1f;
                std::cout << "Exposure: " << state.exposure << std::endl;
                break;
            case SDLK_a:
                state.spinParameter = std::max(0.0f, state.spinParameter - 0.05f);
                std::cout << "Spin a: " << state.spinParameter << std::endl;
                break;
            case SDLK_d:
                state.spinParameter = std::min(0.998f, state.spinParameter + 0.05f);
                std::cout << "Spin a: " << state.spinParameter << std::endl;
                break;
            case SDLK_w:
                state.inclination = std::min(89.9f, state.inclination + 2.0f);
                std::cout << "Inclination: " << state.inclination << "°" << std::endl;
                break;
            case SDLK_s:
                state.inclination = std::max(0.1f, state.inclination - 2.0f);
                std::cout << "Inclination: " << state.inclination << "°" << std::endl;
                break;
            case SDLK_q:
                state.cameraDistance = std::max(10.0f, state.cameraDistance - 1.0f);
                std::cout << "Distance: " << state.cameraDistance << std::endl;
                break;
            case SDLK_e:
                state.cameraDistance = std::min(50.0f, state.cameraDistance + 1.0f);
                std::cout << "Distance: " << state.cameraDistance << std::endl;
                break;
            case SDLK_1:
                state.maxBounces = std::max(1, state.maxBounces - 1);
                std::cout << "Max bounces: " << state.maxBounces << std::endl;
                break;
            case SDLK_2:
                state.maxBounces = std::min(5, state.maxBounces + 1);
                std::cout << "Max bounces: " << state.maxBounces << std::endl;
                break;
            case SDLK_3:
                state.bloomStrength = std::max(0.0f, state.bloomStrength - 0.1f);
                std::cout << "Bloom: " << state.bloomStrength << std::endl;
                break;
            case SDLK_4:
                state.bloomStrength = std::min(2.0f, state.bloomStrength + 0.1f);
                std::cout << "Bloom: " << state.bloomStrength << std::endl;
                break;
            case SDLK_b:
                state.enableBloom = !state.enableBloom;
                std::cout << "Bloom " << (state.enableBloom ? "enabled" : "disabled") << std::endl;
                break;
            case SDLK_r:
                state.spinParameter = 0.9f;
                state.exposure = 1.2f;
                state.inclination = 85.0f;
                state.cameraDistance = 25.0f;
                state.maxBounces = 3;
                state.bloomStrength = 0.5f;
                std::cout << "Reset to defaults" << std::endl;
                break;
        }
    }
}

void printSystemInfo() {
    std::cout << "========================================\n"
              << "Kerr Black Hole Visualizer v2.0\n"
              << "========================================\n"
              << "OpenGL: " << glGetString(GL_VERSION) << "\n"
              << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n"
              << "Renderer: " << glGetString(GL_RENDERER) << "\n"
              << "Vendor: " << glGetString(GL_VENDOR) << "\n"
              << "========================================\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "OpenGL context failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    SDL_GL_SetSwapInterval(1); // VSync
    
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(glewError) << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    printSystemInfo();
    
    // Check compute shader support
    if (!GLEW_ARB_compute_shader) {
        std::cerr << "Compute shaders not supported!" << std::endl;
        return -1;
    }
    
    // Load shaders - try improved version first, fallback to original
    std::string compSource = loadShaderSource("blackhole_improved.comp");
    if (compSource.empty()) {
        std::cout << "Loading original shader..." << std::endl;
        compSource = loadShaderSource("blackhole.comp");
    } else {
        std::cout << "Loaded improved shader!" << std::endl;
    }
    
    std::string vertSource = loadShaderSource("shader.vert");
    std::string fragSource = loadShaderSource("shader.frag");
    
    if (vertSource.empty() || fragSource.empty() || compSource.empty()) {
        std::cerr << "Failed to load shaders" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    GLuint displayProgram = createShaderProgram(vertSource.c_str(), fragSource.c_str());
    GLuint computeProgram = createComputeShader(compSource.c_str());
    
    if (!displayProgram || !computeProgram) {
        std::cerr << "Shader compilation failed" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Create output texture
    GLuint outputTexture;
    glGenTextures(1, &outputTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 
                 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    // Create bloom buffer
    GLuint bloomTexture;
    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 
                 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(1, bloomTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    GLuint quadVAO = createFullscreenQuad();
    
    glUseProgram(displayProgram);
    glUniform1i(glGetUniformLocation(displayProgram, "screenTexture"), 0);
    
    // Main loop
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;
    float fpsTimer = 0.0f;
    
    std::cout << "\n=== CONTROLS ===\n"
              << "Press H for help\n"
              << "ESC to quit\n"
              << "================\n" << std::endl;
    
    while (state.running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        if (!state.paused) {
            state.time += deltaTime;
        }
        
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            std::cout << "FPS: " << (int)fps 
                      << " | Time: " << state.time 
                      << "s | Spin: " << state.spinParameter 
                      << " | Incl: " << state.inclination << "°"
                      << " | Bounces: " << state.maxBounces << std::endl;
            frameCount = 0;
            fpsTimer = 0.0f;
        }
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleInput(event);
        }
        
        // Dispatch compute shader
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "uTime"), state.time);
        glUniform1f(glGetUniformLocation(computeProgram, "uSpinParameter"), state.spinParameter);
        glUniform1f(glGetUniformLocation(computeProgram, "uExposure"), state.exposure);
        glUniform1f(glGetUniformLocation(computeProgram, "uInclination"), state.inclination);
        glUniform1f(glGetUniformLocation(computeProgram, "uCameraDistance"), state.cameraDistance);
        glUniform2f(glGetUniformLocation(computeProgram, "uResolution"), 
                    (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
        glUniform1i(glGetUniformLocation(computeProgram, "uMaxBounces"), state.maxBounces);
        glUniform1f(glGetUniformLocation(computeProgram, "uBloomStrength"), 
                    state.enableBloom ? state.bloomStrength : 0.0f);
        
        glDispatchCompute((WINDOW_WIDTH + 15) / 16, (WINDOW_HEIGHT + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        
        // Render to screen
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(displayProgram);
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        SDL_GL_SwapWindow(window);
    }
    
    // Cleanup
    glDeleteProgram(displayProgram);
    glDeleteProgram(computeProgram);
    glDeleteTextures(1, &outputTexture);
    glDeleteTextures(1, &bloomTexture);
    glDeleteVertexArrays(1, &quadVAO);
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "\nThank you for exploring spacetime! 🌌" << std::endl;
    
    return 0;
}
