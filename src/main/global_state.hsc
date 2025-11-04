package main

import pipeline from "../"
import render from "../"
import vulkan from "vendor"
import window from "../"

GlobalState :: struct {
    instance : VkInstance,
    window_state : ptr[WindowState],
    device : ptr[Device],
    pipeline : ptr[Pipeline],
    render_state : ptr[RenderState],
    close_requested : bool,
}

should_close :: fn(global_state: ptr[GlobalState]) -> bool {
    return global_state.close_requested || window_should_close(global_state.window_state)
}
