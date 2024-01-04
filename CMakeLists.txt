# Loquat Rendering Engine

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Loquat VERSION 1.0.0)

# Options #####################################################################

OPTION(BUILD_SHARED_LIBS
  "Build package with shared libraries"
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

OPTION(LOQUAT_BUILD_DOCS
  "Build documentation using Doxygen"
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

# Use solution folders.
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(WIN32)
  # Use subSETof Windows.h
  ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)

  IF(MSVC)
    OPTION(LOQUAT_INSTALL_PDB
      "Install MSVC debug files"
      ON
	)
    IF(NOT (MSVC_VERSION LESS 1900))
      # Multibyte character SETis deprecated since
	  # at least MSVC2015 (possibly earlier)
      ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
    ENDIF()
	
	OPTION(USE_STATIC_CRT
	  "Link against static runtime libraries"
	  OFF
	)
	
	IF(${USE_STATIC_CRT})
	  ADD_COMPILE_OPTIONS(
	      $<$<CONFIG:>:/MT>
          $<$<CONFIG:Debug>:/MTd>
          $<$<CONFIG:Release>:/MT>
	  )
	ENDIF()
  ENDIF()
ENDIF()

IF(WIN32)
	ADD_DEFINITIONS(-D_LOQUAT_WIN32)
ENDIF()

IF(UNIX)
	IF (LOQUAT_USE_WAYLAND)
		ADD_DEFINITIONS(-D_LOQUAT_WAYLAND)
	ELSE()
		ADD_DEFINITIONS(-D_LOQUAT_X11)
	ENDIF()
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

SET(LOQUAT_PACKAGE_VERSION "0" CACHE STRING "The package-specific version used for uploading the sources")
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_C_STANDARD 99)

INCLUDE_DIRECTORIES(BEFORE
  ./
  src/
  include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/include
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
  # enable multi-core compilation with MSVC
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # clang-cl
    ADD_COMPILE_OPTIONS(/bigobj)
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

GET_CMAKE_PROPERTY(is_multi_config GENERATOR_IS_MULTI_CONFIG)

# Only generate this target if no higher-level project already has
IF (NOT TARGET uninstall AND LOQUAT_INSTALL)
  # add make uninstall capability
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
  ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
ENDIF()


SET(LOQUAT_LIB_INSTALL_DIR ${LOQUAT_LIBRARY_OUTPUT_DIRECTORY})
SET(LOQUAT_INCLUDE_INSTALL_DIR "include")
SET(LOQUAT_BIN_INSTALL_DIR ${LOQUAT_RUNTIME_OUTPUT_DIRECTORY})

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

IF(LOQUAT_BUILD_DOCS)
  ADD_SUBDIRECTORY(doc)
ENDIF()

SET (LOQUAT_BUILD_ARCHITECTURE "" CACHE STRING "describes the current architecture")
IF(NOT LOQUAT_BUILD_ARCHITECTURE STREQUAL "")
  ADD_DEFINITIONS(-D'LOQUAT_BUILD_ARCHITECTURE="${LOQUAT_BUILD_ARCHITECTURE}"')
ENDIF()

SET (LOQUAT_BUILD_COMPILER "" CACHE STRING "describes the current compiler")
IF(NOT LOQUAT_BUILD_COMPILER STREQUAL "")
  ADD_DEFINITIONS(-D'LOQUAT_BUILD_COMPILER="${LOQUAT_BUILD_COMPILER}"')
ENDIF()

MARK_AS_ADVANCED (LOQUAT_BUILD_ARCHITECTURE LOQUAT_BUILD_COMPILER)

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
