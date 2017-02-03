#include "controller.h"

Controller::Controller() :
	initialised_(false),
	hmd_(nullptr),
	index_(12)
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

		// Update the current controller state and pose
		hmd_->GetControllerState( index_, &state_, sizeof( state_ ) );

		/* // The pos given from this function is accurate at the time the state was generated, i think...
		// It definately feels laggier than the one passed from WaitGetPoses()
		hmd_->GetControllerStateWithPose(
			vr::TrackingUniverseOrigin::TrackingUniverseStanding,
			index_,
			&state_, sizeof( state_ ),
			&pose_ ); */
	}
}

void Controller::handleEvent( vr::VREvent_t event )
{
	switch( event.eventType )
	{
	case vr::EVREventType::VREvent_ButtonTouch:
		//printf( "touch button %d\n", event.data.controller.button );
	break;
	case vr::EVREventType::VREvent_ButtonUntouch:
		//printf( "untouch button %d\n", event.data.controller.button );
	break;
	case vr::EVREventType::VREvent_ButtonPress: break;
	case vr::EVREventType::VREvent_ButtonUnpress: break;
	default: break;
	}
}

glm::vec2 Controller::GetAxis( vr::EVRButtonId button ) const
{
	glm::vec2 axis_value;
	unsigned axisId = (unsigned)button - (unsigned)vr::k_EButton_Axis0;
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

glm::vec2 Controller::GetPrevAxis( vr::EVRButtonId button ) const
{
	glm::vec2 axis_value;
	unsigned axis = (unsigned)button - (unsigned)vr::k_EButton_Axis0;
	switch( axis )
	{
	case 0: axis_value = glm::vec2( prev_state_.rAxis[0].x, prev_state_.rAxis[0].y ); break;
	case 1: axis_value = glm::vec2( prev_state_.rAxis[1].x, prev_state_.rAxis[1].y ); break;
	case 2: axis_value = glm::vec2( prev_state_.rAxis[2].x, prev_state_.rAxis[2].y ); break;
	case 3: axis_value = glm::vec2( prev_state_.rAxis[3].x, prev_state_.rAxis[3].y ); break;
	case 4: axis_value = glm::vec2( prev_state_.rAxis[4].x, prev_state_.rAxis[4].y ); break;
	}
	return axis_value;
}

glm::vec2 Controller::GetAxisDelta( vr::EVRButtonId button ) const
{
	glm::vec2 current = GetAxis( button );
	glm::vec2 prev = GetPrevAxis( button );

	return current - prev;
}