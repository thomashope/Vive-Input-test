#include "controller.h"

Controller::Controller() :
	initialised_(false),
	hmd_(nullptr),
	index_(12),
	finger_on_touchpad_(false)
{

}

Controller::~Controller()
{

}

void Controller::init( vr::TrackedDeviceIndex_t index, vr::IVRSystem* hmd )
{
	hmd_ = hmd;
	index_ = index;

	initialised_ = true;
	printf( "controller %d initialised!\n", index_ );
}

void Controller::update()
{
	if( initialised_ )
	{
		// Copy the current state into the old state
		prev_state_ = state_;

		// Update the current state
		hmd_->GetControllerState( index_, &state_, sizeof( state_ ) );
	}
}

void Controller::handleEvent( vr::VREvent_t event )
{
	switch( event.eventType )
	{
	case vr::EVREventType::VREvent_ButtonTouch:
		if( event.data.controller.button == vr::k_EButton_SteamVR_Touchpad ) finger_on_touchpad_ = true;
		printf( "touch pad\n" );
	break;
	case vr::EVREventType::VREvent_ButtonUntouch:
		if( event.data.controller.button == vr::k_EButton_SteamVR_Touchpad ) finger_on_touchpad_ = false;
		printf( "untouch pad\n" );
	break;
	case vr::EVREventType::VREvent_TouchPadMove:
		prev_ /* Touch pad move event not being generated????? */ touchpad_ = touchpad_;
		touchpad_ = glm::vec2( event.data.touchPadMove.fValueXRaw, event.data.touchPadMove.fValueYRaw );
		printf( "raw touch: %f %f \n", event.data.touchPadMove.fValueXRaw, event.data.touchPadMove.fValueYRaw );
	default: break;
	}
}

glm::vec2 Controller::GetAxis( vr::EVRButtonId buttonId )
{
	glm::vec2 axis_value;
	unsigned axisId = (unsigned)buttonId - (unsigned)vr::k_EButton_Axis0;
	switch( axisId )
	{
	case 0: axis_value = glm::vec2( state_.rAxis[0].x, state_.rAxis[0].y ); break;
	case 1: axis_value = glm::vec2( state_.rAxis[1].x, state_.rAxis[1].y ); break;
	case 2: axis_value = glm::vec2( state_.rAxis[2].x, state_.rAxis[2].y ); break;
	case 3: axis_value = glm::vec2( state_.rAxis[3].x, state_.rAxis[3].y ); break;
	case 4: axis_value = glm::vec2( state_.rAxis[4].x, state_.rAxis[4].y ); break;
	}
	return axis_value;
}

glm::vec2 Controller::GetAxisDelta( vr::EVRButtonId buttonId )
{
	glm::vec2 current = GetAxis( buttonId );

	glm::vec2 prev;
	unsigned axisId = (unsigned)buttonId - (unsigned)vr::k_EButton_Axis0;
	switch( axisId )
	{
	case 0: prev = glm::vec2( prev_state_.rAxis[0].x, prev_state_.rAxis[0].y ); break;
	case 1: prev = glm::vec2( prev_state_.rAxis[1].x, prev_state_.rAxis[1].y ); break;
	case 2: prev = glm::vec2( prev_state_.rAxis[2].x, prev_state_.rAxis[2].y ); break;
	case 3: prev = glm::vec2( prev_state_.rAxis[3].x, prev_state_.rAxis[3].y ); break;
	case 4: prev = glm::vec2( prev_state_.rAxis[4].x, prev_state_.rAxis[4].y ); break;
	}

	return current - prev;
}