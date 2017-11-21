cmake_minimum_required(VERSION 2.6)

set (SOURCES 
    css_par.h
    csspdms.h
    directso.c
    factoriz.c
    iterativ.c
    lowlevel.c
    misc.h
    numbfact.c
    permute.c
    spmatrix.c
    spvector.c
    symbfact.c
)


add_library(csspdms
    ${SOURCES}
)

target_include_directories(csspdms 
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


