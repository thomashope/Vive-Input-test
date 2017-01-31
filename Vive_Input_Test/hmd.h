#pragma once

#include <openvr.h>
#include <string>
#include <glm.hpp>

class HMD
{
public:
	HMD();
	~HMD();

	bool init( vr::EVRApplicationType type );

	vr::IVRSystem* get() { return hmd_; };
	bool isValid() { return hmd_ != nullptr; }

	// forward functions to HMD ptr
	std::string getTrackedDeviceString( vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL );
	
	// setters
	void setNearPlane( float dist ) { near_plane_ = dist; }
	void setFarPlane( float dist ) { far_plane_ = dist;  }

	// getters
	uint32_t reccomendedRenderTargetWidth() { return reccomended_render_target_width_; }
	uint32_t reccomendedRenderTargetHeight() { return reccomended_render_target_height_; }
	glm::mat4 projectionMartix( vr::Hmd_Eye eye );
	glm::mat4 eyePoseMatrix( vr::Hmd_Eye eye );

protected:
	vr::IVRSystem* hmd_;
	uint32_t reccomended_render_target_width_;
	uint32_t reccomended_render_target_height_;

	float near_plane_;
	float far_plane_;
};