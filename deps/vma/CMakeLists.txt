add_library(vma STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h
)

target_link_libraries(vma
    PRIVATE volk
)

target_include_directories(vma
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)