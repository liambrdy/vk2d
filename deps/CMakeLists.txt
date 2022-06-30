file(GLOB IMGUI_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h)

add_library(imgui STATIC
    ${IMGUI_SRCS}
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

add_library(spirv-reflect STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect/spirv_reflect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect/spirv_reflect.h
)

target_include_directories(spirv-reflect PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect
)
