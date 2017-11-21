cmake_minimum_required(VERSION 2.6)

set (SOURCES
    linalg.c
    linalg.h
    lm2sch.c
)

add_executable(lm2sch
    ${SOURCES}
)

install(TARGETS lm2sch
        RUNTIME DESTINATION bin)
