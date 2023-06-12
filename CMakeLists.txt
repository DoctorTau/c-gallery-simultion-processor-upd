cmake_minimum_required(VERSION 3.1)

# Set the project name and programming languages
project(my_project C CXX)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the source files for the server program
add_executable(server src/server.c)

# Add the source files for the client program
add_executable(client src/client.c)

#Add the source files for the logger program
add_executable(logger src/logger.c)

# Link with the required libraries
target_link_libraries(server PUBLIC rt pthread)
