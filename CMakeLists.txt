cmake_minimum_required(VERSION 3.0)
project(lin)  # Change "your_project_name" to your project's name

# Add all your source files
set(HEADERS
        lin/env.h
        lin/help.h
        lin/init.h
)

set(SOURCES
        lin/_lin.c
        lin/env.c
        lin/help.c
        lin/init.c
        lin/linio.c
        lin/linio.h
)

# Add the executable target
add_executable(lin ${HEADERS} ${SOURCES})  # Change "your_executable_name" to your desired executable name

# Link OpenSSL libraries
find_package(OpenSSL REQUIRED)
target_link_libraries(lin PRIVATE OpenSSL::SSL OpenSSL::Crypto)
