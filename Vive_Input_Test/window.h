#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <GL/glew.h>
#include "shader.h"

namespace Window
{
	bool init();
	void init_gl();
	void shutdown();
	
	void draw_left_side( GLuint texture );
	void draw_right_side( GLuint texture );
	void present();

	// PUBLIC MEMBERS 
	extern SDL_Window* companion_window;
	extern const int companion_width;
	extern const int companion_height;
	extern SDL_GLContext gl_context;
	
	extern GLuint window_vao;	// Vertex attribute object
	extern GLuint window_vbo;	// Vertex buffer object
	extern GLuint window_ebo;	// element buffer object, the order for vertices to be drawn

	extern Shader shader;
}

#endif