#pragma once


// For applications

#include "Sunrise/Sunrise/core/core.h"

#include "Sunrise/Sunrise/core/Application.h"

//EntryPoint
//#include "Sunrise/Sunrise/core/EntryPoint.h"
//


///////////////////////////////////////////////////////////// MATH ////////////////////////////// MATH IS FUN ////////////////////////

#include "Sunrise/Sunrise/Math.h"



/////////////////////////////////////////////////////// graphics

#include "Sunrise/Sunrise/graphics/vulkan/generalAbstractions/VkAbstractions.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/Renderer.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/RenderSystem.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/MaterialManager.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPURenderStage.h"
#include "Sunrise/Sunrise/graphics/vulkan/GPU Stages/GPUComputeStage.h"


////////////////////////////////////////// Scene

#include "Sunrise/Sunrise/scene/Scene.h"
#include "Sunrise/Sunrise/scene/Camera.h"

////............................... world

#include "Sunrise/Sunrise/world/WorldScene.h"

/////////////////////////////////////////////////////////////////// Dependency System

#include "Sunrise/Sunrise/dependencyManagment/Dispatcher.h"