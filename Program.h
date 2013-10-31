#pragma once

#include <initializer_list>
#include "gl.h"
#include "Shader.h"

namespace gl
{
    class Program
    {
    public:
        Program();
        Program(std::initializer_list<Shader> shaders);
        ~Program();

        GLuint get();

        GLint getUniformLocation(const std::string& name);

        void use();

    private:
        GLuint program;
    };
}
