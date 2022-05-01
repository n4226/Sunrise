#pragma once

#include "srpch.h"
#include "../GPURenderStage.h"

namespace sunrise::gfx {

	class PostProcessingEffect
	{
	public:

		struct Options
		{
			/// <summary>
			/// does this effect only need the previews effect to finish a pixel before dispatching a pixel in this effect
			/// i.e. does the effect read neighbor pixel values
			/// </summary>
			bool dependancyOnlyPerPixel = true;
		};
	};

	class PostProcessingGPUStage: public GPURenderStage
	{
	public:
		PostProcessingGPUStage(SceneRenderCoordinator* coord,const std::vector<PostProcessingEffect>& effects);

	protected:

		void setup() override;


		void cleanup() override;


		vk::CommandBuffer* encode(RunOptions options) override;

	private:
	};

}
