/*
 * Kerr Black Hole Visualization with Gravitational Lensing
 * C++17 OpenGL 4.5 - Windows Ready
 * Physics: Kerr metric geodesic integration (Boyer-Lindquist coordinates)
 * Rendering: Compute shader ray tracing with Doppler beaming & gravitational redshift
 */

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

// Configuration
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Kerr Black Hole - Gravitational Lensing";

// Global state
struct AppState {
    float time = 0.0f;
    float spinParameter = 0.9f;  // Kerr spin a ∈ [0, 0.998]
    float exposure = 1.0f;
    float inclination = 85.0f;   // Observer inclination (degrees)
    float cameraDistance = 25.0f;
    bool paused = false;
    bool running = true;
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
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint createShaderProgram(const char* vertSource, const char* fragSource) {
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);
    
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
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
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
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord attribute
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
                std::cout << "Spin parameter a: " << state.spinParameter << std::endl;
                break;
            case SDLK_d:
                state.spinParameter = std::min(0.998f, state.spinParameter + 0.05f);
                std::cout << "Spin parameter a: " << state.spinParameter << std::endl;
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
                std::cout << "Camera distance: " << state.cameraDistance << std::endl;
                break;
            case SDLK_e:
                state.cameraDistance = std::min(50.0f, state.cameraDistance + 1.0f);
                std::cout << "Camera distance: " << state.cameraDistance << std::endl;
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    // Create window
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
    
    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(glewError) << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Load shaders
    std::string vertSource = loadShaderSource("shader.vert");
    std::string fragSource = loadShaderSource("shader.frag");
    std::string compSource = loadShaderSource("blackhole.comp");
    
    if (vertSource.empty() || fragSource.empty() || compSource.empty()) {
        std::cerr << "Failed to load shader files" << std::endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Create shader programs
    GLuint displayProgram = createShaderProgram(vertSource.c_str(), fragSource.c_str());
    GLuint computeProgram = createComputeShader(compSource.c_str());
    
    if (displayProgram == 0 || computeProgram == 0) {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    // Create fullscreen quad
    GLuint quadVAO = createFullscreenQuad();
    
    // Set up uniforms
    glUseProgram(displayProgram);
    glUniform1i(glGetUniformLocation(displayProgram, "screenTexture"), 0);
    
    // Main loop
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;
    float fpsTimer = 0.0f;
    
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "ESC:    Quit" << std::endl;
    std::cout << "SPACE:  Pause/Resume animation" << std::endl;
    std::cout << "↑/↓:    Adjust exposure" << std::endl;
    std::cout << "A/D:    Adjust spin parameter" << std::endl;
    std::cout << "W/S:    Adjust inclination" << std::endl;
    std::cout << "Q/E:    Adjust camera distance" << std::endl;
    std::cout << "================\n" << std::endl;
    
    while (state.running) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        if (!state.paused) {
            state.time += deltaTime;
        }
        
        // FPS counter
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            std::cout << "FPS: " << frameCount << " | Time: " << state.time 
                      << "s | Spin: " << state.spinParameter 
                      << " | Incl: " << state.inclination << "°" << std::endl;
            frameCount = 0;
            fpsTimer = 0.0f;
        }
        
        // Handle input
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
        
        glDispatchCompute((WINDOW_WIDTH + 15) / 16, (WINDOW_HEIGHT + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        // Render to screen
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(displayProgram);
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Swap buffers
        SDL_GL_SwapWindow(window);
    }
    
    // Cleanup
    glDeleteProgram(displayProgram);
    glDeleteProgram(computeProgram);
    glDeleteTextures(1, &outputTexture);
    glDeleteVertexArrays(1, &quadVAO);
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
