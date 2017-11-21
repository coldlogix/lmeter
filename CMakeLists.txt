cmake_minimum_required(VERSION 2.6)

project(lmeter)

add_subdirectory(contrib)
add_subdirectory(vendor/lm212/src/cif2dxc)
#add_subdirectory(vendor/lm212/src/contrib)
#add_subdirectory(vendor/lm212/src/gtkLM)
add_subdirectory(vendor/lm212/src/lm2sch)
add_subdirectory(vendor/lm212/src/lmeter)
#add_subdirectory(vendor/lm212/src/scripts)

