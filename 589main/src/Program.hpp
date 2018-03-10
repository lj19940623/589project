#ifndef SHADER_HPP
#define SHADER_HPP
#include "generalHeader.h"
/*====================================================================================
 * program is class that holds shader
 */
class Program {
	GLuint vertex_shader;
	GLuint fragment_shader;
 public:
	GLuint id;
	Program();
	// take shader path string as params
	Program(string vertex_path, string fragment_path);
	// init program and add shader to program and link
	void init(string vertex_path, string fragment_path);
	// add shader func and Compile shade code
	GLuint addShader(string path, GLuint type);
	// destroy Program
	~Program();
};
#endif
