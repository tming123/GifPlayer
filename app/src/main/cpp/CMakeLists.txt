cmake_minimum_required(VERSION 3.4.1)
add_library(
        native-lib
        SHARED
        native-lib.cpp
        dgif_lib.c
        gifalloc.c
)

find_library(
        jnigraphics-lib
        jnigraphics
)

find_library(
        log-lib
        log
)

target_link_libraries(
        native-lib
        ${log-lib}
        ${jnigraphics-lib}
)