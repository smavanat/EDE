cmake_minimum_required(VERSION 3.10)
project(Engine LANGUAGES C)

# Set C standard
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
# set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fsanitize=address)

include(FetchContent)

# #Fetch box2D:
FetchContent_Declare (
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.0
)
set(BOX2D_BUILD_WARNINGS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(box2d)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
)


FetchContent_GetProperties(glfw)

if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

endif()

# Collect all .cpp files in current directory
file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
set(SOURCES ${C_SOURCES})

# Create the executable target
add_executable(ede ${SOURCES})

# Add include directories (external libraries)
target_include_directories(ede
    PRIVATE
        "${CMAKE_SOURCE_DIR}/externals/Include"
)

target_link_libraries(ede
                        PRIVATE
                        glfw
                        box2d
                        GL X11 pthread Xi dl m)

# Have to copy all of the assets to the output directory
add_custom_command(TARGET ede POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data/assets"
        $<TARGET_FILE_DIR:ede>/assets
)
