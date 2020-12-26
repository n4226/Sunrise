#pragma once


//External Dependancies

#define _USE_MATH_DEFINES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/vk_mem_alloc.h"

#include <vulkan/vulkan.hpp>

// stl
#include <iostream>
#include <memory>
#include <fstream>


#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <assert.h>
#include <numeric>


#ifdef SR_PLATFORM_WINDOWS
#define NOMINMAX
#include <windows.h>
#endif

// internal

#include "Sunrise/Sunrise/core/core.h"

namespace sunrise {

}