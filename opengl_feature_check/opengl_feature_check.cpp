#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const unsigned int SCR_WIDTH = 400;
const unsigned int SCR_HEIGHT = 300;

static void APIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Test<GLFW+GLAD>", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_output, NULL);

    int profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);

    if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "Core Profile" << std::endl;
    }
    else if (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
        std::cout << "Compatibility Profile" << std::endl;
    }
    else {
        std::cout << "Unknown Profile" << std::endl;
    }

    auto extensions = glGetString(GL_EXTENSIONS);
    if (extensions)
        std::cout << extensions << std::endl;

    std::cout << "error: " << glGetError() << std::endl;

    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);


    std::cout << "ext num: " << numExtensions << std::endl;

    for (GLint i = 0; i < numExtensions; i++) {
        const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        std::cout << extension << std::endl;
    }

    std::cout << "GLAD_GL_KHR_texture_compression_astc_ldr: " << GLAD_GL_KHR_texture_compression_astc_ldr << std::endl;

    GLint supported;
    glGetInternalformativ(GL_TEXTURE_2D, GL_COMPRESSED_RGBA_ASTC_8x8, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
    std::cout << "support GL_COMPRESSED_RGBA_ASTC_8x8 " << supported << std::endl;

    glGetInternalformativ(GL_TEXTURE_CUBE_MAP, GL_COMPRESSED_RGBA8_ETC2_EAC, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
    std::cout << "support GL_TEXTURE_CUBE_MAP GL_COMPRESSED_RGBA8_ETC2_EAC " << supported << std::endl;


    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_ASTC_8x8, 1024, 1024);

    std::cout << "error: " << glGetError() << std::endl;

    glDeleteTextures(1, &texture);

    const char* version = (const char*)glGetString(GL_VERSION);
    std::cout << "version: " << version << std::endl;

    // glfw: terminate, clearing all previously allocated GLFWresources.
    glfwTerminate();

    system("pause");
    return 0;
}

static void APIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{

    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;
    if (severity == GL_DEBUG_SEVERITY_LOW || severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        return;
    }

    std::cout << "\ndebug message(" << id << "):" << message << "\n";
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API ";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System ";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler ";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party ";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: APPLICATION ";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error ";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour ";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour ";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability ";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance ";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker ";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group ";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group ";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other ";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
}
