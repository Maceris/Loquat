package main

cleanup :: fn(global_state: ptr[GlobalState]) {
    //TODO(ches) complete
}

initialize :: fn(global_state: ptr[GlobalState]) {
    //TODO(ches) complete
}

main :: fn() {
    global_state : ptr[GlobalState] = new(GlobalState)

    initialize(global_state)
    load_scene(global_state)
    setup_scene(global_state)

    loop {
        //TODO(ches) glfwPollEvents()
        render_scene(global_state)
    }
    while (!should_close(global_state))

    cleanup(global_state)
}

load_scene :: fn(global_state: ptr[GlobalState]) {
    //TODO(ches) complete
}

render_scene :: fn(global_state: ptr[GlobalState]) {
    //TODO(ches) complete
}

setup_scene :: fn(global_state: ptr[GlobalState]) {
    //TODO(ches) complete
}
