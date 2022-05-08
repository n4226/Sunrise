#pragma once

#include "srpch.h"
#include "Sunrise/core/Window.h"
#include "Sunrise/memory/IndexAllocator.h"
#include "../generalAbstractions/Buffer.h"



namespace sunrise::gfx {

	class Renderer;
	/// <summary>
	/// linked to a single renderer
	/// used to draw debug elements such as lines and vectors (through lines)
	/// data is cleared at end of every frame so lines must be redrawn every frame
	/// </summary>
	class DebugDrawer
	{
	public:
		struct Config
		{
			float lineWidth = 0.1;
		};

		struct LineData
		{
			glm::vec3 p0, p1;
			glm::vec4 color;

			static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();

			static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();
		};

		DebugDrawer(Renderer* renderer,Config config);
		~DebugDrawer();


		void drawLine(glm::vec3 from, glm::vec3 to, glm::vec4 color = { 0.9,0.1,0.1,1 });
		void drawVector(glm::vec3 vector, glm::vec3 atPoint, float length = 0.5f, glm::vec4 color = { 0.9,0.1,0.1,1 });
		
		void drawBox();
		void drawPolygon();

		/// <summary>
		///	for use by the gpu stage responsible for drawing these lines
		/// </summary>
		/// <returns></returns>
		const std::vector<LineData>& getLines();
		const std::unordered_map<Window*, std::vector<Buffer*>>& getBuffers();


	protected:
		friend Renderer;
		/// <summary>
		/// is normally done once per frame by renderer
		/// </summary>
		void flushData();

		void sendToBuffer(Window* window);

		std::vector<LineData> lines;

		std::unordered_map<Window*,std::vector<Buffer*>> linesBuffers;
		//std::unordered_map < Window*, std::vector<IndexAllocator*>> allocators;

		Config config;
		Renderer* renderer;
	};

}
