#pragma once

#include "hmd.h"
#include "helpers.h"

// With help from: https://github.com/zecbmo/ViveSkyrim/blob/master/Source
// Because the documentation is terrible

class Controller
{
public:
	Controller();
	~Controller();

	void init( vr::TrackedDeviceIndex_t index, vr::IVRSystem* hmd );
	void update();
	void handleEvent( vr::VREvent_t event );

	// Setters
	void setPose( vr::TrackedDevicePose_t pose ) { pose_ = pose; }

	// Getters
	bool isButtonDown( vr::EVRButtonId button ) const { return (state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0; }
	bool isButtonPressed( vr::EVRButtonId button ) const { return ((state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0) && ((prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button )) == 0);  }
	bool isButtonReleased( vr::EVRButtonId button ) const { return ((state_.ulButtonPressed & vr::ButtonMaskFromId( button )) == 0) && ((prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button )) != 0); }

	glm::vec2 GetAxis( vr::EVRButtonId button ) const;
	glm::vec2 GetPrevAxis( vr::EVRButtonId button ) const;
	glm::vec2 GetAxisDelta( vr::EVRButtonId button ) const;

	// Getters: pose attributes
	bool isPoseValid() const { return pose_.bPoseIsValid; }
	glm::mat4 deviceToAbsoluteTracking() const { return ConvertHMDMat3ToGLMMat4( pose_.mDeviceToAbsoluteTracking ); }
	glm::vec3 velocity() const { return glm::vec3( pose_.vVelocity.v[0], pose_.vVelocity.v[1], pose_.vVelocity.v[2] ); }
	glm::vec3 angularVelocity() const { return glm::vec3( pose_.vAngularVelocity.v[0], pose_.vAngularVelocity.v[1], pose_.vAngularVelocity.v[2] ); }
	vr::ETrackingResult trackingResult() const { return pose_.eTrackingResult; }

	// Getters: members
	bool initialised() const { return initialised_; }
	vr::TrackedDeviceIndex_t index() const { return index_; }
	vr::VRControllerState_t state() const { return state_; }
	vr::VRControllerState_t prevState() const { return prev_state_; }
	vr::TrackedDevicePose_t pose() const { return pose_; }

protected:
	bool initialised_;

	vr::IVRSystem* hmd_;
	vr::TrackedDeviceIndex_t index_;

	// Stores button and axis information
	vr::VRControllerState_t state_;
	vr::VRControllerState_t prev_state_;

	// Stores location and orientation information
	vr::TrackedDevicePose_t pose_;
};