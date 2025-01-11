# Loquat Rendering Engine

CMAKE_MINIMUM_REQUIRED(VERSION 3.13...3.31.4)

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
  "Disable this IF you want to use Loquat as a submodule"
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

SET (LOQUAT_OPTIX7_PATH $ENV{LOQUAT_OPTIX7_PATH} CACHE PATH "Path to OptiX 7 SDK")

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
      # Multibyte character SET is deprecated since
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

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
SET(LIBLOQUAT_COMPONENT "libloquat${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}.${LOQUAT_VERSION_PATCH}")
SET(LIBLOQUAT-DEV_COMPONENT "libloquat${LOQUAT_VERSION_MAJOR}.${LOQUAT_VERSION_MINOR}.${LOQUAT_VERSION_PATCH}-dev")
SET(CPACK_COMPONENTS_ALL loquat-bin ${LIBLOQUAT_COMPONENT} ${LIBLOQUAT-DEV_COMPONENT} loquat-dev)

INCLUDE(PrecompiledHeader)
# Include module with fuction 'write_basic_package_version_file'
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(CheckLanguage)

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
  # supress warning for double to float conversion IF Double precission is activated
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
    MESSAGE(FATAL_ERROR "MinGW is too old to be supported. Please update MinGW and try again.")
  ELSEIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.3)
    MESSAGE(WARNING "MinGW is old, IF you experience errors, please update MinGW.")
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

ADD_LIBRARY(loquat_warnings INTERFACE)
TARGET_COMPILE_OPTIONS (
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

ADD_LIBRARY(loquat_opt INTERFACE)

###########################################################################
# Check for various C++ features and SET preprocessor variables or
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

IF (HAVE_MMAP)
  ADD_DEFINITIONS(-DLOQUAT_HAVE_MMAP)
ENDIF ()

include (CheckIncludeFiles)

check_cxx_source_compiles ("
#include <intrin.h>
int main() {
    unsigned long lz = 0, v = 1234;
    IF (_BitScanReverse(&lz, v)) return lz;
    return 0;
} " HAS_INTRIN_H)

IF (HAS_INTRIN_H)
  ADD_DEFINITIONS(-DLOQUAT_HAS_INTRIN_H)
ENDIF ()

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

IF (HAVE_ATTRIBUTE_NOINLINE)
  ADD_DEFINITIONS("-DLOQUAT_NOINLINE=__attribute__((noinline))")
ELSEIF (HAVE_DECLSPEC_NOINLINE)
  ADD_DEFINITIONS("-DLOQUAT_NOINLINE=__declspec(noinline)")
ELSE ()
  ADD_DEFINITIONS(-DLOQUAT_NOINLINE)
ENDIF ()

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

IF (HAVE_PRE_POST_BAR_RESTRICT)
  ADD_DEFINITIONS("-DLOQUAT_RESTRICT=__restrict__")
ELSEIF (HAVE_PRE_BAR_RESTRICT)
  ADD_DEFINITIONS("-DLOQUAT_RESTRICT=__restrict")
ELSE ()
  ADD_DEFINITIONS(-DLOQUAT_RESTRICT)
ENDIF ()

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

IF (HAVE__ALIGNED_MALLOC)
  ADD_DEFINITIONS(-DLOQUAT_HAVE__ALIGNED_MALLOC)
ELSEIF (HAVE_POSIX_MEMALIGN)
  ADD_DEFINITIONS(-DLOQUAT_HAVE_POSIX_MEMALIGN)
ELSE ()
  MESSAGE (SEND_ERROR "Unable to find a way to allocate aligned memory")
ENDIF ()

########################################
# are long and int64_t the same

check_cxx_source_compiles ("
#include <cstdint>
#include <type_traits>
static_assert(!std::is_same<long, int64_t>::value && !std::is_same<long long, int64_t>::value);
int main() { }
" INT64_IS_OWN_TYPE)

IF (INT64_IS_OWN_TYPE)
  ADD_DEFINITIONS(-DLOQUAT_INT64_IS_OWN_TYPE)
ENDIF ()

IF (LOQUAT_NVTX)
  ADD_DEFINITIONS(-DNVTX)
ENDIF()

#########################################
## CUDA / OptiX

ADD_LIBRARY(cuda_build_configuration INTERFACE)

ENABLE_LANGUAGE("CUDA")

IF (CMAKE_CUDA_COMPILER)
  IF (CUDA_VERSION_MAJOR LESS 11)
    MESSAGE (WARNING "loquat requires CUDA version 11.0 or later but version ${CUDA_VERSION_MAJOR}.${CUDA_VERSION_MINOR}\
was found. GPU support is therefore disabled. If you have multiple versions\
of CUDA installed, please update your PATH.")
  ELSE ()

    # This seems to be necessary starting with 3.17.1, but gives an error
    # about 17 being an unsupported version earlier...
    IF (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.0")
        SET (CMAKE_CUDA_STANDARD 17)
    ENDIF ()

    # https://github.com/VIAME/VIAME/blob/aa6c5f56a898b08e4da102c400b453e23952199c/CMakeLists.txt#L291
    IF (NOT CUDA_VERSION_PATCH)
      IF (CUDA_NVCC_EXECUTABLE AND
          CUDA_NVCC_EXECUTABLE STREQUAL CMAKE_CUDA_COMPILER AND
          CMAKE_CUDA_COMPILER_VERSION MATCHES [=[([0-9]+)\.([0-9]+)\.([0-9]+)]=])
        SET (CUDA_VERSION_PATCH "${CMAKE_MATCH_3}")
      ELSEIF (CUDA_NVCC_EXECUTABLE)
        EXECUTE_PROCESS (COMMAND ${CUDA_NVCC_EXECUTABLE} "--version" OUTPUT_VARIABLE NOUT)
        IF (NOUT MATCHES [=[ V([0-9]+)\.([0-9]+)\.([0-9]+)]=])
          SET (CUDA_VERSION_PATCH "${CMAKE_MATCH_3}")
        ENDIF ()
      ENDIF ()
    ENDIF ()

    MESSAGE (STATUS "Found CUDA: ${CUDA_VERSION_MAJOR}.${CUDA_VERSION_MINOR}.${CUDA_VERSION_PATCH}")
    IF (CUDA_VERSION_MAJOR EQUAL 11 AND CUDA_VERSION_MINOR EQUAL 3 AND CUDA_VERSION_PATCH LESS 109)
      MESSAGE (SEND_ERROR "Unfortunately, code from pbrt-v4 triggers an internal compiler error in CUDA 11.3.0. Please either use CUDA 11.0-11.2 or 11.3.1 or later.")
    ENDIF ()

    IF ("${LOQUAT_OPTIX7_PATH}" STREQUAL "")
        MESSAGE (WARNING "Found CUDA but LOQUAT_OPTIX7_PATH is not SET. Disabling GPU compilation.")
    ELSE ()
        enable_language (CUDA)
        list (APPEND LOQUAT_DEFINITIONS "LOQUAT_BUILD_GPU_RENDERER")
        IF (LOQUAT_NVTX)
            list (APPEND LOQUAT_DEFINITIONS "NVTX")
        ENDIF ()
        IF (LOQUAT_NVML)
            list (APPEND LOQUAT_DEFINITIONS "LOQUAT_USE_NVML")
        ENDIF ()
        SET (LOQUAT_CUDA_ENABLED ON)

        # FIXME
        INCLUDE_DIRECTORIES (${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})  # for regular c++ compiles

        # http://www.ssl.berkeley.edu/~jimm/grizzly_docs/SSL/opt/intel/cc/9.0/lib/locale/en_US/mcpcom.msg
        TARGET_COMPILE_OPTIONS (
            loquat_warnings
            INTERFACE
                #"$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xptxas --warn-on-double-precision-use>"
                "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcudafe --diag_suppress=partial_override>"
                "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcudafe --diag_suppress=virtual_FUNCTION_decl_hidden>"
                "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcudafe --diag_suppress=integer_sign_change>"
                "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcudafe --diag_suppress=declared_but_not_referenced>"
                # WAR invalid warnings about this with "IF constexpr"
                "$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcudafe --diag_suppress=implicit_return_from_non_void_FUNCTION>"
        )

        # Willie hears yeh..
        string (APPEND CMAKE_CUDA_FLAGS " -Xnvlink -suppress-stack-size-warning")

        TARGET_COMPILE_OPTIONS (
            cuda_build_configuration
            INTERFACE
                "$<$<COMPILE_LANGUAGE:CUDA>:--std=c++17;--use_fast_math;--expt-relaxed-constexpr;--extended-lambda;--forward-unknown-to-host-compiler>"
                # The "$<NOT:$<BOOL:$<TARGET_PROPERTY:CUDA_PTX_COMPILATION>>>" part is to not add debugging symbols when generating PTX files for OptiX;
                # see https://github.com/mmp/pbrt-v4/issues/69#issuecomment-715499748.
                "$<$<COMPILE_LANGUAGE:CUDA>:$<IF:$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:$<TARGET_PROPERTY:CUDA_PTX_COMPILATION>>>>,-G;-g,-lineinfo;-maxrregcount;128>>"
        )

        IF (LOQUAT_GPU_SHADER_MODEL STREQUAL "")
            # https://wagonhelm.github.io/articles/2018-03/detecting-cuda-capability-with-cmake
            # Get CUDA compute capability
            SET (CHECK_CUDA_OUTPUT_EXE ${CMAKE_BINARY_DIR}/checkcuda)
            IF (MSVC)
                EXECUTE_PROCESS (COMMAND ${CMAKE_CUDA_COMPILER} -lcuda ${LOQUAT_SOURCE_CODE}/cmake/checkcuda.cu -ccbin ${CMAKE_CXX_COMPILER} -o ${CHECK_CUDA_OUTPUT_EXE}
                                 RESULT_VARIABLE BUILD_CHECK_CUDA_RETURN_CODE)
            ELSE  ()
                EXECUTE_PROCESS (COMMAND ${CMAKE_CUDA_COMPILER} -lcuda ${LOQUAT_SOURCE_CODE}/cmake/checkcuda.cu -o ${CHECK_CUDA_OUTPUT_EXE}
                                 RESULT_VARIABLE BUILD_CHECK_CUDA_RETURN_CODE)
            ENDIF ()

            IF (NOT ${BUILD_CHECK_CUDA_RETURN_CODE} EQUAL 0)
                MESSAGE (SEND_ERROR "Was unable to build checkcuda, consider manually setting LOQUAT_GPU_SHADER_MODEL")
            ENDIF ()

            EXECUTE_PROCESS (COMMAND ${CHECK_CUDA_OUTPUT_EXE}
                             RESULT_VARIABLE CUDA_RETURN_CODE
                             OUTPUT_VARIABLE CHECK_CUDA_OUTPUT)

            IF (NOT ${CUDA_RETURN_CODE} EQUAL 0)
                MESSAGE (SEND_ERROR ${CHECK_CUDA_OUTPUT})
              ELSE ()
                SET(ARCH "${CHECK_CUDA_OUTPUT}")
                MESSAGE (STATUS "Detected CUDA Architecture: ${ARCH}")
                string (APPEND CMAKE_CUDA_FLAGS " --gpu-architecture=${ARCH}")
            ENDIF ()
        ELSE ()
            SET(ARCH "${LOQUAT_GPU_SHADER_MODEL}")
            MESSAGE (STATUS "Specified CUDA Architecture: ${ARCH}")
            string (APPEND CMAKE_CUDA_FLAGS " --gpu-architecture=${ARCH}")
        ENDIF ()

        SET (LOQUAT_CUDA_LIB cuda)
        # optix
        # FIXME
        INCLUDE_DIRECTORIES (${LOQUAT_OPTIX7_PATH}/include)

        # find CUDA's bin2c executable
        get_filename_component (cuda_compiler_bin "${CMAKE_CUDA_COMPILER}" DIRECTORY)
        find_program (BIN2C
                      NAMES bin2c
                      PATHS ${cuda_compiler_bin}
                      DOC "Path to the CUDA SDK bin2c executable."
                      NO_DEFAULT_PATH)
        IF (NOT BIN2C)
            MESSAGE (FATAL_ERROR
                     "bin2c not found:\n"
                     "  CMAKE_CUDA_COMPILER='${CMAKE_CUDA_COMPILER}'\n"
                     "  cuda_compiler_bin='${cuda_compiler_bin}'\n"
            )
        ENDIF ()

        # this macro defines cmake rules that execute the following four steps:
        # 1) compile the given cuda file ${cuda_file} to an intermediary PTX file
        # 2) use the 'bin2c' tool (that comes with CUDA) to
        #    create a second intermediary (.c-)file which defines a const string variable
        #    (named '${c_var_name}') whose (constant) value is the PTX output
        #    from the previous step.
        # 3) compile the given .c file to an intermediary object file (why thus has
        #    that PTX string 'embedded' as a global constant.
        # 4) assign the name of the intermediary .o file to the cmake variable
        #    'output_var', which can then be added to cmake targets.
        MACRO (cuda_compile_and_embed output_var cuda_file lib_name)
          ADD_LIBRARY("${lib_name}" OBJECT "${cuda_file}")
          SET_PROPERTY(TARGET "${lib_name}" PROPERTY CUDA_PTX_COMPILATION ON)

          # disable "extern declaration... is treated as a static definition" warning
          IF (CUDA_VERSION_MAJOR EQUAL 11 AND CUDA_VERSION_MINOR LESS 2)
              TARGET_COMPILE_OPTIONS ("${lib_name}" PRIVATE
                                      -Xcudafe=--display_error_number -Xcudafe=--diag_suppress=3089)
          ELSE ()
              TARGET_COMPILE_OPTIONS ("${lib_name}" PRIVATE
                                      -Xcudafe=--display_error_number -Xcudafe=--diag_suppress=20044)
          ENDIF ()

          # CUDA integration in Visual Studio seems broken as even IF "Use
          # Host Preprocessor Definitions" is checked, the host preprocessor
          # definitions are still not used when compiling device code.
          # To work around that, define the MACROs using --define-MACRO to
          # avoid CMake identifying those as MACROs and using the proper (but
          # broken) way of specifying them.
          IF (${CMAKE_GENERATOR} MATCHES "^Visual Studio")
            # As LOQUAT_DEBUG_BUILD is specified globally as a definition, we need to
            # manually add it due to the bug mentioned earlier and due to it
            # not being found in LOQUAT_DEFINITIONS.
            IF (CMAKE_BUILD_TYPE MATCHES Debug)
                SET (cuda_definitions "--define-MACRO=LOQUAT_DEBUG_BUILD")
            ENDIF ()
            FOREACH (arg ${LOQUAT_DEFINITIONS})
              list (APPEND cuda_definitions "--define-MACRO=${arg}")
            ENDFOREACH ()
            TARGET_COMPILE_OPTIONS ("${lib_name}" PRIVATE ${cuda_definitions})
          ELSE ()
            TARGET_COMPILE_DEFINITIONS ("${lib_name}" PRIVATE ${LOQUAT_DEFINITIONS})
          ENDIF ()
          TARGET_INCLUDE_DIRECTORIES ("${lib_name}" PRIVATE $<BUILD_INTERFACE:src> $<BUILD_INTERFACE:${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}>)
          TARGET_INCLUDE_DIRECTORIES ("${lib_name}" SYSTEM PRIVATE $<BUILD_INTERFACE:${NANOVDB_INCLUDE}>)
          TARGET_LINK_LIBRARIES ("${lib_name}" PRIVATE cuda_build_configuration loquat_opt loquat_warnings)
          ADD_DEPENDENCIES ("${lib_name}" loquat_soa_generated)
          SET (c_var_name ${output_var})
          SET (embedded_file ${cuda_file}.ptx_embedded.c)
          ADD_CUSTOM_COMMAND (
            OUTPUT "${embedded_file}"
            COMMAND ${CMAKE_COMMAND}
              "-DBIN_TO_C_COMMAND=${BIN2C}"
              "-DOBJECTS=$<TARGET_OBJECTS:${lib_name}>"
              "-DVAR_NAME=${c_var_name}"
              "-DOUTPUT=${embedded_file}"
              -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bin2c_wrapper.cmake
            VERBATIM
            DEPENDS "${lib_name}" $<TARGET_OBJECTS:${lib_name}>
            COMMENT "Embedding PTX generated from ${cuda_file}"
          )
          SET (${output_var} ${embedded_file})
        ENDMACRO ()
    ENDIF ()
  ENDIF()
ELSE ()
    MESSAGE (STATUS "CUDA not found")
ENDIF ()

# Set Loquat project output directory variables.
# Will respect top-level CMAKE_*_OUTPUT_DIRECTORY variables IF any are SET.
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

# Macro used to SET the output directories of a target to the
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

# Only generate this target IF no higher-level project already has
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

FUNCTION (CHECK_EXT NAME DIR HASH)
  IF (NOT IS_DIRECTORY "${LOQUAT_THIRD_PARTY_DIR}/${DIR}")
    MESSAGE (FATAL_ERROR "The ${NAME} submodule directory is missing! "
      "Either that submodule was recently added or you did not clone the project with --recursive. "
      "In order to update the submodules, run:\n"
      "  \"git submodule update --init --recursive\"")
  ENDIF ()

  FIND_PACKAGE(Git)
  IF (GIT_FOUND)
    EXECUTE_PROCESS (
      COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      WORKING_DIRECTORY "${LOQUAT_THIRD_PARTY_DIR}/${DIR}"
      RESULT_VARIABLE "git_return"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE "git_hash")
    IF (NOT ${git_hash} MATCHES "^${HASH}")
      MESSAGE (FATAL_ERROR "The ${LOQUAT_THIRD_PARTY_DIR}/${DIR} "
        "submodule isn't up to date (${git_hash} vs ${HASH}). Please run:\n"
        "  \"git submodule update --recursive\"")
    ELSE ()
      MESSAGE (STATUS "${NAME} at commit: ${git_hash}")
    ENDIF()
  ELSE (GIT_FOUND)
    MESSAGE (STATUS "git not found: unable to verify revisions in submodules")
  ENDIF (GIT_FOUND)
ENDFUNCTION ()

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
#SET(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)

if (MSVC)
  ADD_DEFINITIONS(/D _CRT_SECURE_NO_WARNINGS /Dstrdup=_strdup
    /wd4018 /wd4100 /wd4101 /wd4127 /wd4146 /wd4232 /wd4242 /wd4244 /wd4245 /wd4267 /wd4305 /wd4309 
    /wd4310 /wd4334 /wd4456 /wd4464 /wd4668 /wd4701 /wd4703 /wd4711 /wd4756 /wd4820 /wd5045 /wd5250)
endif ()

###########################################################################
# stb

SET(STB_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/stb)

###########################################################################
# filesystem

SET(FILESYSTEM_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/filesystem)

###########################################################################
# libdeflate

ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/libdeflate)

SET(LIBDEFLATE_INCLUDE_DIRS ${LOQUAT_THIRD_PARTY_DIR}/libdeflate)
SET(LIBDEFLATE_LIBRARIES deflate::deflate)

SET_PROPERTY(TARGET deflate PROPERTY FOLDER "third-party")

###########################################################################
# zlib

FIND_PACKAGE(ZLIB)
if (NOT ZLIB_FOUND)
  # Build zlib
  SET(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
  SET(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/zlib)

  # try to make openexr happy about this...
  SET(ZLIB_LIBRARY zlibstatic)
  SET(ZLIB_FOUND TRUE)

  SET_PROPERTY(TARGET zlibstatic PROPERTY FOLDER "third-party")

  ADD_LIBRARY(ZLIB::ZLIB ALIAS zlibstatic)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})  # yuck, but so openexr/ptex can find zlib.h...
endif ()

SET(ZLIB_INCLUDE_DIR ${LOQUAT_THIRD_PARTY_DIR}/zlib ${LOQUAT_LIB_INSTALL_DIR}/third-party/zlib)
SET(ZLIB_INCLUDE_DIRS ${LOQUAT_THIRD_PARTY_DIR}/zlib)
SET(ZLIB_LIBRARIES zlibstatic)

###########################################################################
# OpenEXR

#TODO(ches) fix the build appending src path to hte folder path during TARGET_INCLUDE_DIRECTORIES

#FIND_PACKAGE(OpenEXR)
#
#if (OPENEXR_FOUND)
#  message (STATUS "Found OpenEXR version ${OPENEXR_VERSION}")
#  SET(OPENEXR_INCLUDE ${OPENEXR_INCLUDES}/OpenEXR ${ILMBASE_INCLUDES}/OpenEXR)
#  if (WIN32)
#    SET(OPENEXR_LIBS ${OPENEXR_LIBRARIES} ${ZLIB_LIBRARY})
#  else ()
#    SET(OPENEXR_LIBS ${OPENEXR_LIBRARIES})
#  endif ()
#else ()
#  message (STATUS "OpenEXR not found; building it from scratch.")
#
#  SET (CMAKE_POLICY_DEFAULT_CMP0077 NEW)
#
#  SET(ILMBASE_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
#  SET(OPENEXR_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
#  SET(OPENEXR_BUILD_SHARED_LIBS    OFF CACHE BOOL " " FORCE)
#  SET(ILMBASE_BUILD_SHARED_LIBS    OFF CACHE BOOL " " FORCE)
#  SET(PYILMBASE_ENABLE             OFF CACHE BOOL " " FORCE)
#  SET(OPENEXR_BUILD_UTILS          OFF CACHE BOOL " " FORCE)
#
#  ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/openexr)
#
#  SET_PROPERTY(TARGET IexMath IlmThread Half
#                Iex Imath IlmImf HalfTest IexTest
#                IlmImfExamples IlmImfTest IlmImfUtil IlmImfUtilTest ImathTest
#                PROPERTY FOLDER "third-party/OpenEXR")
#
#  SET(OPENEXR_INCLUDE
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/IlmBase/Imath
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/IlmBase/Half
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/IlmBase/Iex
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/OpenEXR/IlmImf
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/IlmBase/config
#    ${LOQUAT_THIRD_PARTY_DIR}/openexr/OpenEXR/config
#  )
#
#  if (WIN32)
#    SET(OPENEXR_LIBS OpenEXR::IlmImf IlmBase::Imath IlmBase::Half ${ZLIB_LIBRARY})
#  else ()
#    SET(OPENEXR_LIBS OpenEXR::IlmImf IlmBase::Imath IlmBase::Half)
#  endif ()
#endif ()

###########################################################################
# ptex

SET(PTEX_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)

SET(CMAKE_MACOSX_RPATH 1)
if (WIN32)
  ADD_DEFINITIONS(/DPTEX_STATIC)
endif ()

ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/ptex)

SET_PROPERTY(TARGET Ptex_static ptxinfo halftest ftest rtest wtest PROPERTY FOLDER "third-party/ptex")

SET(PTEX_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/ptex/src/ptex)

###########################################################################
# double-conversion

ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/double-conversion)

SET(DOUBLE_CONVERSION_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/double-conversion)

SET_PROPERTY(TARGET double-conversion cctest PROPERTY FOLDER "third-party")

###########################################################################
# nanovdb

SET(NANOVDB_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/openvdb/nanovdb)

###########################################################################
# FLIP

SET(FLIP_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/flip)

ADD_LIBRARY(flip_lib STATIC ${LOQUAT_THIRD_PARTY_DIR}/flip/flip.cpp)

SET_PROPERTY(TARGET flip_lib PROPERTY FOLDER "third-party")

###########################################################################
# utf8proc

SET(UTF8PROC_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/utf8proc)

ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/utf8proc)

###########################################################################
# qoi

SET(QOI_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/qoi)

###########################################################################
# glfw / glad

SET(GLFW_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/glfw/include)
SET(GLAD_INCLUDE ${LOQUAT_THIRD_PARTY_DIR}/glad/include)

SET(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/glfw)
ADD_SUBDIRECTORY(${LOQUAT_THIRD_PARTY_DIR}/glad)

SET_PROPERTY(TARGET glfw PROPERTY FOLDER "third-party")
SET_PROPERTY(TARGET glad PROPERTY FOLDER "third-party")

SET(ALL_LOQUAT_LIBS
  #loquat_lib
  ${CMAKE_THREAD_LIBS_INIT}
  ${OPENEXR_LIBS}
  Ptex_static
  ${ZLIB_LIBRARIES}
  ${LIBDEFLATE_LIBRARIES}
  double-conversion
  ${LOQUAT_CUDA_LIB}
  utf8proc
  glfw
  glad
  #OpenGL::GL
  ZLIB::ZLIB
  )

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

IF(WIN32)
  IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(BIN_DIR "${PROJECT_SOURCE_DIR}/bin64/")
    SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib64/")
  ELSE()
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

TARGET_COMPILE_DEFINITIONS (soac PRIVATE ${COMPILE_DEFINITIONS})
TARGET_COMPILE_OPTIONS (soac PUBLIC ${CMAKE_CXX_FLAGS})
TARGET_LINK_LIBRARIES (soac PRIVATE loquat_warnings loquat_opt)

set_target_properties (soac PROPERTIES OUTPUT_NAME soac)

ADD_CUSTOM_COMMAND (OUTPUT ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h
    COMMAND soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/pbrt.soa > ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h
    DEPENDS soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/pbrt.soa)
SET (LOQUAT_SOA_GENERATED ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/pbrt_soa.h)

ADD_CUSTOM_COMMAND (OUTPUT ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h
    COMMAND soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/wavefront/work_items.soa > ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h
    DEPENDS soac ${LOQUAT_ROOT_DIRECTORY}/include/pbr/wavefront/work_items.soa)
SET (LOQUAT_SOA_GENERATED ${LOQUAT_SOA_GENERATED} ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY}/wavefront_work_items_soa.h)

add_custom_target (loquat_soa_generated DEPENDS ${LOQUAT_SOA_GENERATED})

SET_PROPERTY(TARGET soac PROPERTY FOLDER "cmd")
