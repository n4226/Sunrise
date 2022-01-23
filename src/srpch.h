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
#include <iomanip>

#include "stb_image.h"

//#include "marl/defer.h"
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

#include "../vendor/imgui/imgui.h"

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

#include <variant>

#include <assert.h>
#include <numeric>


#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

#include <filesystem>

#include <random>

#ifdef SR_PLATFORM_WINDOWS
#define NOMINMAX

// stops windows.h including winsock.h - see: https://stackoverflow.com/questions/1372480/c-redefinition-header-files-winsock2-h
//#define _WINSOCKAPI_   
//#include <windows.h>
#endif

// internal

#include "Sunrise/Sunrise/core/core.h"
#include "Sunrise/Sunrise/core/environment.h"

//pasted in imgui dependancies


// for IntelliSense
namespace sunrise {

}

#endif
