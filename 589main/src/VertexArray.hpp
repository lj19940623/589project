#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include "generalHeader.h"
//====================================================================================
// VertexArray is map-based class buffer handler
class VertexArray {
	std::map<string, GLuint> buffers;
	std::map<string, int> indices;
public:
	GLuint id;
	unsigned int count;
	VertexArray(int c);
	VertexArray(const VertexArray &v);
	void addBuffer(string name, int index, vector<float> buffer);
	void updateBuffer(string name, vector<float> buffer);
	void addBuffer(string name, int index, vector<vec3> buffer);
	void updateBuffer(string name, vector<vec3> buffer);
	~VertexArray();
};

#endif
