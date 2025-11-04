package window

import vulkan from "vendor"

WindowState :: struct {
    window : ptr[Window],
    window_surface : ptr[WindowSurface],
    swap_chain : ptr[SwapChain],
}

window_should_close :: fn(window_state : ptr[WindowState]) -> bool {
    //TODO(ches) Figure out GLFW's glfwWindowShouldClose(glfw_window)
    return false
}
