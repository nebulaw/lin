cmake_minimum_required(VERSION 3.10)
project(lin C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(HEADERS
    lin/help.h
    lin/init.h
    lin/group.h
    lin/checkpoint.h
    lin/add.h
    lin/remove.h
    lin/status.h
    lin/log.h
    lin/stats.h
    lin/diff.h
    lin/linio.h
    lin/error.h
    lin/format.h
    lin/storage.h
    lin/context.h
    lin/manifest.h
    lin/analysis.h
    lin/snap.h
    lin/groupv2.h
    lin/ignore.h
    lin/fsck.h
)

set(SOURCES
    lin/_lin.c
    lin/help.c
    lin/init.c
    lin/linio.c
    lin/group.c
    lin/checkpoint.c
    lin/add.c
    lin/remove.c
    lin/status.c
    lin/log.c
    lin/stats.c
    lin/diff.c
    lin/error.c
    lin/format.c
    lin/storage.c
    lin/context.c
    lin/manifest.c
    lin/analysis.c
    lin/snap.c
    lin/groupv2.c
    lin/ignore.c
    lin/fsck.c
)

add_executable(lin ${HEADERS} ${SOURCES})

target_compile_options(lin PRIVATE -Wall -Wextra -Wpedantic)

find_package(OpenSSL REQUIRED)
target_link_libraries(lin PRIVATE OpenSSL::SSL OpenSSL::Crypto)
