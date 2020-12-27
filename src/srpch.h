#pragma once

#ifndef VMA_IMPLEMENTATION

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

#include <nlohmann/json.hpp>


#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/waitgroup.h"
#include "marl/task.h"
#include "marl/ticket.h"
#include "marl/mutex.h"
#include "marl/blockingcall.h"

#include "cs_cow_guarded.h"
#include "cs_deferred_guarded.h"
#include "cs_lr_guarded.h"
#include "cs_ordered_guarded.h"
#include "cs_shared_guarded.h"
#include "cs_rcu_guarded.h"


// stl
#include <iostream>
#include <memory>
#include <fstream>

#include <optional>

#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <assert.h>
#include <numeric>


#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

#include <filesystem>


#ifdef SR_PLATFORM_WINDOWS
#define NOMINMAX
#include <windows.h>
#endif

// internal

#include "Sunrise/Sunrise/core/core.h"
#include "Sunrise/Sunrise/core/environment.h"

// for IntelliSense
namespace sunrise {

}

#endif
