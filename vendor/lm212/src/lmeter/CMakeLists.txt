cmake_minimum_required(VERSION 2.6)

#	The following -Defines may be added here:
#       -DOLDLM3 to get .dat files for the old solver (not supported now).
#       -DNOINDCORR to suppress boundary inductance correction.
#       -DNDEBUG when completely tested :-) ...
#	-DPSGRAPH to produce a nice picture in PostScript format :)
#	-DLONGCNF to use long long for config vectiors, allowing 64 terminals
#	-DPTHREADS to use multithreading in solver. Usable with Linux Threads.

#CFLAGS = -DLONGCNF -Icsspdms -g -O3 \
#	-finline-functions -funroll-loops -ffast-math \
#	-DPSGRAPH

set (SOURCES
    ael_gm.c
    ael_lm.c
    avltree.c
    avltree.h
    cross_gm.c
    dots_lm.c
    dump_gm.c
    edget_gm.c
    elds_lm.c
    eqs_lm.c
    head_gm.h
    head_lm.h
    inpfiles.c
    input_lm.c
    lmeter.c
    lmeter.h
    main_gm.c
    main_lm.c
    misc.h
    rcnf_lm.c
    show_ps.c
    solve.c
    sysdpnd.h
)

add_executable(lmeter
    ${SOURCES}
)

add_subdirectory(csspdms)

target_link_libraries(lmeter
    csspdms
    m
)

#test: lmeter
#	(cd test; ../lmeter input.lm < test90.dxc)
#
