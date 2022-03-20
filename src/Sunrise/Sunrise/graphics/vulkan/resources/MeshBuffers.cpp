#include "srpch.h"
#include "MeshBuffers.h"
#include "../../../fileFormats/binary/BinaryMesh.h"
#include "../../../math/mesh/Mesh.h"

namespace sunrise::gfx {

	/// <summary>
/// creates a buffer with the right size but does not populate it
/// </summary>
/// <param name="device"></param>
/// <param name="allocator"></param>
/// <param name="options"></param>
/// <param name="mesh">must be non nill so size of buffer can be determind - only derefrenced during initilizer and possibly durring calls to writeMeshToBuffer</param>
	MeshBuffer::MeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options,const Mesh* mesh)
		: baseMesh(mesh)
	{
		auto bufferSize = baseMesh->fullSize();

		buffer = new Buffer(device, allocator, static_cast<VkDeviceSize>(bufferSize), options);
	}

	MeshBuffer::~MeshBuffer()
	{
		delete buffer;
	}

	void MeshBuffer::writeMeshToBuffer(bool mapandUnmap, const Mesh* mesh)
	{
		if (!mesh && !baseMesh)
			return;
		auto baseMesh = this->baseMesh;
		if (mesh)
			baseMesh = mesh;

		if (mapandUnmap)
			buffer->mapMemory();




		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->vertsOffset(), baseMesh->verts.data(), baseMesh->vertsSize());
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->uvsOffset(), baseMesh->uvs.data(), baseMesh->uvsSize());
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->normalsOffset(), baseMesh->normals.data(), baseMesh->normalsSize());
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->tangentsOffset(), baseMesh->tangents.data(), baseMesh->tangentsSize());
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->bitangentsOffset(), baseMesh->bitangents.data(), baseMesh->bitangentsSize());

		//TODO: Support Multi-submesh Meshes of Mesh class -- this might already
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->indiciesOffset(), baseMesh->indicies[0].data(), baseMesh->indiciesSize(0));

		if (mapandUnmap)
			buffer->unmapMemory();
	}

	void MeshBuffer::bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding, Mesh* mesh)
	{
		if (!mesh && !baseMesh)
			return;
		auto baseMesh = this->baseMesh;
		if (mesh)
			baseMesh = mesh;

		commandBuffer.bindVertexBuffers(baseBinding, {
				buffer->vkItem,
				buffer->vkItem,
				buffer->vkItem,
				buffer->vkItem,
				buffer->vkItem
			}, {
				baseMesh->vertsOffset(),
				baseMesh->uvsOffset(),
				baseMesh->normalsOffset(),
				baseMesh->tangentsOffset(),
				baseMesh->bitangentsOffset()
			});
	}

	void MeshBuffer::bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer, Mesh* mesh)
	{
		if (!mesh && !baseMesh)
			return;
		auto baseMesh = this->baseMesh;
		if (mesh)
			baseMesh = mesh;

		commandBuffer.bindIndexBuffer(buffer->vkItem, baseMesh->indiciesOffset(), vk::IndexType::eUint32);
	}

	void MeshBuffer::clearBaseMesh()
	{
		baseMesh = nullptr;
	}



	BindlessMeshBuffer::BindlessMeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options, VkDeviceSize vCount, VkDeviceSize indexCount)
		: vCount(vCount), indexCount(indexCount)
	{
		auto originalUsage = options.usage;

		auto vertbufferSize = (4 * sizeof(glm::vec3) + sizeof(glm::vec2)) * vCount;

		options.usage = originalUsage | vk::BufferUsageFlagBits::eVertexBuffer;

		vertBuffer = new Buffer(device, allocator, vertbufferSize, options);

		options.usage = originalUsage | vk::BufferUsageFlagBits::eIndexBuffer;

		indexBuffer = new Buffer(device, allocator, indexCount * sizeof(glm::uint32_t), options);

	}

	BindlessMeshBuffer::~BindlessMeshBuffer()
	{
		delete vertBuffer;
		delete indexBuffer;
	}

	void BindlessMeshBuffer::writeMeshToBuffer(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, Mesh* mesh, bool mapandUnmap)
	{
		PROFILE_FUNCTION
			if (mapandUnmap)
			{
				vertBuffer->mapMemory();
				indexBuffer->mapMemory();
			}

		memcpy(static_cast<char*>(vertBuffer->mappedData) + vertsOffset() + vertIndex * sizeof(glm::vec3), mesh->verts.data(), mesh->vertsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + uvsOffset() + vertIndex * sizeof(glm::vec2), mesh->uvs.data(), mesh->uvsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + normalsOffset() + vertIndex * sizeof(glm::vec3), mesh->normals.data(), mesh->normalsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + tangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->tangents.data(), mesh->tangentsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + bitangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->bitangents.data(), mesh->bitangentsSize());

		//TODO: Support Multi-submesh Meshes of Mesh class -- this might already
		memcpy(static_cast<char*>(indexBuffer->mappedData) + indIndex * sizeof(glm::uint32), mesh->indicies[0].data(), mesh->indiciesSize(0));

		if (mapandUnmap)
		{
			vertBuffer->unmapMemory();
			indexBuffer->unmapMemory();
		}
	}

	BindlessMeshBuffer::WriteTransactionReceipt BindlessMeshBuffer::genrateWriteReceipt(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, Mesh* mesh)
	{
		BindlessMeshBuffer::WriteTransactionReceipt report;

		BindlessMeshBuffer::WriteLocation location0 = { vertsOffset() + vertIndex * sizeof(glm::vec3), mesh->vertsSize() };
		BindlessMeshBuffer::WriteLocation location1 = { uvsOffset() + vertIndex * sizeof(glm::vec2), mesh->uvsSize() };
		BindlessMeshBuffer::WriteLocation location2 = { normalsOffset() + vertIndex * sizeof(glm::vec3), mesh->normalsSize() };
		BindlessMeshBuffer::WriteLocation location3 = { tangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->tangentsSize() };
		BindlessMeshBuffer::WriteLocation location4 = { bitangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->bitangentsSize() };

		report.vertexLocations = {
			location0,
			location1,
			location2,
			location3,
			location4,
		};
		//TODO: Support Multi-submesh Meshes of Mesh class
		report.indexLocation = { indIndex * sizeof(glm::uint32), mesh->indiciesSize(0) };
		return report;
	}

	void BindlessMeshBuffer::writeMeshToBuffer(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, BinaryMeshSeirilizer* mesh, bool mapandUnmap)
	{
		PROFILE_FUNCTION

			if (mapandUnmap)
			{
				vertBuffer->mapMemory();
				indexBuffer->mapMemory();
			}

		memcpy(static_cast<char*>(vertBuffer->mappedData) + vertsOffset() + vertIndex * sizeof(glm::vec3), mesh->vertsPtr(), mesh->vertsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + uvsOffset() + vertIndex * sizeof(glm::vec2), mesh->uvsPtr(), mesh->uvsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + normalsOffset() + vertIndex * sizeof(glm::vec3), mesh->normalsPtr(), mesh->normalsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + tangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->tangentsPtr(), mesh->tangentsSize());
		memcpy(static_cast<char*>(vertBuffer->mappedData) + bitangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->bitangentsPtr(), mesh->bitangentsSize());
		//sets all submeshes since they are contigius in memory of mesh and index buffer
		memcpy(static_cast<char*>(indexBuffer->mappedData) + indIndex * sizeof(glm::uint32), mesh->indiciesPtr(0), mesh->AllSubMeshIndiciesSize());

		if (mapandUnmap)
		{
			vertBuffer->unmapMemory();
			indexBuffer->unmapMemory();
		}
	}

	BindlessMeshBuffer::WriteTransactionReceipt BindlessMeshBuffer::genrateWriteReceipt(VkDeviceAddress vertIndex, VkDeviceAddress indIndex, BinaryMeshSeirilizer* mesh)
	{
		BindlessMeshBuffer::WriteTransactionReceipt report;

		BindlessMeshBuffer::WriteLocation location0 = { vertsOffset() + vertIndex * sizeof(glm::vec3), mesh->vertsSize() };
		BindlessMeshBuffer::WriteLocation location1 = { uvsOffset() + vertIndex * sizeof(glm::vec2), mesh->uvsSize() };
		BindlessMeshBuffer::WriteLocation location2 = { normalsOffset() + vertIndex * sizeof(glm::vec3), mesh->normalsSize() };
		BindlessMeshBuffer::WriteLocation location3 = { tangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->tangentsSize() };
		BindlessMeshBuffer::WriteLocation location4 = { bitangentsOffset() + vertIndex * sizeof(glm::vec3), mesh->bitangentsSize() };

		report.vertexLocations = {
			location0,
			location1,
			location2,
			location3,
			location4,
		};
		report.indexLocation = { indIndex * sizeof(glm::uint32), mesh->AllSubMeshIndiciesSize() };
		return report;
	}

	void BindlessMeshBuffer::bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding)
	{
		commandBuffer.bindVertexBuffers(baseBinding, {
				vertBuffer->vkItem,
				vertBuffer->vkItem,
				vertBuffer->vkItem,
				vertBuffer->vkItem,
				vertBuffer->vkItem
			}, {
				vertsOffset(),
				uvsOffset(),
				normalsOffset(),
				tangentsOffset(),
				bitangentsOffset()
			});
	}

	std::array<vk::IndirectCommandsStreamNV, 5>* BindlessMeshBuffer::bindVerticiesIntoIndirectCommandsNV(vk::GeneratedCommandsInfoNV& commandsInfo)
	{
		
		auto* streams = new std::array<vk::IndirectCommandsStreamNV, 5>{};

		(*streams)[0].buffer = vertBuffer->vkItem;
		(*streams)[1].buffer = vertBuffer->vkItem;
		(*streams)[2].buffer = vertBuffer->vkItem;
		(*streams)[3].buffer = vertBuffer->vkItem;
		(*streams)[4].buffer = vertBuffer->vkItem;


		(*streams)[0].offset = vertsOffset();
		(*streams)[1].offset = uvsOffset();
		(*streams)[2].offset = normalsOffset();
		(*streams)[3].offset = tangentsOffset();
		(*streams)[4].offset = bitangentsOffset();

		commandsInfo.streamCount = streams->size();
		commandsInfo.pStreams = streams->data();

		return streams;
	}

	void BindlessMeshBuffer::bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.bindIndexBuffer(indexBuffer->vkItem, 0, vk::IndexType::eUint32);
	}

	VkDeviceSize BindlessMeshBuffer::vertsSize()
	{
		return vCount * sizeof(glm::vec3);
	}

	VkDeviceSize BindlessMeshBuffer::uvsSize()
	{
		return vCount * sizeof(glm::vec2);
	}

	VkDeviceSize BindlessMeshBuffer::normalsSize()
	{
		return vCount * sizeof(glm::vec3);
	}

	VkDeviceSize BindlessMeshBuffer::tangentsSize()
	{
		return vCount * sizeof(glm::vec3);
	}

	VkDeviceSize BindlessMeshBuffer::bitangentsSize()
	{
		return vCount * sizeof(glm::vec3);
	}

	VkDeviceSize BindlessMeshBuffer::indiciesSize()
	{
		return indexCount * sizeof(glm::uint32);
	}

	VkDeviceSize BindlessMeshBuffer::vertsOffset()
	{
		return 0;
	}

	VkDeviceSize BindlessMeshBuffer::uvsOffset()
	{
		return vertsOffset() + vertsSize();
	}

	VkDeviceSize BindlessMeshBuffer::normalsOffset()
	{
		return uvsOffset() + uvsSize();
	}

	VkDeviceSize BindlessMeshBuffer::tangentsOffset()
	{
		return normalsOffset() + normalsSize();
	}

	VkDeviceSize BindlessMeshBuffer::bitangentsOffset()
	{
		return tangentsOffset() + tangentsSize();
	}


	Basic2DMeshBuffer::Basic2DMeshBuffer(vk::Device device, VmaAllocator allocator, BufferCreationOptions options, Basic2DMesh* mesh)
		: baseMesh(mesh)
	{
		auto bufferSize = baseMesh->fullSize();

		buffer = new Buffer(device, allocator, static_cast<VkDeviceSize>(bufferSize), options);
	}

	Basic2DMeshBuffer::~Basic2DMeshBuffer()
	{
		delete buffer;
	}

	void Basic2DMeshBuffer::writeMeshToBuffer(bool mapandUnmap)
	{
		if (mapandUnmap)
			buffer->mapMemory();

		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->vertsOffset(), baseMesh->verts.data(), baseMesh->vertsSize());
		memcpy(static_cast<char*>(buffer->mappedData) + baseMesh->indiciesOffset(), baseMesh->indicies.data(), baseMesh->indiciesSize());

		if (mapandUnmap)
			buffer->unmapMemory();
	}

	void Basic2DMeshBuffer::bindVerticiesIntoCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t baseBinding)
	{
		commandBuffer.bindVertexBuffers(baseBinding, {
				buffer->vkItem
			}, {
				baseMesh->vertsOffset()
			});
	}

	void Basic2DMeshBuffer::bindIndiciesIntoCommandBuffer(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.bindIndexBuffer(buffer->vkItem, baseMesh->indiciesOffset(), vk::IndexType::eUint32);
	}

}