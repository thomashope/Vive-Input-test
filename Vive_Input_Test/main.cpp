#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <gtc/type_ptr.hpp>
#include <SDL_opengl.h>
#include <openvr.h>

#include <cstdio>
#include <string>
#include <vector>

#include "shader.h"
#include "helpers.h"
#include "hmd.h"
#include "controller.h"
#include "dear_imgui/imgui.h"
#include "points_mesh.h"
#include "window.h"

// This is a tech test of loading up all the OpenVR things and putting something on the HMD
// Tested on Windows 10 with Visual Studio 2013 community and an Oculus DK2.
// Requires Oculus Home and Steam VR be installed
// Depends on SDL2, GLEW, GLM, and OpenVR
// 
// Oculus Home will try to convince you the DK2 is old and unsupported but it still works as of 2016-12-06
//
// This is based on the code from the hellovr_opengl sample included with OpenVR

/* Global variables */

Shader colour_shader;
GLint colour_matrix_location = -1;

// OpenGL 
GLuint scene_vao = 0;	// Vertex attribute object, stores the vertex layout
GLuint scene_vbo = 0;	// Vertex buffer object, stores the vertex data

int tracked_controller_count;			// Number of controllers
int tracked_controller_vertex_count;
GLuint tracked_controller_vao = 0;
GLuint tracked_controller_vbo = 0;

GLuint virtual_screen_vao = 0;
GLuint virtual_screen_vbo = 0;
GLuint virtual_screen_ebo = 0;

struct FrameBufferDesc
{
	GLuint depth_buffer;
	GLuint render_texture;
	GLuint render_frame_buffer;
	GLuint resolve_texture;
	GLuint resolve_frame_buffer;
} left_eye_desc, right_eye_desc, gui_buffer_desc;

// OpenVR
HMD hmd;
uint32_t hmd_render_target_width;
uint32_t hmd_render_target_height;
glm::mat4 left_eye_projection = glm::mat4( 1.0f );
glm::mat4 left_eye_to_pose = glm::mat4( 1.0f );
glm::mat4 right_eye_projection = glm::mat4( 1.0f );
glm::mat4 right_eye_to_pose = glm::mat4( 1.0f );

vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];
glm::mat4 mat4_device_pose[vr::k_unMaxTrackedDeviceCount];
std::string pose_classes_string;							// what classes we saw poses for this frame
char dev_class_char[vr::k_unMaxTrackedDeviceCount];			// for each device, a character representing its class
int valid_pose_count;
glm::mat4 hmd_pose_matrix;

Controller left_controller;
Controller right_controller;

PointsMesh points_mesh;

/* Functions */

// Usefull for getting information about the current hardware setup
std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

bool CreateSimpleFrameBuffer( int width, int height, FrameBufferDesc& desc )
{
	// frame buffer
	glGenFramebuffers( 1, &desc.render_frame_buffer );
	glBindFramebuffer( GL_FRAMEBUFFER, desc.render_frame_buffer );

	// depth component
	glGenRenderbuffers( 1, &desc.depth_buffer );
	glBindRenderbuffer( GL_RENDERBUFFER, desc.depth_buffer );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, desc.depth_buffer, 0 );

	// texture component
	glGenTextures( 1, &desc.render_texture );
	glBindTexture( GL_TEXTURE_2D, desc.render_texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desc.render_texture, 0 );

	// Check everything went ok
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		printf( "Error creating simple frame buffer!\n" );
		return false;
	}

	// Unbind any bound frame buffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	return true;
}

// Create a frame buffer for use with the HMD
// Fills in the Frame Buffer Description 
bool CreateResolveFrameBuffer( int width, int height, FrameBufferDesc& desc )
{
	// render frame buffer
	glGenFramebuffers( 1, &desc.render_frame_buffer );
	glBindFramebuffer( GL_FRAMEBUFFER, desc.render_frame_buffer );

	// depth
	glGenRenderbuffers( 1, &desc.depth_buffer );
	glBindRenderbuffer( GL_RENDERBUFFER, desc.depth_buffer );
	glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, desc.depth_buffer );

	// texture
	glGenTextures( 1, &desc.render_texture );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, desc.render_texture );
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, desc.render_texture, 0 );

	// resolve frame buffer
	glGenFramebuffers( 1, &desc.resolve_frame_buffer );
	glBindFramebuffer( GL_FRAMEBUFFER, desc.resolve_frame_buffer );

	// resolve texture
	glGenTextures( 1, &desc.resolve_texture );
	glBindTexture( GL_TEXTURE_2D, desc.resolve_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desc.resolve_texture, 0 );

	// Check everything went ok
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		printf( "Error creating resolve frame buffer!\n" );
		return false;
	}

	// Unbind any bound frame buffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	return true;
}

void RenderScene( vr::Hmd_Eye eye )
{
	glm::mat4 view_proj_matrix = glm::mat4( 1.0 );

	glEnable( GL_MULTISAMPLE );

	if( eye == vr::Eye_Left )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, left_eye_desc.render_frame_buffer );
		glViewport( 0, 0, hmd_render_target_width, hmd_render_target_height );

		// Left eye
		view_proj_matrix =
			left_eye_projection
			* left_eye_to_pose
			* hmd_pose_matrix;
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, right_eye_desc.render_frame_buffer );
		glViewport( 0, 0, hmd_render_target_width, hmd_render_target_height );
		// Right Eye
		view_proj_matrix =
			right_eye_projection
			* right_eye_to_pose
			* hmd_pose_matrix;
	}

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// draw the scene
	colour_shader.bind();
	glBindVertexArray( scene_vao );
	glUniformMatrix4fv( colour_matrix_location, 1, GL_FALSE, glm::value_ptr( view_proj_matrix ) );
	glDrawArrays( GL_TRIANGLES, 0, 12 );

	points_mesh.bind();
	glm::mat4 bunny_mat = glm::translate( glm::vec3( 0, 1, -1 ) );
	bunny_mat *= glm::scale( glm::vec3( 5, 5, 5 ) );
	glUniformMatrix4fv( colour_matrix_location, 1, GL_FALSE, glm::value_ptr( view_proj_matrix * bunny_mat ) );
	points_mesh.draw();

	// draw the controller axis lines
	colour_shader.bind();
	glBindVertexArray( tracked_controller_vao );
	glUniformMatrix4fv( colour_matrix_location, 1, GL_FALSE, glm::value_ptr( view_proj_matrix ) );
	glDrawArrays( GL_LINES, 0, tracked_controller_vertex_count );

	// Render ImGui
	Window::shader.bind();
	glBindVertexArray( virtual_screen_vao );
	
	glm::mat4 controller_mat = left_controller.deviceToAbsoluteTracking();
	controller_mat *= glm::rotate( 100.0f, glm::vec3( 1, 0, 0 ) );
	controller_mat *= glm::translate( glm::vec3( -0.25, 0, 0 ) );
	controller_mat *= glm::scale( glm::vec3( 0.5f, 0.5f, 0.5f ) );

	glUniformMatrix4fv( colour_matrix_location, 1, GL_FALSE, glm::value_ptr( view_proj_matrix * controller_mat ) );
	glBindTexture( GL_TEXTURE_2D, gui_buffer_desc.render_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );

	glDisable( GL_MULTISAMPLE );
}

void InitControllers()
{
	// Init the left controller if it hasn't been initialised already
	if( !left_controller.initialised() )
	{
		vr::TrackedDeviceIndex_t left_index = hmd.get()->GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole::TrackedControllerRole_LeftHand );
		if( left_index != -1 )
		{
			left_controller.init( left_index, hmd.get() );
		}
	}

	// Init the right controller if it hasn't been initialised already
	if( !right_controller.initialised() )
	{
		vr::TrackedDeviceIndex_t right_index = hmd.get()->GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole::TrackedControllerRole_RightHand );
		if( right_index != -1 )
		{
			right_controller.init( right_index, hmd.get() );
		}
	}
}

void UpdateControllerAxes()
{
	std::vector<GLfloat> vertex_data;
	tracked_controller_count = 0;
	tracked_controller_vertex_count = 0;

	// Don't draw controllers if somebody else has input focus
	if( hmd.get()->IsInputFocusCapturedByAnotherProcess() )
		return;

	Controller* controllers[] = { &left_controller, &right_controller };

	for( int i = 0; i < sizeof(controllers)/sizeof(void*); i++ )
	{
		Controller* ctrl = controllers[i];

		const glm::mat4 mat = ctrl->deviceToAbsoluteTracking();
		glm::vec4 center = mat * glm::vec4( 0, 0, 0, 1 );

		for( int i = 0; i < 3; ++i )
		{
			glm::vec3 colour( 0, 0, 0 );
			glm::vec4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;
			colour[i] = 1.0;
			point = mat * point;

			vertex_data.push_back( center.x );
			vertex_data.push_back( center.y );
			vertex_data.push_back( center.z );

			vertex_data.push_back( colour.x );
			vertex_data.push_back( colour.y );
			vertex_data.push_back( colour.z );

			vertex_data.push_back( point.x );
			vertex_data.push_back( point.y );
			vertex_data.push_back( point.z );

			vertex_data.push_back( colour.x );
			vertex_data.push_back( colour.y );
			vertex_data.push_back( colour.z );

			tracked_controller_vertex_count += 2;
		}

		glm::vec4 start = mat * glm::vec4( 0, 0, -0.02f, 1 );
		glm::vec4 end = mat * glm::vec4( 0, 0, -15.0f, 1 );
		glm::vec3 colour( .92f, .92f, .71f );

		vertex_data.push_back( start.x ); vertex_data.push_back( start.y ); vertex_data.push_back( start.z );
		vertex_data.push_back( colour.x ); vertex_data.push_back( colour.y ); vertex_data.push_back( colour.z );

		vertex_data.push_back( end.x ); vertex_data.push_back( end.y ); vertex_data.push_back( end.z );
		vertex_data.push_back( colour.x ); vertex_data.push_back( colour.y ); vertex_data.push_back( colour.z );

		tracked_controller_vertex_count += 2;
	}

	// Setup the VAO the first time through
	if( tracked_controller_vao == 0 )
	{
		colour_shader.bind();
		glGenVertexArrays( 1, &tracked_controller_vao );
		glBindVertexArray( tracked_controller_vao );

		glGenBuffers( 1, &tracked_controller_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, tracked_controller_vbo );

		GLuint stride = 2 * 3 * sizeof( GLfloat );
		GLuint offset = 0;

		GLint posAttrib = colour_shader.getAttributeLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		offset += sizeof( GLfloat ) * 3;
		GLint colAttrib = colour_shader.getAttributeLocation( "vColour" );
		glEnableVertexAttribArray( colAttrib );
		glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, tracked_controller_vbo );
	if( vertex_data.size() > 0 )
	{
		glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * vertex_data.size(), vertex_data.data(), GL_STREAM_DRAW );
	}
}

void UpdatePoses()
{
	if( !hmd.isValid() )
		return;

	vr::VRCompositor()->WaitGetPoses( tracked_device_pose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	valid_pose_count = 0;
	pose_classes_string = "";
	for( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if( tracked_device_pose[nDevice].bPoseIsValid )
		{
			valid_pose_count++;
			mat4_device_pose[nDevice] = ConvertHMDMat3ToGLMMat4( tracked_device_pose[nDevice].mDeviceToAbsoluteTracking );
			
			if( left_controller.index() == nDevice )
				left_controller.setPose( tracked_device_pose[nDevice] );

			if( right_controller.index() == nDevice )
				right_controller.setPose( tracked_device_pose[nDevice] );
			
			if( dev_class_char[nDevice] == 0 )
			{
				switch( hmd.get()->GetTrackedDeviceClass( nDevice ) )
				{
				case vr::TrackedDeviceClass_Controller:        dev_class_char[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               dev_class_char[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           dev_class_char[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    dev_class_char[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: dev_class_char[nDevice] = 'T'; break;
				default:                                       dev_class_char[nDevice] = '?'; break;
				}
			}
			pose_classes_string += dev_class_char[nDevice];
		}
	}

	if( tracked_device_pose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		hmd_pose_matrix = mat4_device_pose[vr::k_unTrackedDeviceIndex_Hmd];
		hmd_pose_matrix = glm::inverse( hmd_pose_matrix );
	}
}

void ProcessVREvent( vr::VREvent_t event )
{
	// Forward the event to the correct device
	if( event.trackedDeviceIndex == left_controller.index() )
	{
		left_controller.handleEvent( event );
		return;
	}
	else if( event.trackedDeviceIndex == right_controller.index() )
	{
		right_controller.handleEvent( event );
		return;
	}



	switch( event.eventType )
	{
	case vr::EVREventType::VREvent_MouseMove:
		printf( "Mouse move: %f %f\n", event.data.mouse.x, event.data.mouse.y );
	break;
	case vr::EVREventType::VREvent_ButtonTouch:
		printf( "Button touch: %d\n", event.data.controller.button );
	break;
	case vr::EVREventType::VREvent_ButtonUntouch:
		printf( "Button untouch: %d\n", event.data.controller.button );
	break;
	case vr::EVREventType::VREvent_TouchPadMove:
		printf( "Touchpad move: %f %f\n", event.data.touchPadMove.fValueXFirst, event.data.touchPadMove.fValueYFirst );
	break;
	case vr::EVREventType::VREvent_ButtonPress:
		printf( "Button press: %d\n", event.data.controller.button );
	break;
	default: break;
	}
}

void CreateImGui()
{
	ImGuiIO& imguiio = ImGui::GetIO();

	static float f = 0.0f;
	ImGui::SetWindowFontScale( 3.0f );
	ImGui::Text( "Hello VR!" );
	ImGui::Separator();
	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
	ImGui::Separator();

	glm::vec2 left_touch_pos = left_controller.GetAxis( vr::k_EButton_SteamVR_Touchpad );
	glm::vec2 right_touch_pos = right_controller.GetAxis( vr::k_EButton_SteamVR_Touchpad );

	ImGui::Value( "Left touch x", left_touch_pos.x );
	ImGui::Value( "Left touch y", left_touch_pos.y );
	ImGui::Value( "Left trigger", left_controller.isButtonDown( vr::k_EButton_SteamVR_Trigger ) );
	ImGui::Value( "Right touch x", right_touch_pos.x );
	ImGui::Value( "Right touch y", right_touch_pos.y );
	ImGui::Value( "Right trigger", right_controller.isButtonDown( vr::k_EButton_SteamVR_Trigger ) );
	ImGui::Separator();
	ImGui::Text( "Loaded model: %s", points_mesh.name_.c_str() );
	ImGui::Value( "Total Points", points_mesh.data_.size() / 6 );
	ImGui::Value( "Total Bytes", sizeof(points_mesh.data_[0]) * points_mesh.data_.size() );

	ImGui::SetMouseCursor( ImGuiMouseCursor_Arrow );
	imguiio.MouseDrawCursor = true;
}

bool init()
{
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		printf( "Could not init SDL! Error: %s\n", SDL_GetError() );
		return false;
	}

	{
		// We can call these before we load the runtime
		bool is_hmd_present = vr::VR_IsHmdPresent();
		bool is_runtime_installed = vr::VR_IsRuntimeInstalled();
		printf( "Found HMD: %s\n", is_hmd_present ? "yes" : "no" );
		printf( "Found OpenVR runtime: %s\n", is_runtime_installed ? "yes" : "no" );

		if( is_hmd_present == false || is_runtime_installed == false )
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "error", "Something is missing...", NULL );
			return false;
		}
	}

	bool success = hmd.init( vr::VRApplication_Scene );
	if( !success ) return false;

	{
		// Get some more info about our environment
		std::string driver = "No Driver";
		std::string display = "No Display";

		driver = hmd.getTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
		display = hmd.getTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

		printf( "Device: %s\n", display.c_str() );
		printf( "Driver: %s\n", driver.c_str() );
	}

	vr::EVRInitError init_error = vr::VRInitError_None;
	(vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &init_error );

	// Create the window
	if( !Window::init() ) return false;

	// Setup Shaders
	{
		std::string vertex_source = ReadFileToString( "colour_shader.gl_vs" );
		std::string fragment_source = ReadFileToString( "colour_shader.gl_fs" );
		colour_shader.init( "colour", vertex_source.c_str(), fragment_source.c_str() );
		colour_matrix_location = colour_shader.getUniformLocation( "matrix" );
	}

	// Setup scene data
	{
		// Some hardcoded Triangles
		float vertices[] = {
			0.0f, 0.5f, 2.0f, 1.0, 0.0, 1.0,
			0.5f, -0.5f, 2.0f, 1.0, 1.0, 1.0,
			-0.5f, -0.5f, 2.0f, 1.0, 1.0, 1.0,

			0.0f, 0.5f, -2.0f, 1.0, 1.0, 1.0,
			0.5f, -0.5f, -2.0f, 0.0, 1.0, 1.0,
			-0.5f, -0.5f, -2.0f, 1.0, 1.0, 1.0,

			2, 0, 0, 1.0, 1.0, 1.0,
			2, 1, 0, 1.0, 1.0, 0.0,
			2, 1, 1, 1.0, 1.0, 1.0,

			-2, 0, 0, 1.0, 0.0, 0.0,
			-2, 1, 0, 0.0, 1.0, 1.0,
			-2, 1, 1, 1.0, 1.0, 1.0
		};

		// Create a crappy triangle for rendering

		glGenVertexArrays( 1, &scene_vao );
		glBindVertexArray( scene_vao );

		glGenBuffers( 1, &scene_vbo ); // Generate 1 buffer
		glBindBuffer( GL_ARRAY_BUFFER, scene_vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		GLuint stride = 2 * 3 * sizeof( GLfloat );
		GLuint offset = 0;

		GLint posAttrib = colour_shader.getAttributeLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		offset += sizeof( GLfloat ) * 3;
		GLint colAttrib = colour_shader.getAttributeLocation( "vColour" );
		glEnableVertexAttribArray( colAttrib );
		glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		glBindVertexArray( 0 );
	}

	{
		// x, y,	u, v
		// x and y are in normalised device coordinates
		// each side should take up half the screen
		GLfloat verts[] =
		{
			0.0, 0.0f, 0,		0.0, 0.0,
			1.0, 0.0, 0,		1.0, 0.0,
			0.0, 1.0, 0,		0.0, 1.0,
			1.0, 1.0, 0,		1.0, 1.0
		};

		GLushort indices[] = { 0, 1, 3, 0, 3, 2 };

		glGenVertexArrays( 1, &virtual_screen_vao );
		glBindVertexArray( virtual_screen_vao );

		glGenBuffers( 1, &virtual_screen_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, virtual_screen_vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( verts ), verts, GL_STATIC_DRAW );

		glGenBuffers( 1, &virtual_screen_ebo );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, virtual_screen_ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		GLint posAttrib = Window::shader.getAttributeLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), 0 );

		GLint uvAttrib = Window::shader.getAttributeLocation( "vUV" );
		glEnableVertexAttribArray( uvAttrib );
		glVertexAttribPointer( uvAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), (void*)(3 * sizeof( GLfloat )) );
	}

	points_mesh.init( &colour_shader, "bunny_res1.points" );

	// Setup the render targets
	hmd_render_target_width = hmd.reccomendedRenderTargetWidth();
	hmd_render_target_height = hmd.reccomendedRenderTargetHeight();

	CreateSimpleFrameBuffer( 1024, 1024, gui_buffer_desc );
	CreateResolveFrameBuffer( hmd_render_target_width, hmd_render_target_height, left_eye_desc );
	CreateResolveFrameBuffer( hmd_render_target_width, hmd_render_target_height, right_eye_desc );

	// Setup the compositer
	if( !vr::VRCompositor() )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", "Could not initialise compositor", NULL );
		return false;
	}

	// Grab the projection and eye to pos matrices
	// These are fixed, only the pose changes each frame
	left_eye_projection = hmd.projectionMartix( vr::Eye_Left );
	left_eye_to_pose = hmd.eyePoseMatrix( vr::Eye_Left );
	right_eye_projection = hmd.projectionMartix( vr::Eye_Right );
	right_eye_to_pose = hmd.eyePoseMatrix( vr::Eye_Right );

	// Initialize ImGUi
	ImGui::Init( Window::companion_window );

	return true;
}

int main( int argc, char* argv[] )
{
	// Initialise the application
	if( !init() ) return 1;

	// Finally!
	// The application loop
	bool done = false;
	SDL_Event sdl_event;
	vr::VREvent_t vr_event;
	while( !done )
	{
		// Process SDL events
		while( SDL_PollEvent( &sdl_event ) )
		{
			if( sdl_event.type == SDL_QUIT ) done = true;
			else if( sdl_event.type == SDL_KEYDOWN )
			{
				if( sdl_event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
			}
			ImGui::ProcessEvent( &sdl_event );
		}

		// Start the ImGui frame
		ImGui::Frame( Window::companion_window, &hmd, &left_controller);

		InitControllers();

		CreateImGui();

		// Process SteamVR events
		while( hmd.get()->PollNextEvent( &vr_event, sizeof( vr_event ) ) )
		{
			ProcessVREvent( vr_event );
		}

		// Update poses
		UpdatePoses();

		// Update the controllers
		left_controller.update();
		right_controller.update();

		// Update controller models
		UpdateControllerAxes();

		glEnable( GL_DEPTH_TEST );
		glClearColor( 0.0, 0.0, 0.0, 1.0 );

		// render left eye
		RenderScene( vr::Eye_Left );

		// Blit to the resolve buffer
		glBindFramebuffer( GL_READ_FRAMEBUFFER, left_eye_desc.render_frame_buffer );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, left_eye_desc.resolve_frame_buffer );
		glBlitFramebuffer( 0, 0, hmd_render_target_width, hmd_render_target_height, 0, 0, hmd_render_target_width, hmd_render_target_height,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR );

		// render Right eye
		RenderScene( vr::Eye_Right );

		// Blit to the resolve buffer
		glBindFramebuffer( GL_READ_FRAMEBUFFER, right_eye_desc.render_frame_buffer );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, right_eye_desc.resolve_frame_buffer );
		glBlitFramebuffer( 0, 0, hmd_render_target_width, hmd_render_target_height, 0, 0, hmd_render_target_width, hmd_render_target_height,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR );

		// Render the GUI to a buffer
		glBindFramebuffer( GL_FRAMEBUFFER, gui_buffer_desc.render_frame_buffer );
		glViewport( 0, 0, hmd_render_target_width, hmd_render_target_height );
		glClearColor( 0.0, 0.0, 0.0, 0.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable( GL_BLEND );
		glBlendEquation( GL_FUNC_ADD );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		ImGui::Render();
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		// Submit frames to HMD
		if( !hmd.get()->IsInputFocusCapturedByAnotherProcess() )
		{
			// NOTE: to find out what the error codes mean Ctal+F 'enum EVRCompositorError' in 'openvr.h'
			vr::EVRCompositorError submit_error = vr::VRCompositorError_None;

			// Submit left eye
			vr::Texture_t left_eye_texture = { (void*)left_eye_desc.resolve_texture, vr::ETextureType::TextureType_OpenGL, vr::ColorSpace_Gamma };
			submit_error = vr::VRCompositor()->Submit( vr::Eye_Left, &left_eye_texture, NULL );
			if( submit_error != vr::VRCompositorError_None ) printf( "Error in left eye %d\n", submit_error );
			
			// Submit right eye
			vr::Texture_t right_eye_texture = { (void*)right_eye_desc.resolve_texture, vr::ETextureType::TextureType_OpenGL, vr::ColorSpace_Gamma };
			submit_error = vr::VRCompositor()->Submit( vr::Eye_Right, &right_eye_texture, NULL );
			if( submit_error != vr::VRCompositorError_None ) printf( "Error in right eye %d\n", submit_error );

			// Added on advice from comments in IVRCompositor::submit in openvr.h
			glFlush();
		}
		else
		{
			printf( "Another process has focus of the HMD!\n" );
		}

		// Draw the companion window
		glDisable( GL_DEPTH_TEST );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glViewport( 0, 0, Window::companion_width, Window::companion_height );
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		Window::shader.bind();
		glBindVertexArray( Window::window_vao );
		glUniformMatrix4fv( Window::matrix_location, 1, GL_FALSE, glm::value_ptr( glm::mat4() ) );

		Window::draw_left_side( left_eye_desc.resolve_texture );
		Window::draw_right_side( right_eye_desc.resolve_texture );

		Window::present();
	}

	// Shutdown everything
	ImGui::Shutdown();
	vr::VR_Shutdown();
	Window::shutdown();
	SDL_Quit();

	return 0;
}