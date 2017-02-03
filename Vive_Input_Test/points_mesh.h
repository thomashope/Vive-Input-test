#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include "shader.h"
#include <vector>

class PointsMesh
{
public:
	PointsMesh();
	~PointsMesh() {}

	void init( Shader* shader, std::string filename );

	void bind();
	void draw();

	std::vector<GLfloat> data_;
	std::string name_;

protected:
	GLuint vao_;
	GLuint vbo_;
	size_t num_verts_;
};