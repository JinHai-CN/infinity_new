
add_executable(fast_io
        fast_io.cpp
)

target_include_directories(fast_io
        PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/third_party/fast_io/include")
