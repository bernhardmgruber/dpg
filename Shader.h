#pragma once

#include "gl.h"
#include <string>

namespace gl
{
    class Shader
    {
    public:
        Shader();
        Shader(GLenum shaderType, const std::string& file);
        ~Shader();

        GLuint get();

    private:
        GLuint shader;
    };
}



