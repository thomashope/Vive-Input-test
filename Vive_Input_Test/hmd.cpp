#include "hmd.h"

#include <SDL.h>
#include "helpers.h"

HMD::HMD() :
	hmd_(nullptr),
	reccomended_render_target_width_(0),
	reccomended_render_target_height_(0),
	near_plane_(0.1f),
	far_plane_(100.0f)
{

}

HMD::~HMD()
{

}

bool HMD::init( vr::EVRApplicationType type )
{
	vr::EVRInitError init_error = vr::VRInitError_None;
	hmd_ = vr::VR_Init( &init_error, vr::VRApplication_Scene );
	if( init_error != vr::VRInitError_None )
	{
		hmd_ = nullptr;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( init_error ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}

	hmd_->GetRecommendedRenderTargetSize( &reccomended_render_target_width_, &reccomended_render_target_height_ );

	return true;
}

std::string HMD::getTrackedDeviceString(
	vr::TrackedDeviceIndex_t unDevice,
	vr::TrackedDeviceProperty prop,
	vr::TrackedPropertyError *peError )
{
	uint32_t unRequiredBufferLen = hmd_->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = hmd_->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

glm::mat4 HMD::projectionMartix( vr::Hmd_Eye eye )
{
	// If the hmd failed to initialise return an empty matrix
	if( !hmd_ )	return glm::mat4();

	vr::HmdMatrix44_t matrix = hmd_->GetProjectionMatrix( eye, near_plane_, far_plane_ );

	return ConvertHMDMat4ToGLMMat4( matrix );
}

glm::mat4 HMD::eyePoseMatrix( vr::Hmd_Eye eye )
{
	// If the hmd failed to initialise return an empty matrix
	if( !hmd_ )	return glm::mat4();

	vr::HmdMatrix34_t matrix = hmd_->GetEyeToHeadTransform( eye );

	return ConvertHMDMat3ToGLMMat4( matrix );
}