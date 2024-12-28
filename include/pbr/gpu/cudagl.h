// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out

template <typename PIXEL_FORMAT>
class CUDAOutputBuffer
{
public:
    CUDAOutputBuffer(int32_t width, int32_t height);
    ~CUDAOutputBuffer();

    void start_asynchronous_readback();
    const PIXEL_FORMAT* get_readback_pixels();

    void draw(int window_width, int window_height)
    {
        display->display(m_width, m_height, window_width, window_height, get_PBO());
    }

    PIXEL_FORMAT* map();
    void unmap();

private:
    void setStream(CUstream stream)
    {
        m_stream = stream;
    }

    // Allocate or update device pointer as necessary for CUDA access
    void make_current()
    {
        CUDA_CHECK(cudaSetDevice(m_device_idx));
    }

    // Get output buffer
    GLuint get_PBO();
    void deletePBO();

    int32_t m_width = 0u;
    int32_t m_height = 0u;

    cudaGraphicsResource* m_cuda_gfx_resource = nullptr;
    GLuint m_pbo = 0u;
    PIXEL_FORMAT* m_device_pixels = nullptr;
    PIXEL_FORMAT* m_host_pixels = nullptr;

    bool readback_active = false;
    cudaEvent_t readback_finished_event;

    CUstream m_stream = 0u;
    int32_t m_device_idx = 0;

    BufferDisplay* display = nullptr;
};
