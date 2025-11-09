package render

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

check_swap_chain_support :: fn(device: VkPhysicalDevice) -> SwapChainSupport {
    //TODO(ches) fill this out
    result : SwapChainSupport
    return result
}

configure_surface :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

create_queues :: fn(device: ptr[mut Device]) {
    //TODO(ches) fill this out
}

find_queue_families :: fn(device: VkPhysicalDevice) -> QueueFamilyIndices {
    //TODO(ches) fill this out
    result : QueueFamilyIndices
    return result
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
