# Loquat Rendering Engine

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Loquat VERSION 1.0.0)

# Options #####################################################################

OPTION(BUILD_SHARED_LIBS
  "Build package with shared libraries"
  ON
)

OPTION(LOQUAT_BUILD_GPU_RENDERER
  "Include CUDA acceleration for software rendering"
  ON
)

OPTION(LOQUAT_INSTALL
  "Disable this if you want to use Loquat as a submodule"
  ON
)

OPTION(ADDRESS_SANITIZER
  "Enable address sanitizer"
  OFF
)

OPTION(UB_SANITIZER
  "Enable Undefined Behavior sanitizer."
  OFF
)

OPTION(LOQUAT_USE_WAYLAND
  "Use Wayland instead of X11 on linux"
  OFF
)

OPTION(DOUBLE_PRECISION_FLOAT
  "Use double-precision floating points"
  OFF
)

IF(DOUBLE_PRECISION_FLOAT)
  ADD_DEFINITIONS(-DDOUBLE_PRECISION_FLOAT)
ENDIF()

# Use solution folders.
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(CMAKE_SYSTEM_NAME STREQUAL Windows)
  # Use subsetof Windows.h
  ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
  ADD_DEFINITIONS(-DLOQUAT_IS_WINDOWS)
  ADD_DEFINITIONS(-DNOMINMAX)
  ADD_DEFINITIONS(-DLOQUAT_HAS_INTRIN_H)

  IF(MSVC)
    IF(NOT (MSVC_VERSION LESS 1900))
      # Multibyte character set is deprecated since
	  # at least MSVC2015 (possibly earlier)
      ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
    ENDIF()
  ENDIF()
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  ADD_DEFINITIONS(-DLOQUAT_IS_OSX)
  ADD_DEFINITIONS(-Wno-deprecated-declarations)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL Linux)
  ADD_DEFINITIONS(-DLOQUAT_IS_LINUX)
  # -rdynamic for backtrace symbols
  # --no-as-needed so libprofiler sticks around
  STRING(APPEND CMAKE_EXE_LINKER_FLAGS " -rdynamic -Wl,--no-as-needed")
ELSE()
  MESSAGE(SEND_ERROR "Unknown system name: " + CMAKE_SYSTEM_NAME)
ENDIF()

IF(BUILD_SHARED_LIBS)
  MESSAGE(STATUS "Shared libraries enabled")
ELSE()
  MESSAGE(STATUS "Shared libraries disabled")
  SET(LINK_SEARCH_START_STATIC TRUE)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES 
    ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES}
  )
ENDIF()

SET(LOQUAT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET(LOQUAT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET(LOQUAT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
SET(LOQUAT_VERSION ${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}.${LOQUAT_VERSION_PATCH})
SET(LOQUAT_SOVERSION 1)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD 99)

INCLUDE_DIRECTORIES(BEFORE
  ./
  src/
  include/
)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
SET(LIBLOQUAT_COMPONENT "libloquat${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}.${LOQUAT_VERSION_PATCH}")
SET(LIBLOQUAT-DEV_COMPONENT "libloquat${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}.${LOQUAT_VERSION_PATCH}-dev")
SET(CPACK_COMPONENTS_ALL loquat-bin ${LIBLOQUAT_COMPONENT} ${LIBLOQUAT-DEV_COMPONENT} loquat-dev)

IF(UNIX)
  # Use GNUInstallDirs for Unix predefined directories
  INCLUDE(GNUInstallDirs)
  # Ensure that we do not run into issues like http://www.tcm.phy.cam.ac.uk/sw/inodes64.html on 32 bit linux
  IF(NOT ${OPERATING_SYSTEM} MATCHES "Android")
    IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	  # only necessary for 32-bit linux
      ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
    ENDIF()
  ENDIF()
ENDIF()

# Grouped compiler settings ###################################################
IF ((CMAKE_C_COMPILER_ID MATCHES "GNU") AND NOT MINGW AND NOT HAIKU)
  SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
  
  IF(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 13)
    MESSAGE(STATUS "GCC13 detected, disabling \"-Wdangling-reference\" in CPP files as it appears to be a false positive")
    ADD_COMPILE_OPTIONS("$<$<COMPILE_LANGUAGE:CXX>:-Wno-dangling-reference>")
  ENDIF()
  # hide all non-exported symbols
  IF(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "-mxgot -fvisibility=hidden -fno-strict-aliasing -Wall ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
    SET(LIBSTDC++_LIBRARIES -lstdc++)
  ELSE()
    SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
    SET(LIBSTDC++_LIBRARIES -lstdc++)
  ENDIF()
ELSEIF(MSVC)
  ADD_DEFINITIONS(-DLOQUAT_IS_MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
  
  # enable multi-core compilation with MSVC
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # clang-cl
    ADD_COMPILE_OPTIONS(/bigobj)
	IF(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
	  SET(CMAKE_CXX_FLAGS "-ffp-contract=off ${CMAKE_CXX_FLAGS}")
	ENDIF()
  ELSE()
    # msvc
    ADD_COMPILE_OPTIONS(/MP /bigobj)
  ENDIF()
  
  # disable "elements of array '' will be default initialized" warning on MSVC2013
  IF(MSVC12)
    ADD_COMPILE_OPTIONS(/wd4351)	
  ENDIF()
  # supress warning for double to float conversion if Double precission is activated
  ADD_COMPILE_OPTIONS(/wd4244)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /Zi /Od")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /PDBALTPATH:%_PDB% /OPT:REF /OPT:ICF")
ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
  SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ELSEIF(MINGW)
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    message(FATAL_ERROR "MinGW is too old to be supported. Please update MinGW and try again.")
  ELSEIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.3)
    message(WARNING "MinGW is old, if you experience errors, please update MinGW.")
  ENDIF()
  SET(CMAKE_CXX_FLAGS "-std=gnu++20 ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long -Wa,-mbig-obj -g ${CMAKE_CXX_FLAGS}")
  ELSE()
    SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long -Wa,-mbig-obj -O3 ${CMAKE_CXX_FLAGS}")
  ENDIF()
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ENDIF()

IF (LOQUAT_BUILD_GPU_RENDERER)
  ADD_DEFINITIONS(-DLOQUAT_BUILD_GPU_RENDERER)
ENDIF()

IF (ADDRESS_SANITIZER)
  MESSAGE(STATUS "Address sanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fsanitize=address")
ENDIF()

IF (UB_SANITIZER)
  MESSAGE(STATUS "Undefined Behavior sanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,shift,shift-exponent,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr,pointer-overflow,builtin -fno-sanitize-recover=all")
  SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fsanitize=undefined,shift,shift-exponent,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr,pointer-overflow,builtin -fno-sanitize-recover=all")
ENDIF()

add_library (loquat_warnings INTERFACE)
target_compile_options (
    loquat_warnings
    INTERFACE
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4244>"  # int -> float conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4267>"  # size_t -> int conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4305>"  # double constant assigned to float
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4552>"  # result of expression not used
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4838>"  # double -> int conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4843>"  # double -> float conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd26451>" # arithmetic on 4-byte value, then cast to 8-byte
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd26495>" # uninitialized member variable
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4334>"  # 32 to 64 bit displacement
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4146>"  # NanoVDB: unary minus operator applied to unsigned type, result still unsigned
)

add_library (loquat_opt INTERFACE)

###########################################################################
# Check for various C++ features and set preprocessor variables or
# define workarounds.

include (CheckCXXSourceCompiles)

check_cxx_source_compiles ("
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
int main() {
   int fd = open(\"foo\", O_RDONLY);
   struct stat s;
   fstat(fd, &s);
   size_t len = s.st_size;
   void *ptr = mmap(0, len, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
   munmap(ptr, len);   
}
" HAVE_MMAP)

if (HAVE_MMAP)
  ADD_DEFINITIONS(-DLOQUAT_HAVE_MMAP)
ENDIF ()

include (CheckIncludeFiles)

check_cxx_source_compiles ("
#include <intrin.h>
int main() {
    unsigned long lz = 0, v = 1234;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
} " HAS_INTRIN_H)

if (HAS_INTRIN_H)
  ADD_DEFINITIONS(-DLOQUAT_HAS_INTRIN_H)
endif ()

########################################
# noinline

check_cxx_source_compiles (
"__declspec(noinline) void foo() { }
int main() { }"
HAVE_DECLSPEC_NOINLINE)

check_cxx_source_compiles (
"__attribute__((noinline)) void foo() { }
int main() { }"
HAVE_ATTRIBUTE_NOINLINE)

if (HAVE_ATTRIBUTE_NOINLINE)
  ADD_DEFINITIONS("-DLOQUAT_NOINLINE=__attribute__((noinline))")
elseif (HAVE_DECLSPEC_NOINLINE)
  ADD_DEFINITIONS("-DLOQUAT_NOINLINE=__declspec(noinline)")
else ()
  ADD_DEFINITIONS(-DLOQUAT_NOINLINE)
endif ()

########################################
# restrict

check_cxx_source_compiles (
"int * __restrict__ ptr;
int main() { }"
HAVE_PRE_POST_BAR_RESTRICT)

check_cxx_source_compiles (
"int * __restrict ptr;
int main() { }"
HAVE_PRE_BAR_RESTRICT)

if (HAVE_PRE_POST_BAR_RESTRICT)
  ADD_DEFINITIONS("-DLOQUAT_RESTRICT=__restrict__")
elseif (HAVE_PRE_BAR_RESTRICT)
  ADD_DEFINITIONS("-DLOQUAT_RESTRICT=__restrict")
else ()
  ADD_DEFINITIONS(-DLOQUAT_RESTRICT)
endif ()

########################################
# Aligned memory allocation

check_cxx_source_compiles ( "
#include <malloc.h>
int main() { void * ptr = _aligned_malloc(1024, 32); }
" HAVE__ALIGNED_MALLOC )

check_cxx_source_compiles ( "
#include <stdlib.h>
int main() {
  void *ptr;
  posix_memalign(&ptr, 32, 1024);
} " HAVE_POSIX_MEMALIGN )

if (HAVE__ALIGNED_MALLOC)
  ADD_DEFINITIONS(-DLOQUAT_HAVE__ALIGNED_MALLOC)
elseif (HAVE_POSIX_MEMALIGN)
  ADD_DEFINITIONS(-DLOQUAT_HAVE_POSIX_MEMALIGN)
else ()
  message (SEND_ERROR "Unable to find a way to allocate aligned memory")
endif ()

########################################
# are long and int64_t the same

check_cxx_source_compiles ("
#include <cstdint>
#include <type_traits>
static_assert(!std::is_same<long, int64_t>::value && !std::is_same<long long, int64_t>::value);
int main() { }
" INT64_IS_OWN_TYPE)

if (INT64_IS_OWN_TYPE)
  ADD_DEFINITIONS(-DLOQUAT_INT64_IS_OWN_TYPE)
endif ()

if (LOQUAT_NVTX)
  ADD_DEFINITIONS(-DNVTX)
endif()

INCLUDE(PrecompiledHeader)

# Set Loquat project output directory variables.
# Will respect top-level CMAKE_*_OUTPUT_DIRECTORY variables if any are set.
IF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  SET(LOQUAT_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" CACHE STRING "Path for runtime output files")
ELSE()
  SET(LOQUAT_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/" CACHE STRING "Path for runtime output files")
ENDIF()

IF(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  SET(LOQUAT_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" CACHE STRING "Path for library output files")
ELSE()
  SET(LOQUAT_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" CACHE STRING "Path for library output files")
ENDIF()

IF(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  SET(LOQUAT_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" CACHE STRING "Path for archive output files")
ELSE()
  SET(LOQUAT_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/" CACHE STRING "Path for archive output files")
ENDIF()

# Macro used to set the output directories of a target to the
# respective Loquat output directories.
MACRO(TARGET_USE_COMMON_OUTPUT_DIRECTORY target)
  SET_TARGET_PROPERTIES(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}
      LIBRARY_OUTPUT_DIRECTORY ${LOQUAT_LIBRARY_OUTPUT_DIRECTORY}
      ARCHIVE_OUTPUT_DIRECTORY ${LOQUAT_ARCHIVE_OUTPUT_DIRECTORY}
  )
ENDMACRO()

INCLUDE_DIRECTORIES(BEFORE
	${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/
	${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/src
)

GET_CMAKE_PROPERTY(is_multi_config GENERATOR_IS_MULTI_CONFIG)

# Only generate this target if no higher-level project already has
IF (NOT TARGET uninstall AND LOQUAT_INSTALL)
  # add make uninstall capability
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
  ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
ENDIF()

SET(LOQUAT_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
SET(LOQUAT_LIB_INSTALL_DIR ${LOQUAT_LIBRARY_OUTPUT_DIRECTORY})
SET(LOQUAT_INCLUDE_INSTALL_DIR "include")
SET(LOQUAT_BIN_INSTALL_DIR ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY})

SET(LOQUAT_THIRD_PARTY_DIR ${LOQUAT_ROOT_DIRECTORY}/third-party)

SET(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

SET(CONFIG_INSTALL_DIR "${LOQUAT_LIB_INSTALL_DIR}/cmake/loquat-${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}")
SET(CMAKE_CONFIG_TEMPLATE_FILE "cmake-modules/loquat-plain-config.cmake.in")
STRING(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
SET(NAMESPACE "${PROJECT_NAME_LOWERCASE}::")
SET(TARGETS_EXPORT_NAME "${PROJECT_NAME_LOWERCASE}Targets")
SET(VERSION_CONFIG "${GENERATED_DIR}/${PROJECT_NAME_LOWERCASE}ConfigVersion.cmake")
SET(PROJECT_CONFIG "${GENERATED_DIR}/${PROJECT_NAME_LOWERCASE}Config.cmake")

SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT loquat)

# Include module with fuction 'write_basic_package_version_file'
INCLUDE(CMakePackageConfigHelpers)

# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file("${VERSION_CONFIG}" COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${CMAKE_CONFIG_TEMPLATE_FILE}
  "${PROJECT_CONFIG}"
  INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)

INSTALL(
  FILES "${PROJECT_CONFIG}" "${VERSION_CONFIG}"
  DESTINATION "${CONFIG_INSTALL_DIR}"
  COMPONENT ${LIBLOQUAT-DEV_COMPONENT}
)

INSTALL(
  EXPORT "${TARGETS_EXPORT_NAME}"
  NAMESPACE "${NAMESPACE}"
  DESTINATION "${CONFIG_INSTALL_DIR}"
  COMPONENT ${LIBLOQUAT-DEV_COMPONENT}
)

SET (LOQUAT_BUILD_ARCHITECTURE "" CACHE STRING "describes the current architecture")
IF(NOT LOQUAT_BUILD_ARCHITECTURE STREQUAL "")
  ADD_DEFINITIONS(-D'LOQUAT_BUILD_ARCHITECTURE="${LOQUAT_BUILD_ARCHITECTURE}"')
ENDIF()

SET (LOQUAT_BUILD_COMPILER "" CACHE STRING "describes the current compiler")
IF(NOT LOQUAT_BUILD_COMPILER STREQUAL "")
  ADD_DEFINITIONS(-D'LOQUAT_BUILD_COMPILER="${LOQUAT_BUILD_COMPILER}"')
ENDIF()

MARK_AS_ADVANCED (LOQUAT_BUILD_ARCHITECTURE LOQUAT_BUILD_COMPILER)

function (CHECK_EXT NAME DIR HASH)
  if (NOT IS_DIRECTORY "${LOQUAT_THIRD_PARTY_DIR}/${DIR}")
    message (FATAL_ERROR "The ${NAME} submodule directory is missing! "
      "Either that submodule was recently added or you did not clone the project with --recursive. "
      "In order to update the submodules, run:\n"
      "  \"git submodule update --init --recursive\"")
  endif ()

  find_package(Git)
  if (GIT_FOUND)
    execute_process (
      COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      WORKING_DIRECTORY "${LOQUAT_THIRD_PARTY_DIR}/${DIR}"
      RESULT_VARIABLE "git_return"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE "git_hash")
    if (NOT ${git_hash} MATCHES "^${HASH}")
      message (FATAL_ERROR "The ${LOQUAT_THIRD_PARTY_DIR}/${DIR} "
        "submodule isn't up to date (${git_hash} vs ${HASH}). Please run:\n"
        "  \"git submodule update --recursive\"")
    else ()
      message (STATUS "${NAME} at commit: ${git_hash}")
    endif()
  else (GIT_FOUND)
    message (STATUS "git not found: unable to verify revisions in submodules")
  endif (GIT_FOUND)
endfunction ()

check_ext ("OpenEXR" "openexr/OpenEXR" 5cfb5dab6dfada731586b0281bdb15ee75e26782)
check_ext ("OpenVDB" "openvdb/nanovdb" 414bed84c2fc22e188eac7b611aa85c7edd7a5a9)
check_ext ("Ptex" "ptex/src" 4cd8e9a6db2b06e478dfbbd8c26eb6df97f84483)
check_ext ("double-conversion" "double-conversion/cmake" cc1f75a114aca8d2af69f73a5a959aecbab0e87a)
check_ext ("filesystem" "filesystem/filesystem" c5f9de30142453eb3c6fe991e82dfc2583373116)
check_ext ("libdeflate" "libdeflate/common" 1fd0bea6ca2073c68493632dafc4b1ddda1bcbc3)
check_ext ("lodepng" "lodepng/examples" 8c6a9e30576f07bf470ad6f09458a2dcd7a6a84a)
check_ext ("qoi" "qoi" 028c75fd26e5e0758c7c711216c00404994c1ad3)
check_ext ("stb" "stb/tools" af1a5bc352164740c1cc1354942b1c6b72eacb8a)
check_ext ("utf8proc" "utf8proc/bench" 2484e2ed5e1d9c19edcccf392a7d9920ad90dfaf)
check_ext ("zlib" "zlib/doc" 54d591eabf9fe0e84c725638f8d5d8d202a093fa)

# 3rd Party Libraries
ADD_SUBDIRECTORY(third-party/glfw/)
ADD_SUBDIRECTORY(third-party/glm/)

# Main Loquat code
ADD_SUBDIRECTORY(src/)

# Generate a pkg-config.pc, and revision.h for the Loquat library.
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/loquat.pc.in" "${PROJECT_BINARY_DIR}/loquat.pc" @ONLY)
IF (LOQUAT_INSTALL)
  INSTALL(FILES "${PROJECT_BINARY_DIR}/loquat.pc" DESTINATION ${LOQUAT_LIB_INSTALL_DIR}/pkgconfig/ COMPONENT ${LIBLOQUAT-DEV_COMPONENT})
ENDIF()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/revision.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/revision.h
)

if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(BIN_DIR "${PROJECT_SOURCE_DIR}/bin64/")
    SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib64/")
  else()
    SET(BIN_DIR "${PROJECT_SOURCE_DIR}/bin32/")
    SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib32/")
  ENDIF()

  IF(MSVC_TOOLSET_VERSION)
    SET(MSVC_PREFIX "vc${MSVC_TOOLSET_VERSION}")
    SET(LOQUAT_MSVC_VERSION ${MSVC_PREFIX})
  ELSE()
    IF(MSVC12)
      SET(LOQUAT_MSVC_VERSION "vc120")
    ELSEIF(MSVC14)
      SET(LOQUAT_MSVC_VERSION "vc140")
    ELSEIF(MSVC15)
      SET(LOQUAT_MSVC_VERSION "vc141")
    ELSEIF(MSV142)
      SET(LOQUAT_MSVC_VERSION "vc142")
    ELSEIF(MSVC143)
      SET(LOQUAT_MSVC_VERSION "vc143")
    ENDIF()
  ENDIF()
ENDIF ()


######################
# soac

add_executable (soac ${LOQUAT_ROOT_DIRECTORY}/src/pbr/cmd/soac.cpp)
TARGET_USE_COMMON_OUTPUT_DIRECTORY(soac)
add_executable (loquat::soac ALIAS soac)

target_compile_definitions (soac PRIVATE ${COMPILE_DEFINITIONS})
target_compile_options (soac PUBLIC ${CMAKE_CXX_FLAGS})
target_link_libraries (soac PRIVATE loquat_warnings loquat_opt)

set_target_properties (soac PROPERTIES OUTPUT_NAME soac)

add_custom_command (OUTPUT ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h
    COMMAND soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/pbrt.soa > ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h
    DEPENDS soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/pbrt.soa)
set (LOQUAT_SOA_GENERATED ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h)

add_custom_command (OUTPUT ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h
    COMMAND soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/wavefront/work_items.soa > ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h
    DEPENDS soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/wavefront/work_items.soa)
set (LOQUAT_SOA_GENERATED ${LOQUAT_SOA_GENERATED} ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h)

add_custom_target (loquat_soa_generated DEPENDS ${LOQUAT_SOA_GENERATED})

set_property (TARGET soac PROPERTY FOLDER "cmd")
