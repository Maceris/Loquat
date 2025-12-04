package render

import os from "std"
import vulkan from "vendor"

DEVICE_REQUIRED_EXTENSIONS :: string[]{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
}

ENABLE_VALIDATION_LAYERS :: true

VALIDATION_LAYERS :: string[] {
    "VK_LAYER_KHRONOS_validation"
}

Device :: struct {
    physical_device : VkPhysicalDevice,
    logical_device  : VkDevice,
    graphics_queue  : VkQueue,
    present_queue   : VkQueue,
    descriptor_pool : VkDescriptorPool,
    //TODO(ches) can we just keep this in a couple functions?
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

configure_surface :: fn(device: ptr[mut Device], surface: ptr[mut WindowSurface]) {
    //TODO(ches) this SwapChainSupport struct feels kinda pointless, can we just inline this function?
    swap_chain_support : SwapChainSupport = check_swap_chain_support(device.physical_device, surface.vulkan_surface)
    select_present_mode(surface, swap_chain_support.present_modes)
    select_surface_format(surface, swap_chain_support.formats)
}

create_queues :: fn(device: ptr[mut Device]) {
    queue_index : u32 : 0
    vkGetDeviceQueue(device.logical_device, device.indices.present_family or_return void,
        queue_index, &device.present_queue);
    vkGetDeviceQueue(device.logical_device, device.indices.graphics_family or_return void,
        queue_index, &device.graphics_queue);
}

destroy_device :: fn(device: ptr[mut Device]) {
    vkDestroyDescriptorPool(device.physical_device, device.descriptor_pool, null)

    //NOTE(ches) queues are implicitly destroyed when the logical device is
    //NOTE(ches) physical device gets destroyed implicitly with the instance
    if !vk_is_null_handle(device.logical_device) {
        vkDestroyDevice(device.logical_device, null)
    }
}

find_queue_families :: fn(device: VkPhysicalDevice, surface: VkSurfaceKHR) -> QueueFamilyIndices {
    result : QueueFamilyIndices

    queue_family_count : u32 = 0
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, null)

    queue_families : VkQueueFamilyProperties[..]
    array_reserve(queue_families, queue_family_count)
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families)

    for queue_family, index in queue_families {
        if queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT != 0 {
            result.graphics_family = cast[VkQueueFlags](index)
        }

        present_support : b32 = false
        vkGetPhysicalDeviceSurfaceSupportKHR(device, cast[u32](index), surface, &present_support)

        if present_support {
            result.present_family = cast[VkQueueFlags](index)
        }

        if result.graphics_family is_some && result.present_family is_some {
            break
        }
    }

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
        log_fatal("Unable to create a descriptor pool")
        os.exit(-1)
    }
}

rate_device :: fn(device: VkPhysicalDevice, surface: VkSurfaceKHR) -> uint {
    score: uint = 0
    
    device_features : VkPhysicalDeviceFeatures
    vkGetPhysicalDeviceFeatures(device, &device_features)

    if !device_features.geometryShader {
        return 0
    }

    queue_families : QueueFamilyIndices : find_queue_families(device, surface)

    if queue_families.graphics_family is_none || queue_families.present_family is_none {
        return 0
    }

    if !supports_required_extensions(device) {
        return 0
    }
    
    swap_chain_support : SwapChainSupport : check_swap_chain_support(device, surface)
    if swap_chain_support.formats.count == 0 || swap_chain_support.present_modes.count == 0 {
        return 0
    }

    device_properties : VkPhysicalDeviceProperties
    vkGetPhysicalDeviceProperties(device, &device_properties)

    if device_properties.deviceType == .VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU {
        score += 1000
    }

    score += cast[uint](device_properties.limits.maxImageDimension2D)

    return score
}

select_logical_device :: fn(device: ptr[mut Device], surface: VkSurfaceKHR) {
    device.indices = find_queue_families(device.physical_device, surface)

    unique_queue_count : usize = 1
    if (device.indices.graphics_family or_else 0) != (device.indices.present_family or_else 0) {
        unique_queue_count = 2
    }

    queue_create_infos : VkDeviceQueueCreateInfo[..]
    defer array_free(queue_create_infos)
    array_reserve(queue_create_infos, unique_queue_count)
    
    queue_priorities := f32[] { 1 }

    with {
        i : usize = 0
    }
    loop {
        defer i += 1
        array_add(queue_create_infos, VkDeviceQueueCreateInfo.{
            sType = .VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            pNext = null,
            flags = 0,
            queueFamilyIndex = match i {
                0 => device.indices.graphics_family or_else 0
                1 => device.indices.present_family or_else 0
                _ => 0
            },
            queueCount = 1,
            queuePriorities = queue_priorities,
        })
    }
    while i < unique_queue_count

    device_features : VkPhysicalDeviceFeatures
    //TODO(ches) select physical device features

    enabled_validation_layers : string[] = match ENABLE_VALIDATION_LAYERS {
        true => VALIDATION_LAYERS
        false => string[0]{}
    }

    create_info := VkDeviceCreateInfo.{
        sType = .VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        pNext = null,
        flags = 0,
        queueCreateInfos = queue_create_infos,
        enabledLayerNames = enabled_validation_layers,
        enabledExtensionNames = DEVICE_REQUIRED_EXTENSIONS,
        enabledFeatures = &device_features,
    }

    if vkCreateDevice(device.physical_device, &create_info, null, &device.logical_device) != .VK_SUCCESS {
        log_fatal("Could not create a logical device")
        os.exit(-1)
    }
}

select_physical_device :: fn(instance: VkInstance, surface: VkSurfaceKHR, device: ptr[mut Device]) {
    device_count : u32 = 0

    vkEnumeratePhysicalDevices(instance, &device_count)

    if device_count == 0 {
        log_fatal("No GPUs support Vulkan")
        os.exit(-1)
    }

    devices : VkPhysicalDevice[..]
    defer array_free(devices)
    array_reserve(devices, device_count)

    vkEnumeratePhysicalDevices(instance, &device_count, devices)
    
    best_device : VkPhysicalDevice? = null
    best_score : uint = max_value(uint)
    for device in devices {
        score : uint : rate_device(device, surface)
        if score > 0 && score < best_score {
            best_device = device
            best_score = score
        }
    }

    if best_device is_none {
        log_fatal("No GPUs are suitable for this program")
        os.exit(-1)
    }

    device.physical_device = best_device or_return
}

supports_required_extensions(device: VkPhysicalDevice) -> bool {
    //TODO(ches) fill this out
    return false
}
