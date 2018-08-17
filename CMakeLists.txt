cmake_minimum_required(VERSION 3.12)

project(picoray)

#set(CMAKE_CXX_STANDARD 11)
set(SDL2_PATH "..\\..\\dev\\SDL2-2.0.8")
set(SDL2_IMAGE_PATH "..\\..\\dev\\SDL2_image-2.0.3")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

if(NOT WIN32)
    find_package(OpenGL REQUIRED)
endif()

include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include)

add_executable (picoray source/main.cpp)
target_link_libraries(picoray ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES})

if(WIN32)
    foreach(DLL ${SDL2_DLLS})
        add_custom_command(TARGET picoray POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:picoray>)
    endforeach()
endif()