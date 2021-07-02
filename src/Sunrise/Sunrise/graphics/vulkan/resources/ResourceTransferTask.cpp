#include "srpch.h"
#include "ResourceTransferTask.h"
#include "../renderer/Renderer.h"
#include "../generalAbstractions/VkAbstractions.h"



namespace sunrise {

	MarlSafeTicketLock::MarlSafeTicketLock(marl::Ticket ticket)
		: ticket(ticket)
	{
		ticket.wait();
	}

	MarlSafeTicketLock::~MarlSafeTicketLock()
	{
		ticket.done();
	}

	namespace gfx {

		ResourceTransferer::ResourceTransferer(vk::Device device, Renderer& renderer)
			: device(device), renderer(renderer), ticketQueue()
		{

			vk::CommandPoolCreateInfo poolInfo{};

			poolInfo.queueFamilyIndex = renderer.queueFamilyIndices.resourceTransferFamily.value();
			poolInfo.flags = vk::CommandPoolCreateFlags(); // Optional

			copyPool = device.createCommandPool(poolInfo);

			poolInfo.queueFamilyIndex = renderer.queueFamilyIndices.graphicsFamily.value();

			gfxPool = device.createCommandPool(poolInfo);

			vkHelpers::allocateCommandBuffers(device, copyPool, &copyCmdBuffer, 1);
			vkHelpers::allocateCommandBuffers(device, gfxPool, &gfxCmdBuffer, 1);

			vk::FenceCreateInfo fenceCreate{};

			fenceCreate.flags = {};

			waitFence = device.createFence(fenceCreate);

		}

		ResourceTransferer::~ResourceTransferer()
		{
			ticketQueue.take().wait();
			device.destroyCommandPool(copyPool);
		}



		void ResourceTransferer::newTask(std::vector<Task>& tasks, std::function<void()> completionHandler, bool synchronus, bool requiresGfxQueue)
		{
			PROFILE_FUNCTION

				//marl::schedule([]() {printf("transfer job"); });
				auto ticket = ticketQueue.take();

#if SR_SingleQueueForRenderDoc
			requiresGfxQueue = true;
			synchronus = true;
#endif

			if (synchronus)
				performTask(tasks, ticket, completionHandler, requiresGfxQueue);
			else {
				marl::schedule([tasks, ticket, completionHandler, this, requiresGfxQueue]() { performTask(tasks, ticket, completionHandler, requiresGfxQueue); });
			}
		}


		void ResourceTransferer::performTask(std::vector<Task> tasks, marl::Ticket ticket, std::function<void()> completionHandler, bool requiresGfxQueue)
		{


			static int c = 0;
			//if (c++ > 7) return;

			PROFILE_FUNCTION
			{
				//// will lock the ticket until the lock goes out of scope
				//TODO: fix this
				//MarlSafeTicketLock lock(ticket);

				ticket.wait();

				PROFILE_SCOPE("performTask Marl blocking");

				vk::CommandBuffer cmdBuffer;

				if (requiresGfxQueue) {
					cmdBuffer = gfxCmdBuffer;
					device.resetCommandPool(gfxPool, {});
				}
				else {
					cmdBuffer = copyCmdBuffer;
					device.resetCommandPool(copyPool, {});
				}

				cmdBuffer.begin({ { vk::CommandBufferUsageFlagBits::eOneTimeSubmit } });

				renderer.debugObject.beginRegion(cmdBuffer, "Resource Transfer Task", glm::vec4(0.4,0.2,0.1,1));

				// perform tasks
				for (Task& task : tasks) {
					switch (task.type)
					{
					case TaskType::bufferTransfers:
						performBufferTransferTask(task.bufferTransferTask,cmdBuffer);
						break;

					case TaskType::imageLayoutTransition:
						performImageLayoutTransitionTask(task.imageLayoutTransitonTask,cmdBuffer);
						break;

					case TaskType::bufferToImageCopyWithTransition:
						performBufferToImageCopyWithTransitionTask(task.bufferToImageCopyWithTransitionTask,cmdBuffer);
						break;

						//Graphics Queue


						case TaskType::generateMipMaps:
							performGenerateMipMapsTask(task.generateMipMapsTask,cmdBuffer);
							break;
						default:
							break;
						}

				}
				renderer.debugObject.endRegion(cmdBuffer);
				
				cmdBuffer.end();


				vk::SubmitInfo submitInfo{};
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &cmdBuffer;

				if (requiresGfxQueue) {
					SR_CORE_WARN("submitting resource task(s) to main gfx queue");
					renderer.deviceQueues.graphics.submit(submitInfo, waitFence);
					//renderer.deviceQueues.graphics.waitIdle();
				}
				else {
					SR_CORE_INFO("submitting resource task(s) to transfer queue");
					renderer.deviceQueues.resourceTransfer.submit(submitInfo, waitFence);
					//renderer.deviceQueues.resourceTransfer.waitIdle();
					//renderer.device.waitIdle();


					//device.waitForFences(waitFence, VK_TRUE, UINT64_MAX);
					//device.resetFences(waitFence);
				}


				//TODO: find better way than blocking the resource transfer marl queue


				device.waitForFences(waitFence, VK_TRUE, UINT64_MAX);
				device.resetFences(waitFence);

				ticket.done();
			}


				// after ticket unlcok so other threads can access the vk resources
				//marl::blocking_call(completionHandler);
			completionHandler();
		}


		void ResourceTransferer::performBufferTransferTask(ResourceTransferer::BufferTransferTask& t, vk::CommandBuffer cmdBuffer)
		{
			cmdBuffer.copyBuffer(t.srcBuffer, t.dstBuffer, t.regions);
		}

		void ResourceTransferer::performBufferToImageCopyWithTransitionTask(ResourceTransferer::BufferToImageCopyWithTransitionTask& t, vk::CommandBuffer cmdBuffer)
		{

			ImageLayoutTransitionTask layoutTask;
			layoutTask.image = t.image;
			layoutTask.imageAspectMask = t.imageAspectMask;
			layoutTask.oldLayout = t.oldLayout;
			layoutTask.newLayout = vk::ImageLayout::eTransferDstOptimal;
			layoutTask.baseMipLevel = 0;
			layoutTask.mipLevelCount = t.mipLevelCount;

			performImageLayoutTransitionTask(layoutTask, cmdBuffer);

			vk::BufferImageCopy region;

			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = t.imageAspectMask;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset.x = 0;
			region.imageOffset.y = 0;
			region.imageOffset.z = 0;
			region.imageExtent = t.imageSize;



			cmdBuffer.copyBufferToImage(
				t.buffer,
				t.image,
				vk::ImageLayout::eTransferDstOptimal,
				{ region }
			);


			if (layoutTask.newLayout != t.finalLayout) {

				layoutTask.oldLayout = layoutTask.newLayout;
				layoutTask.newLayout = t.finalLayout;

				performImageLayoutTransitionTask(layoutTask, cmdBuffer);

			}
		}

		void ResourceTransferer::performImageLayoutTransitionTask(ResourceTransferer::ImageLayoutTransitionTask& t, vk::CommandBuffer cmdBuffer)
		{
			vk::ImageMemoryBarrier barrier{};
			barrier.oldLayout = t.oldLayout;
			barrier.newLayout = t.newLayout;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = t.image;
			barrier.subresourceRange.aspectMask = t.imageAspectMask;
			barrier.subresourceRange.baseMipLevel = t.baseMipLevel;
			barrier.subresourceRange.levelCount = t.mipLevelCount;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			if (t.oldLayout == vk::ImageLayout::eUndefined && t.newLayout == vk::ImageLayout::eTransferDstOptimal) {
				barrier.srcAccessMask = vk::AccessFlagBits{ 0 };
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
				destinationStage = vk::PipelineStageFlagBits::eTransfer;
			}
			else if ((t.oldLayout == vk::ImageLayout::eTransferDstOptimal || t.oldLayout == vk::ImageLayout::eTransferSrcOptimal) && t.newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				//TODO: extract this as a property of the transfer that can be specified
				//TODO:::::::::::: fix this i'm not sure it this is right 
				destinationStage = vk::PipelineStageFlagBits::eAllCommands;//vk::PipelineStageFlagBits::eFragmentShader;
			}
			else if (t.oldLayout == vk::ImageLayout::eTransferDstOptimal && t.newLayout == vk::ImageLayout::eTransferSrcOptimal) {
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

				sourceStage = vk::PipelineStageFlagBits::eTransfer;

				destinationStage = vk::PipelineStageFlagBits::eTransfer;//vk::PipelineStageFlagBits::eFragmentShader;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			//barrier.srcAccessMask = srcAccessMask; // TODO
			//barrier.dstAccessMask = dstAccessMask; // TODO

			cmdBuffer.pipelineBarrier(
				sourceStage, destinationStage, //TODO
				{},
				{},
				{},
				{ barrier });
		}

		void ResourceTransferer::performGenerateMipMapsTask(ResourceTransferer::GenerateMipMapsTask& t, vk::CommandBuffer cmdBuffer)
		{
			ImageLayoutTransitionTask layoutTask;
			layoutTask.image = t.image;
			layoutTask.imageAspectMask = t.imageAspectMask;


			int32_t mipWidth = t.imageSize.width;
			int32_t mipHeight = t.imageSize.height;

			for (uint32_t i = 1; i < t.mipLevels; i++) {
				layoutTask.oldLayout = t.oldLayout;
				layoutTask.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				layoutTask.baseMipLevel = i - 1;

				performImageLayoutTransitionTask(layoutTask, cmdBuffer);

				vk::ImageBlit blit{};
				blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
				blit.srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
				blit.srcSubresource.aspectMask = layoutTask.imageAspectMask;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
				blit.dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
				blit.dstSubresource.aspectMask = layoutTask.imageAspectMask;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				cmdBuffer.blitImage(t.image, vk::ImageLayout::eTransferSrcOptimal, t.image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);


				layoutTask.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				layoutTask.newLayout = t.finalLayout;

				performImageLayoutTransitionTask(layoutTask, cmdBuffer);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}


			layoutTask.baseMipLevel = t.mipLevels - 1;
			layoutTask.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			layoutTask.newLayout = t.finalLayout;

			//performImageLayoutTransitionTask(layoutTask,cmdBuffer);



		}

	}
}