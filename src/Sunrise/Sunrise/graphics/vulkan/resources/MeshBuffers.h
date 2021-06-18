#pragma once

#include "srpch.h"
#include "../generalAbstractions/Buffer.h"
#include "../../../fileFormats/binary/BinaryMesh.h"

namespace sunrise {
	class Mesh;
}

namespace sunrise::gfx {

	/// <summary>
	/// with the current implimentation, all vertex attributes and indicies are stored in a single buffer.
	/// </summary>
	class SUNRISE_API MeshBuffer {
	public:
		MeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options, Mesh* mesh);
		~MeshBuffer();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="mapandUnmap">if true will call map and unmap before and after writing</param>
		void writeMeshToBuffer(bool mapandUnmap);
		void bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding);
		void bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer);

		Buffer* buffer;
		Mesh* baseMesh;

	private:
	};

	/// <summary>
	/// with the current implimentation, all vertex attributes and indicies are stored in a single buffer.
	/// </summary>
	class SUNRISE_API Basic2DMeshBuffer {
	public:
		Basic2DMeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options, Basic2DMesh* mesh);
		~Basic2DMeshBuffer();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="mapandUnmap">if true will call map and unmap before and after writing</param>
		void writeMeshToBuffer(bool mapandUnmap);
		void bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding);
		void bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer);

		Buffer* buffer;
		Basic2DMesh* baseMesh;

	private:
	};


	class SUNRISE_API BindlessMeshBuffer {
	public:

		struct WriteLocation
		{
			VkDeviceSize offset;
			VkDeviceSize size;
		};

		struct WriteTransactionReceipt
		{
			std::array<WriteLocation, 5> vertexLocations;
			WriteLocation indexLocation;
		};

		BindlessMeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options, VkDeviceSize vCount, VkDeviceSize indexCount);
		~BindlessMeshBuffer();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="mapandUnmap">if true will call map and unmap before and after writing</param>
		void writeMeshToBuffer(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, Mesh* mesh, bool mapandUnmap);
		WriteTransactionReceipt genrateWriteReceipt(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, Mesh* mesh);

		void writeMeshToBuffer(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, BinaryMeshSeirilizer* mesh, bool mapandUnmap);
		WriteTransactionReceipt genrateWriteReceipt(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, BinaryMeshSeirilizer* mesh);

		void bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding);
		/// <summary>
		/// </summary>
		/// <param name="commandsInfo"></param>
		/// <returns>a pointer to the streams that should be deleted when the vkGeneratedCommandsInfoNV struct is no longer neaded</returns>
		std::array<vk::IndirectCommandsStreamNV, 5>* bindVerticiesIntoIndirectCommandsNV(vk::GeneratedCommandsInfoNV& commandsInfo);
		void bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer);

		const VkDeviceSize vCount;
		const VkDeviceSize indexCount;

		Buffer* vertBuffer;
		Buffer* indexBuffer;

		VkDeviceSize         vertsSize();
		VkDeviceSize           uvsSize();
		VkDeviceSize       normalsSize();
		VkDeviceSize      tangentsSize();
		VkDeviceSize    bitangentsSize();
		VkDeviceSize      indiciesSize();

		VkDeviceSize       vertsOffset();
		VkDeviceSize         uvsOffset();
		VkDeviceSize     normalsOffset();
		VkDeviceSize    tangentsOffset();
		VkDeviceSize  bitangentsOffset();

	private:


	};

}