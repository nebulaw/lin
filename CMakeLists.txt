cmake_minimum_required(VERSION 3.0)
project(lin)  # Change "your_project_name" to your project's name

# Add all your source files
set(HEADERS
    lin/env.h
    lin/help.h
    lin/init.h
    lin/group.h
    lin/enc.h
        lin/checkpoint.h
        lin/add.h
        lin/objects.h
        lin/objects.c
)

set(SOURCES
    lin/_lin.c
    lin/env.c
    lin/help.c
    lin/init.c
    lin/linio.c
    lin/linio.h
    lin/group.c
    lin/enc.c
        lin/checkpoint.c
        lin/add.c
)

# Add the executable target
add_executable(lin ${HEADERS} ${SOURCES})

# Link OpenSSL libraries
find_package(OpenSSL REQUIRED)
target_link_libraries(lin PRIVATE OpenSSL::SSL OpenSSL::Crypto)
