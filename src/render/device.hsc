package render

import vulkan from "vendor"

DEVICE_REQUIRED_EXTENSIONS :: string[]{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
}

Device :: struct {
    physical_device : VkPhysicalDevice,
    logical_device  : VkDevice,
    graphics_queue  : VkQueue,
    present_queue   : VkQueue,
    descriptor_pool : VkDescriptorPool,
    indices         : QueueFamilyIndices,
}

QueueFamilyIndices :: struct {
    graphics_family : VkQueueFlags?,
    present_family : VkQueueFlags?,
}

check_swap_chain_support :: fn(device: VkPhysicalDevice, surface: VkSurfaceKHR) -> SwapChainSupport {
    result : SwapChainSupport

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &result.capabilities)

    format_count : u32
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, null)

    if format_count > 0 {
        array_reserve(result.formats, format_count)
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, result.formats)
    }

    present_mode_count : u32
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &format_count, null)

    if present_mode_count > 0 {
        array_reserve(result.present_modes, present_mode_count)
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, result.present_modes)
    }

    return result
}

configure_surface :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

create_queues :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

destroy_device :: fn(device: ptr[mut Device]) {
    vkDestroyDescriptorPool(device.physical_device, device.descriptor_pool, null)

    //NOTE(ches) queues are implicitly destroyed when the logical device is
    //NOTE(ches) physical device gets destroyed implicitly with the instance
    if !vk_is_null_handle(device.logical_device) {
        vkDestroyDevice(device.logical_device, null)
    }
}

find_queue_families :: fn(device: VkPhysicalDevice) -> QueueFamilyIndices {
    //TODO(ches) fill this out
    result : QueueFamilyIndices
    return result
}

initialize_device :: fn(device: ptr[mut Device]) {
    select_physical_device(device)
    select_logical_device(device)
    create_queues(device)
    configure_surface(device)

    pool_sizes :: VkDescriptorPoolSize[] {
        {type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT},
        {type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT},
        {type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT},
    }

    descriptor_pool_info : VkDescriptorPoolCreateInfo
    descriptor_pool_info.sType = .VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
    descriptor_pool_info.pNext = null
    descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    descriptor_pool_info.maxSets = MAX_FRAMES_IN_FLIGHT
    descriptor_pool_info.poolSizes = pool_sizes

    if vkCreateDescriptorPool(device.logical_device, &descriptor_pool_info, null, &device.descriptor_pool) != .VK_SUCCESS {
        //TODO(ches) fatal log
    }
}

rate_device :: fn(device: VkPhysicalDevice) -> uint {
    //TODO(ches) fill this out
    return 0
}

select_logical_device :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

select_physical_device :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

supports_required_extensions(device: VkPhysicalDevice) -> bool {
    //TODO(ches) fill this out
    return false
}
