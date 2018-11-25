#pragma once

#include "Shader.h"
#include "gl.h"
#include <initializer_list>

namespace gl {
	class Program {
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
