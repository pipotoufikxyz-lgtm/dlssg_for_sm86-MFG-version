#include "renderer/vulkan_renderer.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <cstring>
#include <limits>
#include <set>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace renderer {
namespace {
const char* ErrorName(VkResult result) {
    return result == VK_SUCCESS ? "success" : "Vulkan operation failed";
}
}

struct VulkanRenderer::Resource {
    enum class Kind { Buffer, Image, Shader };
    Kind kind = Kind::Buffer;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkShaderModule shader = VK_NULL_HANDLE;
};

VulkanRenderer::~VulkanRenderer() { Shutdown(); }

bool VulkanRenderer::Check(VkResult result, const char* action,
                           std::string& error) {
    if (result == VK_SUCCESS) return true;
    error = std::string(action) + ": " + ErrorName(result);
    return false;
}

bool VulkanRenderer::CreateInstance(bool validation, std::string& error) {
    std::vector<const char*> layers;
    if (validation) {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> available(count);
        vkEnumerateInstanceLayerProperties(&count, available.data());
        for (const auto& layer : available)
            if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
                layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
#if defined(_WIN32)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "DLSSG renderer";
    app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app.pEngineName = "renderer";
    app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo create{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create.pApplicationInfo = &app;
    create.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create.ppEnabledExtensionNames = extensions.data();
    create.enabledLayerCount = static_cast<uint32_t>(layers.size());
    create.ppEnabledLayerNames = layers.data();
    return Check(vkCreateInstance(&create, nullptr, &instance_), "vkCreateInstance",
                 error);
}

bool VulkanRenderer::CreateDevice(std::string& error) {
    uint32_t count = 0;
    if (!Check(vkEnumeratePhysicalDevices(instance_, &count, nullptr),
               "vkEnumeratePhysicalDevices", error) || count == 0) {
        error = "No Vulkan physical device is available";
        return false;
    }

    std::vector<VkPhysicalDevice> devices(count);
    if (!Check(vkEnumeratePhysicalDevices(instance_, &count, devices.data()),
               "vkEnumeratePhysicalDevices", error))
        return false;
    physical_device_ = devices.front();
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device_, &properties);
    device_info_.name = properties.deviceName;
    device_info_.api = "Vulkan";
    device_info_.vendor_id = properties.vendorID;
    device_info_.device_id = properties.deviceID;

    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &family_count,
                                             families.data());
    for (uint32_t i = 0; i < family_count; ++i) {
        if (families[i].queueCount && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            graphics_family_ = i;
        if (families[i].queueCount && surface_ != VK_NULL_HANDLE) {
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &present);
            if (present) present_family_ = i;
        }
    }
    if (graphics_family_ == UINT32_MAX || present_family_ == UINT32_MAX) {
        error = "No Vulkan graphics/present queue pair is available";
        return false;
    }
    std::set<uint32_t> families_used{graphics_family_, present_family_};
    float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queues;
    for (uint32_t family : families_used) {
        VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue.queueFamilyIndex = family;
        queue.queueCount = 1;
        queue.pQueuePriorities = &priority;
        queues.push_back(queue);
    }
    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo create{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create.queueCreateInfoCount = static_cast<uint32_t>(queues.size());
    create.pQueueCreateInfos = queues.data();
    create.enabledExtensionCount = 1;
    create.ppEnabledExtensionNames = extensions;
    if (!Check(vkCreateDevice(physical_device_, &create, nullptr, &device_),
               "vkCreateDevice", error))
        return false;
    vkGetDeviceQueue(device_, graphics_family_, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, present_family_, 0, &present_queue_);
    VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool.queueFamilyIndex = graphics_family_;
    if (!Check(vkCreateCommandPool(device_, &pool, nullptr, &command_pool_),
               "vkCreateCommandPool", error))
        return false;
    VkCommandBufferAllocateInfo allocate{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate.commandPool = command_pool_;
    allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate.commandBufferCount = 1;
    return Check(vkAllocateCommandBuffers(device_, &allocate, &command_buffer_),
                 "vkAllocateCommandBuffers", error);
}

bool VulkanRenderer::CreateSwapchain(uint32_t width, uint32_t height,
                                     std::string& error) {
    if (surface_ == VK_NULL_HANDLE) {
        error = "Vulkan surface creation requires a native Win32 window";
        return false;
    }
    VkSurfaceCapabilitiesKHR capabilities{};
    if (!Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_,
                                                         &capabilities),
               "vkGetPhysicalDeviceSurfaceCapabilitiesKHR", error))
        return false;
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
    if (format_count == 0) { error = "Vulkan surface has no formats"; return false; }
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count,
                                         formats.data());
    VkSurfaceFormatKHR format = formats.front();
    VkExtent2D extent{width, height};
    if (capabilities.currentExtent.width != UINT32_MAX)
        extent = capabilities.currentExtent;
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 &&
        image_count > capabilities.maxImageCount)
        image_count = capabilities.maxImageCount;
    VkSwapchainCreateInfoKHR create{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    create.surface = surface_;
    create.minImageCount = image_count;
    create.imageFormat = format.format;
    create.imageColorSpace = format.colorSpace;
    create.imageExtent = extent;
    create.imageArrayLayers = 1;
    create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    uint32_t queue_families[] = {graphics_family_, present_family_};
    if (graphics_family_ != present_family_) {
        create.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create.queueFamilyIndexCount = 2;
        create.pQueueFamilyIndices = queue_families;
    } else {
        create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    create.preTransform = capabilities.currentTransform;
    create.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create.clipped = VK_TRUE;
    if (!Check(vkCreateSwapchainKHR(device_, &create, nullptr, &swapchain_),
               "vkCreateSwapchainKHR", error))
        return false;
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, images_.data());
    views_.resize(image_count);
    for (uint32_t i = 0; i < image_count; ++i) {
        VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view.image = images_[i];
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format.format;
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.layerCount = 1;
        if (!Check(vkCreateImageView(device_, &view, nullptr, &views_[i]),
                   "vkCreateImageView", error))
            return false;
    }
    VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    return Check(vkCreateSemaphore(device_, &semaphore, nullptr, &image_available_),
                 "vkCreateSemaphore", error) &&
           Check(vkCreateSemaphore(device_, &semaphore, nullptr, &render_finished_),
                 "vkCreateSemaphore", error) &&
           Check(vkCreateFence(device_, &fence, nullptr, &frame_fence_),
                 "vkCreateFence", error);
}

bool VulkanRenderer::Initialize(const CreateInfo& info, std::string& error) {
    Shutdown();
    if (!CreateInstance(info.validation, error)) return false;
#if defined(_WIN32)
    HWND window = static_cast<HWND>(info.native_window);
    if (window == nullptr) { error = "Vulkan requires a Win32 HWND"; return false; }
    VkWin32SurfaceCreateInfoKHR surface{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    surface.hwnd = window;
    surface.hinstance = GetModuleHandleW(nullptr);
    if (!Check(vkCreateWin32SurfaceKHR(instance_, &surface, nullptr, &surface_),
               "vkCreateWin32SurfaceKHR", error)) return false;
#else
    error = "Vulkan renderer currently requires Win32";
    return false;
#endif
    return CreateDevice(error) && CreateSwapchain(info.width, info.height, error);
}

bool VulkanRenderer::Resize(uint32_t width, uint32_t height, std::string& error) {
    if (device_ == VK_NULL_HANDLE) { error = "Vulkan device is not initialized"; return false; }
    vkDeviceWaitIdle(device_);
    DestroySwapchain();
    return CreateSwapchain(width, height, error);
}

bool VulkanRenderer::BeginFrame(std::string& error) {
    if (!Check(vkWaitForFences(device_, 1, &frame_fence_, VK_TRUE, UINT64_MAX),
               "vkWaitForFences", error) ||
        !Check(vkResetFences(device_, 1, &frame_fence_), "vkResetFences", error))
        return false;
    if (!Check(vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                     image_available_, VK_NULL_HANDLE,
                                     &image_index_),
               "vkAcquireNextImageKHR", error))
        return false;
    if (!Check(vkResetCommandBuffer(command_buffer_, 0),
               "vkResetCommandBuffer", error))
        return false;
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (!Check(vkBeginCommandBuffer(command_buffer_, &begin),
               "vkBeginCommandBuffer", error))
        return false;
    frame_active_ = true;
    return true;
}

bool VulkanRenderer::EndFrame(std::string& error) {
    if (!frame_active_) {
        error = "Vulkan EndFrame called without BeginFrame";
        return false;
    }
    if (!Check(vkEndCommandBuffer(command_buffer_), "vkEndCommandBuffer",
               error))
        return false;
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &image_available_;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_buffer_;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &render_finished_;
    if (!Check(vkQueueSubmit(graphics_queue_, 1, &submit, frame_fence_),
               "vkQueueSubmit", error))
        return false;
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &render_finished_;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &image_index_;
    if (!Check(vkQueuePresentKHR(present_queue_, &present), "vkQueuePresentKHR",
               error))
        return false;
    frame_active_ = false;
    return true;
}

void VulkanRenderer::DestroySwapchain() noexcept {
    if (device_ == VK_NULL_HANDLE) return;
    if (frame_fence_) vkDestroyFence(device_, frame_fence_, nullptr);
    if (image_available_) vkDestroySemaphore(device_, image_available_, nullptr);
    if (render_finished_) vkDestroySemaphore(device_, render_finished_, nullptr);
    for (auto view : views_) vkDestroyImageView(device_, view, nullptr);
    views_.clear(); images_.clear();
    if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
    frame_fence_ = VK_NULL_HANDLE;
    image_available_ = render_finished_ = VK_NULL_HANDLE;
}

void VulkanRenderer::Shutdown() noexcept {
    if (device_) vkDeviceWaitIdle(device_);
    DestroySwapchain();
    if (command_pool_) vkDestroyCommandPool(device_, command_pool_, nullptr);
    if (device_) vkDestroyDevice(device_, nullptr);
    if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_) vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE; surface_ = VK_NULL_HANDLE; device_ = VK_NULL_HANDLE;
    command_pool_ = VK_NULL_HANDLE; physical_device_ = VK_NULL_HANDLE;
    graphics_queue_ = present_queue_ = VK_NULL_HANDLE;
}

void* VulkanRenderer::CreateBuffer(const BufferDesc& desc, std::string& error) {
    if (!device_ || desc.size == 0) { error = "Invalid Vulkan buffer request"; return nullptr; }
    auto* resource = new Resource();
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    switch (desc.usage) {
    case BufferUsage::Index: usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
    case BufferUsage::Uniform: usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
    case BufferUsage::Storage: usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
    default: break;
    }
    if (!CreateBufferInternal(desc.size, usage,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               *resource, error) ||
        !UploadBuffer(*resource, desc.initial_data, desc.size, error)) {
        DestroyResource(resource);
        return nullptr;
    }
    return resource;
}

void* VulkanRenderer::CreateTexture(const TextureDesc& desc, std::string& error) {
    if (!device_ || desc.width == 0 || desc.height == 0) {
        error = "Invalid Vulkan texture request"; return nullptr;
    }
    auto* resource = new Resource();
    resource->kind = Resource::Kind::Image;
    VkImageCreateInfo image{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image.imageType = VK_IMAGE_TYPE_2D;
    image.extent = {desc.width, desc.height, 1};
    image.mipLevels = desc.mip_levels;
    image.arrayLayers = 1;
    image.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.render_target) image.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!Check(vkCreateImage(device_, &image, nullptr, &resource->image),
               "vkCreateImage", error))
        return nullptr;
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(device_, resource->image, &requirements);
    uint32_t type = FindMemoryType(requirements.memoryTypeBits,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (type == UINT32_MAX) { error = "No compatible Vulkan image memory type"; DestroyResource(resource); return nullptr; }
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = type;
    if (!Check(vkAllocateMemory(device_, &allocate, nullptr, &resource->memory),
               "vkAllocateMemory", error) ||
        !Check(vkBindImageMemory(device_, resource->image, resource->memory, 0),
               "vkBindImageMemory", error)) {
        DestroyResource(resource); return nullptr;
    }
    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.image = resource->image;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = image.format;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.levelCount = desc.mip_levels;
    view.subresourceRange.layerCount = 1;
    if (!Check(vkCreateImageView(device_, &view, nullptr, &resource->view),
               "vkCreateImageView", error)) {
        DestroyResource(resource); return nullptr;
    }
    return resource;
}

void* VulkanRenderer::CreateShader(const ShaderDesc& desc, std::string& error) {
    if (!device_ || !desc.bytecode || desc.bytecode_size == 0 ||
        desc.bytecode_size % 4 != 0) {
        error = "Vulkan shader bytecode must be aligned SPIR-V"; return nullptr;
    }
    auto* resource = new Resource();
    resource->kind = Resource::Kind::Shader;
    VkShaderModuleCreateInfo create{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    create.codeSize = desc.bytecode_size;
    create.pCode = static_cast<const uint32_t*>(desc.bytecode);
    if (!Check(vkCreateShaderModule(device_, &create, nullptr, &resource->shader),
               "vkCreateShaderModule", error)) {
        delete resource; return nullptr;
    }
    return resource;
}

void VulkanRenderer::DestroyResource(void* value) noexcept {
    auto* resource = static_cast<Resource*>(value);
    if (!resource || !device_) { delete resource; return; }
    if (resource->view) vkDestroyImageView(device_, resource->view, nullptr);
    if (resource->shader) vkDestroyShaderModule(device_, resource->shader, nullptr);
    if (resource->buffer) vkDestroyBuffer(device_, resource->buffer, nullptr);
    if (resource->image) vkDestroyImage(device_, resource->image, nullptr);
    if (resource->memory) vkFreeMemory(device_, resource->memory, nullptr);
    delete resource;
}

}  // namespace renderer
