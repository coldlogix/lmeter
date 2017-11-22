cmake_minimum_required(VERSION 2.6)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(CIF cif.y ${CMAKE_CURRENT_BINARY_DIR}/cif.tab.c)
FLEX_TARGET(CIF cif.l  ${CMAKE_CURRENT_BINARY_DIR}/cif.yy.c)

set(SOURCES
    cifgeom.c
    cifgeom.h
    list.c
    list.h
    ${BISON_CIF_OUTPUTS}
    ${FLEX_CIF_OUTPUTS}
)

add_executable(cif2dxc
    cif2dxc.c
    ${SOURCES}
)

target_include_directories(cif2dxc PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} # For lexx/yacc products
)

target_link_libraries(cif2dxc m)

add_executable(cif2ps
    cif2ps.c
    ${SOURCES}
)


target_include_directories(cif2ps PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} # For lexx/yacc products
)

target_link_libraries(cif2ps m)

install(TARGETS cif2ps cif2dxc
        RUNTIME DESTINATION bin)

