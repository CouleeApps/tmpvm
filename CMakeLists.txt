project(tmpvm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O1 -mllvm -inline-threshold=65537 -ftemplate-depth=65537")

add_executable(tmpvm tmpvm.cpp)
