cmake_minimum_required(VERSION 3.16)

project(LRP
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(lrp STATIC
    src/lrp_frame.c
    src/lrp_dedup.c
    src/lrp_router.c
)

target_include_directories(lrp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(lrp PRIVATE /W4 /WX)
else()
    target_compile_options(lrp PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

enable_testing()

add_executable(test_frame
    tests/test_frame.c
)

target_link_libraries(test_frame
    PRIVATE
        lrp
)

add_test(NAME frame_layer_strict_validation COMMAND test_frame)

add_executable(test_dedup
    tests/test_dedup.c
)

target_link_libraries(test_dedup
    PRIVATE
        lrp
)

add_test(NAME dedup_layer_strict_validation COMMAND test_dedup)

add_executable(test_router
    tests/test_router.c
)

target_link_libraries(test_router
    PRIVATE
        lrp
)

add_test(NAME router_layer_strict_validation COMMAND test_router)

add_executable(simulated_three_nodes
    examples/simulated_three_nodes/main.c
)

target_link_libraries(simulated_three_nodes
    PRIVATE
        lrp
)

add_executable(sample_mvp_basic_usage
    samples/mvp_basic_usage/main.c
)

target_link_libraries(sample_mvp_basic_usage
    PRIVATE
        lrp
)
