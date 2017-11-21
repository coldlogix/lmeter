cmake_minimum_required(VERSION 2.6)

project(lmeter)

add_subdirectory(contrib)
add_subdirectory(vendor/lm212/src/cif2dxc)
#add_subdirectory(vendor/lm212/src/contrib)
#add_subdirectory(vendor/lm212/src/gtkLM)
add_subdirectory(vendor/lm212/src/lm2sch)
add_subdirectory(vendor/lm212/src/lmeter)
#add_subdirectory(vendor/lm212/src/scripts)

install(FILES 
            vendor/lm212/src/scripts/cif2sch
            vendor/lm212/src/scripts/spiceflatten
            vendor/lm212/src/scripts/trylm2sch
            vendor/lm212/src/scripts/updatevalues
        DESTINATION bin)


#
# CPack
#

SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

set(CPACK_PACKAGE_NAME "lmeter")
set(CPACK_PACKAGE_VENDOR "ColdLogix")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "lmeter - A 2d inductance extraction tool by Paul Bunyk")
set(CPACK_PACKAGE_VERSION "2.12.0")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "12")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "lmeter")

SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jonathan Stark") 

include(CPack)

set(CPACK_COMPONENTS_ALL applications)

