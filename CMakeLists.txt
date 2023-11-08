cmake_minimum_required(VERSION 3.10)
project(HSM CXX)

set(CMAKE_CXX_STANDARD 11)

add_library(hsm STATIC
	HSM.cpp
	HSM.hpp
)

target_compile_options(hsm PRIVATE -Wall -Wextra)

add_executable(demo
	demo.cpp
)

add_executable(test
	test.cpp
)

target_link_libraries(demo hsm)
target_link_libraries(test hsm)

install(TARGETS
	hsm
)

install(FILES
	HSM.hpp
	
	TYPE INCLUDE
)
