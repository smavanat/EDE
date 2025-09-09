cmake_minimum_required(VERSION 3.10)
project(Engine LANGUAGES C)

# Set C standard
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include(FetchContent)

# #Fetch box2D:
# FetchContent_Declare (
#     box2d
#     GIT_REPOSITORY https://github.com/erincatto/box2d.git
#     GIT_TAG v3.1.0
# )
# set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
# set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
# set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)

# FetchContent_MakeAvailable(box2d)

# Collect all .cpp files in current directory
file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
# file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(SOURCES ${C_SOURCES})

# Create the executable target
add_executable(engine ${SOURCES})

# Add include directories (external libraries)
target_include_directories(engine
    PRIVATE
        "${CMAKE_SOURCE_DIR}/externals/Include"
)

# target_link_libraries(engine PRIVATE box2d::box2d)
target_link_libraries(engine
                        PRIVATE
                        "${CMAKE_SOURCE_DIR}/externals/libs_linux/libglfw3.a"
                        GL X11 pthread Xi dl m)

# Have to copy all of the assets to the output directory
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data/assets"
        $<TARGET_FILE_DIR:engine>/assets
)
