#pragma once

#include <openvr.h>
#include <glm.hpp>

glm::mat4 ConvertHMDMat4ToGLMMat4( const vr::HmdMatrix44_t& mat );

glm::mat4 ConvertHMDMat3ToGLMMat4( const vr::HmdMatrix34_t& mat );