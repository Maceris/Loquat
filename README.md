# Loquat

Loquat is a rendering engine written in Vulkan.

The PBR portion is based on pbrt v4.
Book: https://pbrt.org/
Code: https://github.com/mmp/pbrt-v4
License: LICENSE_pbrt.txt

## Building

This is being written in Hemera, at the time of writing we don't have a full compiler for that yet. Build info
will be updated when third party libraries and builds are better ironed out.

A partial build existed in C++ before the rewrite and is currently located on the cpp_backup branch, the below were notes related to that.

This project uses submodules for some third party libraries, and so the `--recursive` flag must be used when cloning the
repository.

```bash
$ git clone --recursive https://github.com/Maceris/Loquat.git
```

If you accidentally clone without using `--recursive`, (or to update the source tree after a new submodule has been added),
run this command to update dependencies:
```bash
$ git submodule update --init --recursive
```

The build system uses [cmake](http://www.cmake.org/). Release builds are the default, provide `-DCMAKE_BUILD_TYPE=Debug` to cmake
for a debug build.

The project also requires CUDA to be installed for GPU builds, version 11.0 or later, as well as OPTIX 7 for OptiX.

The build scripts automatically attempt to find a CUDA compiler, looking in the usual places.
The cmake output will indicate whether it was successful.
It is necessary to manually set the cmake `LOQUAT_OPTIX7_PATH` configuration option to point at an OptiX installation.
By default, the GPU shader model is set automatically based on the GPU in the system.
Alternatively, the `LOQUAT_GPU_SHADER_MODEL` option can be set manually (e.g., `-DLOQUAT_GPU_SHADER_MODEL=sm_80`).
