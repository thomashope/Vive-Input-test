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

	bool GetButton( vr::EVRButtonId buttonId ) { return state_.ulButtonPressed & vr::ButtonMaskFromId( buttonId ) != 0; }

	glm::vec2 GetAxis( vr::EVRButtonId buttonId = vr::k_EButton_SteamVR_Touchpad );
	glm::vec2 GetAxisDelta( vr::EVRButtonId buttonId = vr::k_EButton_SteamVR_Touchpad );

	glm::vec2 GetTouchpadDelta() { return touchpad_ - prev_touchpad_; }

	// TODO
	void update();
	void handleEvent( vr::VREvent_t event );

	vr::VRControllerState_t* state() { return &state_; }
	vr::VRControllerState_t* prevState() { return &prev_state_; }
	vr::TrackedDeviceIndex_t index() { return index_; }
	bool initialised() { return initialised_; }
	bool finger_on_touchpad() { return finger_on_touchpad_; }

protected:
	bool initialised_;

	vr::IVRSystem* hmd_;
	vr::TrackedDeviceIndex_t index_;

	vr::VRControllerState_t state_;
	vr::VRControllerState_t prev_state_;

	bool finger_on_touchpad_;
	glm::vec2 touchpad_;
	glm::vec2 prev_touchpad_;
};