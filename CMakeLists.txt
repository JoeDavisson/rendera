cmake_minimum_required(VERSION 3.16)

if(APPLE OR WIN32)
  set(APP_TARGET "Rendera")
else()
  set(APP_TARGET "rendera")
endif()

set(APP_VERSION "0.2.9")

set(CMAKE_CXX_STANDARD 20)

if (${CMAKE_VERSION} VERSION_LESS "3.21")
  set(CMAKE_C_STANDARD 11)
else()
  set(CMAKE_C_STANDARD 17)
endif()

project(${APP_TARGET}
  VERSION ${APP_VERSION}
  LANGUAGES CXX
)

add_definitions(-DPACKAGE_STRING="${APP_VERSION}")

option(SANITIZE "Build with asan/ubsan" OFF)

if(SANITIZE)
  message("Sanitizers enabled")
else()
  message("Sanitizers disabled (pass -DSANITIZE=1 to enable)")
endif()

#-------------------------------------------------------------------------------
# DEPENDENCIES
#-------------------------------------------------------------------------------

find_package(FLTK REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

#-------------------------------------------------------------------------------
# APP SOURCES
#-------------------------------------------------------------------------------

set(APP_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS ${APP_SRCDIR}/*.cxx)

set(APP_ALL_SOURCES
  ${APP_SOURCES}
)

# Make source groups match file tree
source_group(TREE ${APP_SRCDIR} PREFIX "" FILES ${APP_ALL_SOURCES})

#-------------------------------------------------------------------------------
# EXECUTABLE TARGET
#-------------------------------------------------------------------------------

set(APP_LIBRARIES
  ${FLTK_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${PNG_LIBRARIES}
)

if(WIN32)
  #TODO: Write Windows case.
elseif(APPLE)
  #TODO: Write macOS case.
else()
  add_executable(${APP_TARGET} ${APP_ALL_SOURCES})
endif()

target_include_directories(${APP_TARGET} PRIVATE
  ${APP_SRCDIR}
  ${FLTK_INCLUDE_DIRS}
  ${JPEG_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
)

#-------------------------------------------------------------------------------
# COMPILER/LINKER OPTIONS
#-------------------------------------------------------------------------------

if(NOT MSVC)
  target_compile_options(${APP_TARGET} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    )

  if(SANITIZE)
    #TODO: Add sanitizers.
  endif()
else()
  #TODO: Write MSVC case.
endif()

set_target_properties(${APP_TARGET} PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(${APP_TARGET} ${APP_LIBRARIES})

#-------------------------------------------------------------------------------
# PRE-BUILD HEADER GENERATION
#-------------------------------------------------------------------------------

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/Images.H"
    COMMAND gcc -O3 "${CMAKE_CURRENT_SOURCE_DIR}/makeheader.c" -o "${CMAKE_CURRENT_SOURCE_DIR}/makeheader"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/makeheader" "src/Images.H" "images/*.png"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/makeheader.c" "${CMAKE_CURRENT_SOURCE_DIR}/images/*.png"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating Images.H"
)

# Add the custom command as a dependency for the build
add_custom_target(images_header DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Images.H")

# Linking target with the custom target
add_dependencies(${APP_TARGET} images_header)


#-------------------------------------------------------------------------------
# PLATFORM-SPECIFIC LIBRARIES
#-------------------------------------------------------------------------------

if (APPLE)
  #TODO: Write macOS case.
else()
  set(APP_PARENT_DIR "$<TARGET_FILE_DIR:${APP_TARGET}>")
endif()

# Windows-specific libraries
if(WIN32)
  #TODO: Write Windows case.
endif()

