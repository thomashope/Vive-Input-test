#include "helpers.h"
#include <fstream>

glm::mat4 ConvertHMDMat4ToGLMMat4( const vr::HmdMatrix44_t& mat )
{
	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

glm::mat4 ConvertHMDMat3ToGLMMat4( const vr::HmdMatrix34_t& mat )
{
	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
		mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
		mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
		mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
	);
}

std::string ReadFileToString( std::string filepath )
{
	std::ifstream f( filepath );
	std::string str;

	// Check we fould the file
	if( !f ) {
		printf( "Could not open %s", filepath.c_str() );
		return std::string( "" );
	}

	// allocate the size of the string upfront
	f.seekg( 0, std::ios::end );
	str.reserve( f.tellg() );
	f.seekg( 0, std::ios::beg );

	str.assign( (std::istreambuf_iterator<char>( f )),
		std::istreambuf_iterator<char>() );

	return str;
}