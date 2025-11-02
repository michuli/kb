// Simplified Linux version for testing (no SDL2, just OpenGL context)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;

std::string loadFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << path << std::endl;
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
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader error:\n" << log << std::endl;
        return 0;
    }
    return shader;
}

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }

    // Create OpenGL context
    static int attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    
    XVisualInfo* vi = glXChooseVisual(display, 0, attribs);
    Window root = DefaultRootWindow(display);
    
    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(display, root, vi->visual, AllocNone);
    swa.event_mask = ExposureMask | KeyPressMask;
    
    Window win = XCreateWindow(display, root, 0, 0, WIDTH, HEIGHT, 0,
                               vi->depth, InputOutput, vi->visual,
                               CWColormap | CWEventMask, &swa);
    XMapWindow(display, win);
    XStoreName(display, win, "Kerr Black Hole");
    
    GLXContext glc = glXCreateContext(display, vi, nullptr, GL_TRUE);
    glXMakeCurrent(display, win, glc);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return 1;
    }
    
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Load and compile shaders
    std::string compSrc = loadFile("blackhole.comp");
    if (compSrc.empty()) return 1;
    
    GLuint computeShader = compileShader(GL_COMPUTE_SHADER, compSrc.c_str());
    if (!computeShader) return 1;
    
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    
    GLint success;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(computeProgram, 1024, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
        return 1;
    }
    
    // Create output texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    // Render one frame
    glUseProgram(computeProgram);
    glUniform1f(glGetUniformLocation(computeProgram, "uTime"), 0.0f);
    glUniform1f(glGetUniformLocation(computeProgram, "uSpinParameter"), 0.9f);
    glUniform1f(glGetUniformLocation(computeProgram, "uExposure"), 1.0f);
    glUniform1f(glGetUniformLocation(computeProgram, "uInclination"), 85.0f);
    glUniform1f(glGetUniformLocation(computeProgram, "uCameraDistance"), 25.0f);
    glUniform2f(glGetUniformLocation(computeProgram, "uResolution"), (float)WIDTH, (float)HEIGHT);
    
    std::cout << "Dispatching compute shader..." << std::endl;
    glDispatchCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    std::cout << "Reading pixels..." << std::endl;
    float* pixels = new float[WIDTH * HEIGHT * 4];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);
    
    // Save as PPM
    std::cout << "Saving image..." << std::endl;
    std::ofstream out("output.ppm", std::ios::binary);
    out << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    for (int i = 0; i < WIDTH * HEIGHT * 4; i += 4) {
        out << (unsigned char)(pixels[i] * 255);
        out << (unsigned char)(pixels[i+1] * 255);
        out << (unsigned char)(pixels[i+2] * 255);
    }
    out.close();
    
    delete[] pixels;
    std::cout << "Done! Output saved to output.ppm" << std::endl;
    
    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glc);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
    
    return 0;
}
