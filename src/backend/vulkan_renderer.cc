#include <unigui/backend/vulkan_types.h>
#include <unigui/backend/backend_factory.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <cstdio>

namespace unigui {
namespace {

class VulkanRenderer : public RendererBackend {
public:
    VulkanRenderer(VulkanDevice* vd, VulkanSwapchain* sc, VulkanPipeline* pl, std::vector<FrameResources>* frames)
        : vd_(vd), sc_(sc), pl_(pl), frames_(frames) {}

    bool Init(ImGuiContext*) override { initialized_ = vd_ && sc_ && pl_ && frames_; return initialized_; }
    void Shutdown() override { initialized_ = false; }
    void RenderDrawData(ImDrawData* dd) override {
        if (!initialized_ || !dd || frames_->empty()) return;
        auto& fr = (*frames_)[currentFrame_];
        vkWaitForFences(vd_->device, 1, &fr.inFlight, VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        VkResult r = vkAcquireNextImageKHR(vd_->device, sc_->swapchain, UINT64_MAX, fr.imageAvailable, VK_NULL_HANDLE, &imageIndex);
        if (r == VK_ERROR_OUT_OF_DATE_KHR) return;
        vkResetFences(vd_->device, 1, &fr.inFlight);
        vkResetCommandBuffer(fr.commandBuffer, 0);

        VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(fr.commandBuffer, &bi);

        VkClearValue cv{};
        cv.color.float32[0] = clearR_; cv.color.float32[1] = clearG_;
        cv.color.float32[2] = clearB_; cv.color.float32[3] = clearA_;
        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = sc_->renderPass; rp.framebuffer = sc_->framebuffers[imageIndex];
        rp.renderArea.extent = sc_->extent; rp.clearValueCount = 1; rp.pClearValues = &cv;
        vkCmdBeginRenderPass(fr.commandBuffer, &rp, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(dd, fr.commandBuffer);

        vkCmdEndRenderPass(fr.commandBuffer); vkEndCommandBuffer(fr.commandBuffer);

        VkPipelineStageFlags ws[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.waitSemaphoreCount = 1; si.pWaitSemaphores = &fr.imageAvailable;
        si.pWaitDstStageMask = ws; si.commandBufferCount = 1; si.pCommandBuffers = &fr.commandBuffer;
        si.signalSemaphoreCount = 1; si.pSignalSemaphores = &fr.renderFinished;
        vkQueueSubmit(vd_->graphicsQueue, 1, &si, fr.inFlight);

        VkPresentInfoKHR pi{};
        pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; pi.waitSemaphoreCount = 1; pi.pWaitSemaphores = &fr.renderFinished;
        pi.swapchainCount = 1; pi.pSwapchains = &sc_->swapchain; pi.pImageIndices = &imageIndex;
        vkQueuePresentKHR(vd_->presentQueue, &pi);

        currentFrame_ = (currentFrame_ + 1) % (uint32_t)frames_->size();
    }
    void SetClearColor(float r, float g, float b, float a) override { clearR_ = r; clearG_ = g; clearB_ = b; clearA_ = a; }

private:
    VulkanDevice* vd_; VulkanSwapchain* sc_; VulkanPipeline* pl_; std::vector<FrameResources>* frames_;
    uint32_t currentFrame_ = 0; float clearR_ = 0.10f, clearG_ = 0.10f, clearB_ = 0.12f, clearA_ = 1.00f;
    bool initialized_ = false;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateVulkanRenderer() { return nullptr; }

} // namespace unigui