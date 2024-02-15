# Loquat Rendering Engine

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(HEADER_PATH ${LOQUAT_ROOT_DIRECTORY}/include)
SET(SOURCE_PATH ${LOQUAT_ROOT_DIRECTORY}/src)
SET(RESOURCE_PATH ${LOQUAT_ROOT_DIRECTORY}/resources)
SET(IMGUI_PATH ${LOQUAT_ROOT_DIRECTORY}/third-party/imgui)

SET(SHADER_SOURCE_DIR ${RESOURCE_PATH}/shaders)
SET(CONFIGURATION_BINARY_DIR ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
SET(RESOURCE_BINARY_DIR ${CONFIGURATION_BINARY_DIR}/resources)
SET(SHADER_BINARY_DIR ${RESOURCE_BINARY_DIR}/shaders)

SET(PUBLIC_HEADERS
  ${HEADER_PATH}/debug/logger.h
  ${HEADER_PATH}/device/device.h
  ${HEADER_PATH}/main/global_state.h
  ${HEADER_PATH}/main/loquat.h
  ${HEADER_PATH}/main/memory_utils.h
  ${HEADER_PATH}/main/vulkan_instance.h
  ${HEADER_PATH}/pbr/bsdf.h
  ${HEADER_PATH}/pbr/bxdfs.h
  ${HEADER_PATH}/pbr/films.h
  ${HEADER_PATH}/pbr/light_samplers.h
  ${HEADER_PATH}/pbr/shapes.h
  ${HEADER_PATH}/pbr/base/aggregates.h
  ${HEADER_PATH}/pbr/base/bssrdf.h
  ${HEADER_PATH}/pbr/base/bxdf.h
  ${HEADER_PATH}/pbr/base/camera.h
  ${HEADER_PATH}/pbr/base/film.h
  ${HEADER_PATH}/pbr/base/filter.h
  ${HEADER_PATH}/pbr/base/integrator.h
  ${HEADER_PATH}/pbr/base/light.h
  ${HEADER_PATH}/pbr/base/light_sampler.h
  ${HEADER_PATH}/pbr/base/material.h
  ${HEADER_PATH}/pbr/base/medium.h
  ${HEADER_PATH}/pbr/base/primitive.h
  ${HEADER_PATH}/pbr/base/render.h
  ${HEADER_PATH}/pbr/base/sampler.h
  ${HEADER_PATH}/pbr/base/shape.h
  ${HEADER_PATH}/pbr/base/spectrum.h
  ${HEADER_PATH}/pbr/base/texture.h
  ${HEADER_PATH}/pbr/math/aabb.h
  ${HEADER_PATH}/pbr/math/float.h
  ${HEADER_PATH}/pbr/math/hash.h
  ${HEADER_PATH}/pbr/math/interval.h
  ${HEADER_PATH}/pbr/math/math.h
  ${HEADER_PATH}/pbr/math/matrix.h
  ${HEADER_PATH}/pbr/math/point.h
  ${HEADER_PATH}/pbr/math/quaternion.h
  ${HEADER_PATH}/pbr/math/ray.h
  ${HEADER_PATH}/pbr/math/sampling.h
  ${HEADER_PATH}/pbr/math/transform.h
  ${HEADER_PATH}/pbr/math/vec.h
  ${HEADER_PATH}/pbr/math/vector_math.h
  ${HEADER_PATH}/pbr/struct/image.h
  ${HEADER_PATH}/pbr/struct/interaction.h
  ${HEADER_PATH}/pbr/struct/parameter_dictionary.h
  ${HEADER_PATH}/pbr/struct/soa.h
  ${HEADER_PATH}/pbr/util/color.h
  ${HEADER_PATH}/pbr/util/color_space.h
  ${HEADER_PATH}/pbr/util/parallel.h
  ${HEADER_PATH}/pbr/util/tagged_pointer.h
  ${HEADER_PATH}/pipeline/pipeline.h
  ${HEADER_PATH}/render/render.h
  ${HEADER_PATH}/render/render_state.h
  ${HEADER_PATH}/resource/default_resource_loader.h
  ${HEADER_PATH}/resource/resource.h
  ${HEADER_PATH}/resource/resource_cache.h
  ${HEADER_PATH}/resource/resource_file.h
  ${HEADER_PATH}/resource/resource_file_folder.h
  ${HEADER_PATH}/resource/resource_handle.h
  ${HEADER_PATH}/resource/resource_loader.h
  ${HEADER_PATH}/shader/shader.h
  ${HEADER_PATH}/window/swap_chain.h
  ${HEADER_PATH}/window/window.h
  ${HEADER_PATH}/window/window_state.h
  ${HEADER_PATH}/window/window_surface.h
)

SET(CORE_SRCS
  ${SOURCE_PATH}/debug/logger.cpp
  ${SOURCE_PATH}/device/device.cpp
  ${SOURCE_PATH}/main/global_state.cpp
  ${SOURCE_PATH}/main/loquat.cpp
  ${SOURCE_PATH}/main/vulkan_instance.cpp
  ${SOURCE_PATH}/pbr/base/integrator.cpp
  ${SOURCE_PATH}/pbr/math/transform.cpp
  ${SOURCE_PATH}/pbr/struct/interaction.cpp
  ${SOURCE_PATH}/pipeline/pipeline.cpp
  ${SOURCE_PATH}/render/render.cpp
  ${SOURCE_PATH}/render/render_state.cpp
  ${SOURCE_PATH}/resource/default_resource_loader.cpp
  ${SOURCE_PATH}/resource/resource.cpp
  ${SOURCE_PATH}/resource/resource_cache.cpp
  ${SOURCE_PATH}/resource/resource_file_folder.cpp
  ${SOURCE_PATH}/resource/resource_handle.cpp
  ${SOURCE_PATH}/resource/resource_loader.cpp
  ${SOURCE_PATH}/shader/shader.cpp
  ${SOURCE_PATH}/window/swap_chain.cpp
  ${SOURCE_PATH}/window/window.cpp
  ${SOURCE_PATH}/window/window_state.cpp
  ${SOURCE_PATH}/window/window_surface.cpp
)

SET(IMGUI_HEADERS
  ${IMGUI_PATH}/imconfig.h
  ${IMGUI_PATH}/imgui.h
  ${IMGUI_PATH}/imgui_internal.h
  ${IMGUI_PATH}/imstb_rectpack.h
  ${IMGUI_PATH}/imstb_textedit.h
  ${IMGUI_PATH}/imstb_truetype.h
  ${IMGUI_PATH}/backends/imgui_impl_glfw.h
  ${IMGUI_PATH}/backends/imgui_impl_vulkan.h
  ${IMGUI_PATH}/backends/imgui_impl_win32.h
  ${IMGUI_PATH}/misc/cpp/imgui_stdlib.h
)

INCLUDE_DIRECTORIES(BEFORE
	${IMGUI_PATH}/
	${IMGUI_PATH}/backends
	${IMGUI_PATH}/misc/cpp
)

SET(IMGUI_SRCS
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui_demo.cpp
  ${IMGUI_PATH}/imgui_draw.cpp
  ${IMGUI_PATH}/imgui_tables.cpp
  ${IMGUI_PATH}/imgui_widgets.cpp
  ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
  ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
  ${IMGUI_PATH}/backends/imgui_impl_win32.cpp
  ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp
)

SOURCE_GROUP(TREE ${HEADER_PATH} PREFIX "include" FILES ${PUBLIC_HEADERS})
SOURCE_GROUP(TREE ${SOURCE_PATH} PREFIX "src" FILES ${CORE_SRCS})
SOURCE_GROUP(imgui PREFIX "imgui" FILES ${IMGUI_SRCS} ${IMGUI_HEADERS})

SET(loquat_src
  ${CORE_SRCS}
  ${IMGUI_SRCS}
  
  # Necessary to show the headers in the project when using the VC++ generator:
  ${PUBLIC_HEADERS}
  ${IMGUI_HEADERS}
)

ADD_EXECUTABLE(loquat ${loquat_src})

SET_PROPERTY(TARGET loquat PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CONFIGURATION_BINARY_DIR})

IF (LOQUAT_WARNINGS_AS_ERRORS)
  MESSAGE(STATUS "Treating all warnings as errors (for loquat library only)")
  IF (MSVC)
    IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # clang-cl
      TARGET_COMPILE_OPTIONS(loquat PRIVATE -Wall -Werror
        -Wno-microsoft-enum-value
        -Wno-switch-enum
        -Wno-covered-switch-default
        -Wno-reserved-identifier
        -Wno-c++98-compat-pedantic
        -Wno-c++98-compat
        -Wno-documentation
        -Wno-documentation-unknown-command
        -Wno-deprecated-dynamic-exception-spec
        -Wno-undef
        -Wno-suggest-destructor-override
        -Wno-suggest-override
        -Wno-zero-as-null-pointer-constant
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-extra-semi-stmt
        -Wno-missing-prototypes
        -Wno-old-style-cast
        -Wno-cast-align
        -Wno-cast-qual
        -Wno-float-equal
        -Wno-implicit-int-float-conversion
        -Wno-sign-conversion
        -Wno-implicit-float-conversion
        -Wno-implicit-int-conversion
        -Wno-float-conversion
        -Wno-double-promotion
        -Wno-unused-macros
        -Wno-disabled-macro-expansion
        -Wno-shadow-field
        -Wno-shadow
        -Wno-language-extension-token
        -Wno-header-hygiene
        -Wno-tautological-value-range-compare
        -Wno-tautological-type-limit-compare
        -Wno-missing-variable-declarations
        -Wno-extra-semi
        -Wno-nonportable-system-include-path
        -Wno-undefined-reinterpret-cast
        -Wno-shift-sign-overflow
        -Wno-deprecated
        -Wno-format-nonliteral
        -Wno-comma
        -Wno-implicit-fallthrough
        -Wno-unused-template
        -Wno-undefined-func-template
        -Wno-declaration-after-statement
     )
    ELSE()
      TARGET_COMPILE_OPTIONS(loquat PRIVATE /W4 /WX)
    ENDIF()
  ELSE()
    TARGET_COMPILE_OPTIONS(loquat PRIVATE -Wall -Werror)
  ENDIF()
ENDIF()

# adds C_FLAGS required to compile zip.c on old GCC 4.x compiler
TARGET_COMPILE_FEATURES(loquat PRIVATE c_std_99)

FIND_PACKAGE(Vulkan REQUIRED COMPONENTS glslc)
FIND_PROGRAM(glslc_executable NAMES glslc HINTS Vulkan::glslc)

TARGET_LINK_LIBRARIES(loquat glfw Vulkan::Vulkan glm)

TARGET_INCLUDE_DIRECTORIES (loquat PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
  $<BUILD_INTERFACE:${GLFW_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${LOQUAT_INCLUDE_INSTALL_DIR}>
)

# Copy DLL files on platforms where it's needed 
IF (CMAKE_IMPORT_LIBRARY_SUFFIX) 
	ADD_CUSTOM_COMMAND(TARGET loquat POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:loquat> $<TARGET_FILE_DIR:loquat> COMMAND_EXPAND_LISTS)
ENDIF()

IF(MSVC)
  # in order to prevent DLL hell, each of the DLLs are suffixed with the major version and msvc prefix
  # CMake 3.12 added a variable for this
  IF(MSVC_TOOLSET_VERSION)
    SET(MSVC_PREFIX "vc${MSVC_TOOLSET_VERSION}")
  ELSE()
    IF(MSVC70 OR MSVC71)
      SET(MSVC_PREFIX "vc70")
    ELSEIF(MSVC80)
      SET(MSVC_PREFIX "vc80")
    ELSEIF(MSVC90)
      SET(MSVC_PREFIX "vc90")
    ELSEIF(MSVC10)
      SET(MSVC_PREFIX "vc100")
    ELSEIF(MSVC11)
      SET(MSVC_PREFIX "vc110")
    ELSEIF(MSVC12)
      SET(MSVC_PREFIX "vc120")
    ELSEIF(MSVC_VERSION LESS 1910)
      SET(MSVC_PREFIX "vc140")
    ELSEIF(MSVC_VERSION LESS 1920)
      SET(MSVC_PREFIX "vc141")
    ELSEIF(MSVC_VERSION LESS 1930)
      SET(MSVC_PREFIX "vc142")
    ELSE()
      MESSAGE(WARNING "unknown msvc version ${MSVC_VERSION}")
      SET(MSVC_PREFIX "vc150")
    ENDIF()
  ENDIF()
  SET(LIBRARY_SUFFIX "${PROJECT_VERSION_MAJOR}-${MSVC_PREFIX}" CACHE STRING "the suffix for the loquat windows library")
ENDIF()

IF(MINGW)
  SET(LIBRARY_SUFFIX "-${LOQUAT_SOVERSION}" CACHE STRING "the suffix for the loquat MinGW shared library")
  SET_TARGET_PROPERTIES(loquat PROPERTIES
    ARCHIVE_OUTPUT_NAME loquat
 )
  IF(NOT BUILD_SHARED_LIBS)
    TARGET_LINK_LIBRARIES (loquat -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++)
  ENDIF ()
ENDIF()

SET_TARGET_PROPERTIES(loquat PROPERTIES
  VERSION ${LOQUAT_VERSION}
  SOVERSION ${LOQUAT_SOVERSION}
  OUTPUT_NAME loquat${LIBRARY_SUFFIX}
)

INSTALL(TARGETS loquat
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION ${LOQUAT_LIB_INSTALL_DIR} COMPONENT ${LIBLOQUAT_COMPONENT}
  ARCHIVE DESTINATION ${LOQUAT_LIB_INSTALL_DIR} COMPONENT ${LIBLOQUAT-DEV_COMPONENT}
  RUNTIME DESTINATION ${LOQUAT_BIN_INSTALL_DIR} COMPONENT ${LIBLOQUAT_COMPONENT}
  FRAMEWORK DESTINATION ${LOQUAT_LIB_INSTALL_DIR} COMPONENT ${LIBLOQUAT_COMPONENT}
  INCLUDES DESTINATION ${LOQUAT_INCLUDE_INSTALL_DIR}
)

INSTALL(FILES ${PUBLIC_HEADERS} DESTINATION ${LOQUAT_INCLUDE_INSTALL_DIR}/loquat COMPONENT loquat-dev)

if(MSVC AND LOQUAT_INSTALL_PDB)
  # When only the static library is built, these properties must
  # be set to ensure the static lib .pdb is staged for installation.
  IF(NOT BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES(loquat PROPERTIES
      COMPILE_PDB_OUTPUT_DIRECTORY ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}
      COMPILE_PDB_NAME loquat${LIBRARY_SUFFIX}
      COMPILE_PDB_NAME_DEBUG loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}
    )
    IF(GENERATOR_IS_MULTI_CONFIG)
      install(FILES ${CONFIGURATION_BINARY_DIR}/loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS Debug
      )
    ELSE()
      install(FILES ${CONFIGURATION_BINARY_DIR}/loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS Debug
      )
    ENDIF()
  ELSE()
    install(FILES $<TARGET_PDB_FILE:loquat>
      DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
    )
  ENDIF()
ENDIF ()

FILE(GLOB SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss
)

ADD_CUSTOM_COMMAND(COMMAND
  ${CMAKE_COMMAND} -E make_directory ${RESOURCE_BINARY_DIR}
  OUTPUT ${RESOURCE_BINARY_DIR}
  COMMENT "Creating resource output directory ${RESOURCE_BINARY_DIR}"
)

ADD_CUSTOM_COMMAND(COMMAND
  ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating shader output directory ${SHADER_BINARY_DIR}"
)

FOREACH(source IN LISTS SHADERS)
  GET_FILENAME_COMPONENT(FILENAME ${source} NAME)
  ADD_CUSTOM_COMMAND(
    COMMAND
      ${glslc_executable}
      # -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling shader ${FILENAME}"
  )
  LIST(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
ENDFOREACH()

ADD_CUSTOM_TARGET(shaders ALL DEPENDS ${SPV_SHADERS})
TARGET_USE_COMMON_OUTPUT_DIRECTORY(loquat)