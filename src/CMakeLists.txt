# Loquat Rendering Engine

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(HEADER_PATH ../include)

SET(PUBLIC_HEADERS
  # ${HEADER_PATH}/command
  ${HEADER_PATH}/debug/logger.h
  ${HEADER_PATH}/device/device.h
  ${HEADER_PATH}/main/global_state.h
  ${HEADER_PATH}/main/vulkan_instance.h
  ${HEADER_PATH}/memory/memory_utils.h
  # ${HEADER_PATH}/pipeline
  # ${HEADER_PATH}/shader
  ${HEADER_PATH}/window/window.h
  ${HEADER_PATH}/window/window_state.h
  ${HEADER_PATH}/window/window_surface.h
)

SET(CORE_SRCS
  debug/logger.cpp
  device/device.cpp
  main/global_state.cpp
  main/loquat.cpp
  main/vulkan_instance.cpp
  window/window.cpp
  window/window_state.cpp
  window/window_surface.cpp
)

SOURCE_GROUP(include FILES ${PUBLIC_HEADERS})

SET(loquat_src
  ${CORE_SRCS}
  
  # Necessary to show the headers in the project when using the VC++ generator:
  ${PUBLIC_HEADERS}
)

ADD_EXECUTABLE(loquat ${loquat_src})

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

FIND_PACKAGE(Vulkan REQUIRED)

TARGET_LINK_LIBRARIES(loquat glfw Vulkan::Vulkan)

TARGET_INCLUDE_DIRECTORIES (loquat PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
  $<BUILD_INTERFACE:${GLFW_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${LOQUAT_INCLUDE_INSTALL_DIR}>
)

# Copy DLL files on platforms where it's needed 
IF (CMAKE_IMPORT_LIBRARY_SUFFIX) 
	add_custom_command(TARGET loquat POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:loquat> $<TARGET_FILE_DIR:loquat> COMMAND_EXPAND_LISTS)
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
      COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMPILE_PDB_NAME loquat${LIBRARY_SUFFIX}
      COMPILE_PDB_NAME_DEBUG loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}
   )

    IF(GENERATOR_IS_MULTI_CONFIG)
      install(FILES ${LOQUAT_BINARY_DIR}/code/Debug/loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS Debug
     )
      install(FILES ${LOQUAT_BINARY_DIR}/code/RelWithDebInfo/loquat${LIBRARY_SUFFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS RelWithDebInfo
     )
    ELSE()
      install(FILES ${LOQUAT_BINARY_DIR}/code/loquat${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS Debug
     )
      install(FILES ${LOQUAT_BINARY_DIR}/code/loquat${LIBRARY_SUFFIX}.pdb
        DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
        CONFIGURATIONS RelWithDebInfo
     )
    ENDIF()
  ELSE()
    install(FILES $<TARGET_PDB_FILE:loquat>
      DESTINATION ${LOQUAT_LIB_INSTALL_DIR}
   )
  ENDIF()
ENDIF ()
