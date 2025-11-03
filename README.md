# Loquat

# This is a backup uf the main branch, before we switched to Hemera

Loquat is a rendering engine written in Vulkan.

The PBR portion is based on pbrt v4.
Book: https://pbrt.org/
Code: https://github.com/mmp/pbrt-v4
License: LICENSE_pbrt.txt

## Building

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

To create the project or solution files for the project, you can run `regenerate-project.bat` or `regenerate-project.sh`. 
This will generate all of the project files in the `bin` folder. These can be rerun to wipe out the whole bin directory
again and regenerate the project contents.

If using Visual Studio, you can open the Loquat solution and build and run it from there.
