#include "points_mesh.h"
#include <fstream>

PointsMesh::PointsMesh() :
	vao_( 0 ),
	vbo_( 0 ),
	num_verts_( 0 )
{

}

void PointsMesh::init( Shader* shader, std::string filename )
{
	glGenVertexArrays( 1, &vao_ );
	glGenBuffers( 1, &vbo_ );

	name_ = filename;

	shader->bind();
	bind();

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	GLint posAttrib = shader->getAttributeLocation( "vPosition" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	offset += sizeof( GLfloat ) * 3;
	GLint colAttrib = shader->getAttributeLocation( "vColour" );
	glEnableVertexAttribArray( colAttrib );
	glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	std::ifstream file( filename );
	if( file )
	{
		printf( "Loaded %s\n", filename.c_str() );
		float x, y, z;
		float confidence, intensity;

		while( file )
		{
			file >> x >> y >> z;// >> confidence >> intensity;
			data_.push_back( x );
			data_.push_back( y );
			data_.push_back( z );
			data_.push_back( 1 );
			data_.push_back( 1 );
			data_.push_back( 1 );
		}
	}
	else
	{
		printf( "Could not load %s\n", filename.c_str() );
		// fill with some random points
		for( int i = 0; i < 500; i++ )
		{
			data_.push_back( (rand() / (float)RAND_MAX) - 0.5f );
			data_.push_back( (rand() / (float)RAND_MAX) - 0.5f );
			data_.push_back( (rand() / (float)RAND_MAX) - 0.5f );
			data_.push_back( 1 );
			data_.push_back( 1 );
			data_.push_back( 1 );
		}
	}

	// Send the verticies
	glBufferData( GL_ARRAY_BUFFER, sizeof( data_[0] ) * data_.size(), data_.data(), GL_STATIC_DRAW );
	num_verts_ = data_.size()/6;

	glBindVertexArray( 0 );
}

void PointsMesh::bind()
{
	glBindVertexArray( vao_ );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_ );
}

void PointsMesh::draw()
{
	glDrawArrays( GL_POINTS, 0, num_verts_ );
}

