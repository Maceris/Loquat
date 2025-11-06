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
