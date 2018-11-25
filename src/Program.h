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
        Program(std::initializer_list<const Shader> shaders);
        ~Program();

        GLuint get() const;

        GLint getUniformLocation(const std::string& name) const;

        void use() const;

    private:
        GLuint program;
    };
}
