#include "srpch.h"
#include "DebugDrawer.h"
#include "../generalAbstractions/AttributeHelpers.h"
#include "Renderer.h"
#include "Sunrise/core/Window.h"
#include "Sunrise/core/Application.h"

namespace sunrise::gfx {




	void DebugDrawer::drawLine(glm::vec3 from, glm::vec3 to, glm::vec4 color /*= { 0.2,0.8,0.1,1 }*/)
	{
		lines.push_back({ from,to,color });
	}

	void DebugDrawer::drawVector(glm::vec3 vector, glm::vec3 atPoint, float length, glm::vec4 color /*= { 0.2,0.8,0.1,1 }*/)
	{
		drawLine(atPoint, atPoint + glm::normalize(vector) * length,color);
	}

	void DebugDrawer::flushData()
	{
		lines.clear();
	}

	const std::vector<sunrise::gfx::DebugDrawer::LineData>& DebugDrawer::getLines()
	{
		return lines;
	}

	DebugDrawer::DebugDrawer(Renderer* renderer, Config config)
		: renderer(renderer), config(config)
	{

	}

	void DebugDrawer::sendToBuffer(Window* window)
	{
	const auto maxLines = 1000;
	const auto lineBuffStride = sizeof(glm::vec3) * 2;
		if (linesBuffers.size() == 0) {
			for (auto win : renderer->app.windows) {
				linesBuffers[win] = {};
				linesBuffers[win].resize(win->numSwapImages());

				allocators[win] = {};
				allocators[win].resize(win->numSwapImages());
			}
		}


		auto& buffsForWindow = linesBuffers.at(window);
		auto& allocsForWindow = allocators.at(window);

		//know that each line takes up exactlyu sizeof(glm::vec3) * 2 in the buffer
		if (buffsForWindow[window->currentSurfaceIndex] == nullptr) {

			BufferCreationOptions options =
			{ ResourceStorageType::cpu,{ vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc }, vk::SharingMode::eConcurrent,
				{ renderer->queueFamilyIndices.graphicsFamily.value(), renderer->queueFamilyIndices.resourceTransferFamily.value() } };

			buffsForWindow[window->currentSurfaceIndex] = new Buffer(renderer->device, renderer->allocator, maxLines * sizeof(lineBuffStride), options);
			allocsForWindow[window->currentSurfaceIndex] = new IndexAllocator(maxLines * lineBuffStride, lineBuffStride);
		}
		//todo: delete old lines once their drawable has been released
		//although this is sort of done sine bufer cleared on new draw of same surface
		
		auto currentBuff = buffsForWindow[window->currentSurfaceIndex];
		auto currentAlloc = allocsForWindow[window->currentSurfaceIndex];

		//actually dont need allocator since each frame reset so buffer will jsut be populated with values from begigigng

		SR_CORE_ASSERT(lines.size() < maxLines);
		for (int i = 0; i < lines.size() ; i++) {
			//asuming p0 and p1 are next to eachother in memory
			currentBuff->tempMapAndWrite(&lines[i], lineBuffStride * i, lineBuffStride);
		}

	}

	const std::unordered_map<Window*, std::vector<Buffer*>>& DebugDrawer::getBuffers()
	{
		return linesBuffers;
	}

	std::array<VkVertexInputBindingDescription, 1> DebugDrawer::LineData::getBindingDescription()
	{
		return { makeVertBinding(0, sizeof(glm::vec3)) };
	}
	std::array<VkVertexInputAttributeDescription, 1> DebugDrawer::LineData::getAttributeDescriptions()
	{
		return { makeVertAttribute(0, 0, VertexAttributeFormat::vec3, 0) };
	}

}