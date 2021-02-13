#pragma once

#include "srpch.h"

//#include "Sunrise/Sunrise/core/Application.h"

namespace sunrise::gfx {

	class GPUStageDispatcher;

	/// <summary>
	/// AN abstract base clase for the GPU Stage System.
	/// 
	/// GPU Stages allow for dependancy graphs to be bult for different types of gpu work which cna trhen be efficieanlty scheduled
	/// - this is how most GPU work is submitted to the Sunrise Engine
	/// 
	/// </summary>
	class SUNRISE_API GPUStage
	{
	public:

		GPUStage();
		virtual ~GPUStage();



	protected:

		

		virtual vk::CommandBuffer* encode() = 0;

	};

}