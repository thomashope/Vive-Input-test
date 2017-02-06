#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>

class Shader
{
public:
	Shader();
	~Shader();


	bool init( const char* name, const char* vertex_source, const char* fragment_source );
	void bind();

	GLint getUniformLocation( std::string name );
	GLint getAttributeLocation( std::string name );

protected:
	GLuint program_;
	std::string name_;
};

#endif