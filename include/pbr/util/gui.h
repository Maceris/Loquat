// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out

#include <set>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "main/loquat.h"

#ifdef LOQUAT_BUILD_GPU_RENDERER
#include "pbr/gpu/cudagl.h"
#endif

#include "pbr/util/color.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
    enum DisplayState
    {
        EXIT,
        RESET,
        NONE
    };

    class GUI
    {
    public:
        GUI(std::string title, Vec2i resolution, AABB3f sceneBounds);
        ~GUI();

        RGB* map_framebuffer()
        {
#ifdef LOQUAT_BUILD_GPU_RENDERER
            if (cuda_framebuffer)
            {
                return cuda_framebuffer->map();
            }
            else
#endif
            {
                return cpu_framebuffer;
            }
        }

        void unmap_framebuffer()
        {
#ifdef LOQUAT_BUILD_GPU_RENDERER
            if (cuda_framebuffer)
            {
                cuda_framebuffer->unmap();
            }
#endif
        }

        DisplayState refresh_display();

        // It's a little messy that the state of values controlled via the UI
        // are just public variables here but it's probably not worth putting
        // an abstraction layer on top of all this at this point.
        Transform get_camera_transform() const { return moving_from_camera; }
        Float exposure = 1.f;
        bool print_camera_transform = false;

        void keyboard_callback(GLFWwindow* window, int key, int scan, int action, int mods);
        void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
        void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

        static void initialize();
        static Point2i get_resolution();

    private:
        bool process_keys();
        bool process_mouse();
        bool process();

        std::set<char> keys_down;
        Float move_scale = 1.f;
        Transform moving_from_camera;
        Vec2i resolution;
        bool record_frames = false;
        int frame_number = 0;
        bool pressed = false;
        Float xoffset = 0.f;
        Float yoffset = 0.f;
        double lastX = 0.f;
        double lastY = 0.f;

#ifdef LOQUAT_BUILD_GPU_RENDERER
        CUDAOutputBuffer<RGB>* cuda_framebuffer = nullptr;
#endif
        RGB* cpu_framebuffer = nullptr;
        GLFWwindow* window = nullptr;
    };

}