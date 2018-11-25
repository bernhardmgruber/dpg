#include <exception>
#include "utils.h"

#include "Shader.h"

using namespace std;

namespace gl
{
    Shader::Shader()
    {
        shader = 0;
    }

    Shader::Shader(GLenum shaderType, const std::string& file)
    {
        shader = glCreateShader(shaderType);

        // read source
        string buffer;
        if (!readFile(file, buffer))
            throw exception(("Could not read file " + file).c_str());
        const char* source = buffer.c_str();
        glShaderSource(shader, 1, (const char**)&source, nullptr);

        // compile
        glCompileShader(shader);

        // check compilation status
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

            char* infoLog = new char[length];
            glGetShaderInfoLog(shader, length, nullptr, infoLog);

            string log(infoLog);
            delete[] infoLog;

            throw exception(("Failed to compile shader " + file + ":\n" + log).c_str());
        }
    }

    Shader::~Shader()
    {
        glDeleteShader(shader);
    }

    GLuint Shader::get() const
    {
        return shader;
    }
}