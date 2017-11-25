cmake_minimum_required(VERSION 2.6)

project(lmeter)

add_subdirectory(contrib)
add_subdirectory(vendor/NGST_lm/lm/src/cif2dxc)
#add_subdirectory(vendor/NGST_lm/lm/src/contrib)
#add_subdirectory(vendor/NGST_lm/lm/src/gtkLM)
add_subdirectory(vendor/NGST_lm/lm/src/lm2sch)
add_subdirectory(vendor/NGST_lm/lm/src/lmeter)
#add_subdirectory(vendor/NGST_lm/lm/src/scripts)

install(FILES 
            vendor/NGST_lm/lm/src/scripts/cif2sch
            vendor/NGST_lm/lm/src/scripts/spiceflatten
            vendor/NGST_lm/lm/src/scripts/trylm2sch
            vendor/NGST_lm/lm/src/scripts/updatevalues
        DESTINATION bin)

install(FILES 
            vendor/NGST_lm/lm/man/dxc.5
            vendor/NGST_lm/lm/man/lm2sch.1
            vendor/NGST_lm/lm/man/lmeter.1
            vendor/NGST_lm/lm/man/lmeter.5
            vendor/NGST_lm/lm/man/man.ps
#        DIRECTORY
#            vendor/NGST_lm/lm/man/readme
        DESTINATION man)

#
# CPack
#

SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

set(CPACK_PACKAGE_NAME "lmeter")
set(CPACK_PACKAGE_VENDOR "ColdLogix")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "lmeter - A 2d inductance extraction tool by Paul Bunyk")
set(CPACK_PACKAGE_VERSION "2.13.0")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "13")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "lmeter")


#
# Debian
#

set (CPACK_DEBIAN_PACKAGE_MAINTAINER	"Jonathan Stark")
set (CPACK_PACKAGE_DESCRIPTION
"lmeter computes a current flowing through terminals of a multilayer superconductive integrated circuit in several experiments when
unit Josephson phase difference across one of the terminals - current sources and zero phase differencies  across  other  terminals  are
applied.  Used  approach is based on the solution of 2D laplace equation on Josephson phase differencies between superconductive layers 
with coefficients adjusted to take into account 3D effects and changes in feature sizes during fabrication.")

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY 
"lmeter is a 2d superdconductor inductance extraction tool written by Paul Bunyk"
)
set (CPACK_DEBIAN_PACKAGE_DEPENDS	"")
set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "")
set (CPACK_DEBIAN_PACKAGE_SECTION	"Electronics")
#set (CPACK_DEBIAN_PACKAGE_VERSION	)


include(CPack)

set(CPACK_COMPONENTS_ALL applications)

