#pragma once

#include "hmd.h"

// With help from: https://github.com/zecbmo/ViveSkyrim/blob/master/Source
// Because the documentation is terrible

class Controller
{
public:
	Controller();
	~Controller();

	void init( vr::TrackedDeviceIndex_t index, vr::IVRSystem* hmd );

	bool isButtonDown( vr::EVRButtonId button ) { return state_.ulButtonPressed & vr::ButtonMaskFromId( button ) != 0; }
	bool isButtonPressed( vr::EVRButtonId button ) { return (state_.ulButtonPressed & vr::ButtonMaskFromId( button ) != 0) && (prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button ) == 0);  }
	bool isButtonReleased( vr::EVRButtonId button ) { return (state_.ulButtonPressed & vr::ButtonMaskFromId( button ) == 0) && (prev_state_.ulButtonPressed & vr::ButtonMaskFromId( button ) != 0); }

	glm::vec2 GetAxis( vr::EVRButtonId button );
	glm::vec2 GetPrevAxis( vr::EVRButtonId button );
	glm::vec2 GetAxisDelta( vr::EVRButtonId button );

	// TODO
	void update();
	void handleEvent( vr::VREvent_t event );

	vr::VRControllerState_t* state() { return &state_; }
	vr::VRControllerState_t* prevState() { return &prev_state_; }
	vr::TrackedDeviceIndex_t index() { return index_; }
	bool initialised() { return initialised_; }

protected:
	bool initialised_;

	vr::IVRSystem* hmd_;
	vr::TrackedDeviceIndex_t index_;

	vr::VRControllerState_t state_;
	vr::VRControllerState_t prev_state_;
};