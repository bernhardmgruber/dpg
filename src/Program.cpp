#include "Program.h"

using namespace std;

namespace gl {
	Program::Program() {
		program = 0;
	}

	Program::Program(initializer_list<const Shader> shaders) {
		program = glCreateProgram();

		for (const auto& shader : shaders)
			glAttachShader(program, shader.get());

		glLinkProgram(program);

		// check linker status
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			char* infoLog = new char[length];
			glGetProgramInfoLog(program, length, nullptr, infoLog);

			string log(infoLog);
			delete[] infoLog;

			throw exception(("Failed to link program:\n" + log).c_str());
		}
	}

	Program::~Program() {
		glDeleteProgram(program);
	}

	GLuint Program::get() const {
		return program;
	}

	GLint Program::getUniformLocation(const std::string& name) const {
		return glGetUniformLocation(program, name.c_str());
	}

	void Program::use() const {
		glUseProgram(program);
	}
}