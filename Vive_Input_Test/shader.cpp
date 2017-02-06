#include "shader.h"
#include <iostream>

Shader::Shader() :
	program_(0)
{

}

Shader::~Shader()
{
	if( program_ )
	{
		glDeleteProgram( program_ );
		program_ = 0;
	}
}

 bool Shader::init( const char* name, const char* vertex_source, const char* fragment_source )
 {
	 program_ = glCreateProgram();
	 name_ = name;

	 // Vertex shader
	 GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
	 glShaderSource( vertex_shader, 1, &vertex_source, NULL );
	 glCompileShader( vertex_shader );

	 GLint status = GL_FALSE;
	 glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &status );
	 if( status != GL_TRUE )
	 {
		 // Get the length of the error log
		 GLint log_length = 0;
		 glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &log_length );

		 // Now get the error log itself
		 GLchar* buffer = new GLchar[log_length];
		 glGetShaderInfoLog( vertex_shader, log_length, NULL, buffer );

		 // Print the error
		 printf( "ERROR: compiling shader...\n" );
		 printf( "%s", buffer );
		 delete[] buffer;
		 glDeleteProgram( program_ );
		 glDeleteShader( vertex_shader );
		 return false;
	 }
	 else
	 {
		 glAttachShader( program_, vertex_shader );
		 glDeleteShader( vertex_shader ); // We can throw it away not it's been attached
	 }

	 // Fragment shader
	 GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
	 glShaderSource( fragment_shader, 1, &fragment_source, NULL );
	 glCompileShader( fragment_shader );

	 status = GL_FALSE;
	 glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &status );
	 if( status != GL_TRUE )
	 {
		 // Get the length of the error log
		 GLint log_length = 0;
		 glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &log_length );

		 // Now get the error log itself
		 GLchar* buffer = new GLchar[log_length];
		 glGetShaderInfoLog( fragment_shader, log_length, NULL, buffer );

		 // Print the error
		 printf( "ERROR: compiling shader...\n" );
		 printf( "%s", buffer );
		 delete[] buffer;
		 glDeleteProgram( program_ );
		 glDeleteShader( fragment_shader );
		 return false;
	 }
	 else
	 {
		 glAttachShader( program_, fragment_shader );
		 glDeleteShader( fragment_shader );
	 }

	 // Now link the shaders into the program
	 glLinkProgram( program_ );
	 status = GL_TRUE;
	 glGetProgramiv( program_, GL_LINK_STATUS, &status );
	 if( status != GL_TRUE )
	 {
		 printf( "Failed to link %s program\n", name );
		 glDeleteProgram( program_ );
		 program_ = 0;
		 return false;
	 }
	 else
	 {
		 printf( "Shader success! %s : %d\n", name, program_ );
	 }

	 // Reset the current shader program
	 glUseProgram( 0 );
	 return true;
 }

 void Shader::bind()
 {
	 glUseProgram( program_ );
 }

 GLint Shader::getUniformLocation( std::string name )
 {
	GLint uniform = glGetUniformLocation( program_, name.c_str() );
	if( uniform == -1 ) std::cout << "Could not find " << name << " in shader " << name_ << std::endl;
	return uniform;
 }

 GLint Shader::getAttributeLocation( std::string name )
 {
	 GLint attrib = glGetAttribLocation( program_, name.c_str() );
	 if( attrib == -1 ) std::cout << "Could not find '" << name << "' in shader '" << name_ << "'" << std::endl;
	 return attrib;
 }