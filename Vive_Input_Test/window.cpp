#include "window.h"

namespace Window
{
	SDL_Window* companion_window = nullptr;
	const int companion_width = 1280;
	const int companion_height = 640;
	SDL_GLContext gl_context = 0;

	GLuint window_vao = 0;	// Vertex attribute object
	GLuint window_vbo = 0;	// Vertex buffer object
	GLuint window_ebo = 0;	// element buffer object, the order for vertices to be drawn

	bool init()
	{
		companion_window = SDL_CreateWindow(
			"Hello VR",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			companion_width,
			companion_height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

		if( companion_window == NULL )
		{
			return false;
		}

		// Setup the OpenGL Context
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

		Window::gl_context = SDL_GL_CreateContext( Window::companion_window );
		// DISABLE VSYNC, IMPORTANT FOR MAX VIVE FRAMERATE
		SDL_GL_SetSwapInterval( 0 );
		if( Window::gl_context == NULL )
		{
			return false;
		}


		return true;
	}

	void init_gl()
	{
		// Setup the companion window data
		// x, y,	u, v
		// x and y are in normalised device coordinates
		// each side should take up half the screen
		GLfloat verts[] =
		{
			// Left side
			-1.0, -1.0f, 0,		0.0, 0.0,
			0.0, -1.0, 0,		1.0, 0.0,
			-1.0, 1.0, 0,		0.0, 1.0,
			0.0, 1.0, 0,		1.0, 1.0,

			// Right side
			0.0, -1.0, 0,		0.0, 0.0,
			1.0, -1.0, 0,		1.0, 0.0,
			0.0, 1.0, 0,		0.0, 1.0,
			1.0, 1.0, 0,		1.0, 1.0
		};

		GLushort indices[] = { 0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6 };

		glGenVertexArrays( 1, &window_vao );
		glBindVertexArray( window_vao );

		glGenBuffers( 1, &window_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, window_vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( verts ), verts, GL_STATIC_DRAW );

		glGenBuffers( 1, &window_ebo );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, window_ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		//GLint posAttrib = texture_shader.getAttributeLocation( "vPosition" );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), 0 );

		//GLint uvAttrib = texture_shader.getAttributeLocation( "vUV" );
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), (void*)(3 * sizeof( GLfloat )) );
	}

	void shutdown()
	{
		if( companion_window )
		{
			SDL_DestroyWindow( companion_window );
			companion_window = nullptr;
		}
	}

	void draw_left_side( GLuint texture )
	{
		glBindTexture( GL_TEXTURE_2D, texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
	}
	void draw_right_side( GLuint texture )
	{
		glBindTexture( GL_TEXTURE_2D, texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)(12) );
	}

	void present()
	{
		SDL_GL_SwapWindow( Window::companion_window );

	}
}