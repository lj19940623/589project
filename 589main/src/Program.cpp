
#include "Program.hpp"

Program :: Program() {
	vertex_shader = 0;
	fragment_shader = 0;
	id = 0;
}
// take shader path string as params
Program :: Program(string vertex_path, string fragment_path) {
	init(vertex_path, fragment_path);
}
// init program and add shader to program and link
void Program :: init(string vertex_path, string fragment_path) {
	id = glCreateProgram();
	vertex_shader = addShader(vertex_path, GL_VERTEX_SHADER);
	fragment_shader = addShader(fragment_path, GL_FRAGMENT_SHADER);
	if (vertex_shader)
	glAttachShader(id, vertex_shader);
	if (fragment_shader)
	glAttachShader(id, fragment_shader);

	glLinkProgram(id);
}
// add shader func and Compile shade code
GLuint Program :: addShader(string path, GLuint type) {
	std::ifstream in(path);
	string buffer = [&in] {
		std::ostringstream ss {};
		ss << in.rdbuf();
		return ss.str();
	}();
	const char *buffer_array[] = { buffer.c_str() };

	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, buffer_array, 0);
	glCompileShader(shader);

	// Compile results
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shader, info.length(), &length, &info[0]);
		cerr << "ERROR compiling shader:" << endl << endl;
		cerr << info << endl;
	}
	return shader;
}
// destroy Program
Program :: ~Program() {
	glUseProgram(0);
	glDeleteProgram(id);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}
