package render

import vulkan from "vendor"

WindowState :: struct {
    window : Window,
    window_surface : WindowSurface,
    swap_chain : ptr[SwapChain],
}

create_vulkan_window :: fn() {
    //TODO(ches) fill this out
}

window_should_close :: fn(window_state : ptr[WindowState]) -> bool {
    //TODO(ches) Figure out GLFW's glfwWindowShouldClose(glfw_window)
    return false
}
