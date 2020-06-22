
# CMakeLists.txt, generated gencmake.pl, on 2020/05/04 16:44:53
cmake_minimum_required( VERSION 3.6 )

# CMakeScripts or use the ones that come by default with CMake.
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

project( DibView )

# The version number.
set( dv32_MAJOR 3 )
set( dv32_MINOR 0 )
set( dv32_POINT 0 )


# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Set ON to build shared Library (DLL)" OFF )
option( ADD_SHOWDIB_EXE  "Set ON to build the SHOWDIB EXE"  OFF )
option( ADD_SHOWDIB2_EXE "Set ON to build the SHOWDIB2 EXE" OFF )
option( ADD_WJPG32_2_DLL "Set ON to build the ADD_WJPG32_2 DLL" OFF )
option( ADD_WJPEG32_DLL  "Set ON to build the WJPEG32 lib" OFF )

# check 64 bit
if( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "4" )
   set( BITNESS x86 )
   set( HAVE_64_BIT 0 )
   message(STATUS "*** Seems sizeof(void *) = 4 == 32-bits")
else()
   if( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8" )
        set( HAVE_64_BIT 1 )
        set( BITNESS x64 )
        message(STATUS "*** Seems sizeof(void *) == 8 == 64-bits")
        if (MSVC)
            add_definitions( -DWIN64 )
        endif ()
   else ()
        message(FATAL_ERROR "Variable CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P} NOT 4 or 8! *** FIX ME ***")
   endif ()
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    foreach(warning 4244 4267)
        set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    endforeach(warning)
    # other falgs -DNOMINMAX  -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS
    set( MSVC_FLAGS "-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS" )
    #set( NOMINMAX 1 )
    # to distinguish between debug and release libs
    set( CMAKE_DEBUG_POSTFIX "d" )
    list(APPEND add_LIBS comctl32.lib winmm.lib)
else()
    # items for unix
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

#------------------------------------------------------------------------
# Static Windows Runtime
#   Option to statically link to the Windows runtime. Maybe only 
#   applies to WIN32/MSVC.
#------------------------------------------------------------------------
if (MSVC)
    option( USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" ON )
    if (USE_STATIC_RUNTIME)
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()    
        message(STATUS "Using /MT STATIC runtime")
    else ()
        message(STATUS "Using /MD DYNAMIC runtime")
    endif ()
endif ()



# configuration file, if needed
# configure_file( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
# add_definitions( -DHAVE_CONFIG_H )
# include_directories( ${CMAKE_BINARY_DIR} )
##add_definitions( -DDIBVIEW2 ) # to be check CHECKME
add_definitions( -DINCLWMCODE )

if(BUILD_SHARED_LIB)
   set(LIB_TYPE SHARED)
   message(STATUS "*** Building DLL library ${LIB_TYPE}")
else()
   message(STATUS "*** Option BUILD_SHARED_LIB is OFF ${LIB_TYPE}")
endif()
# Total libs 1, exes 5
#################################################
##### LIBRARIES 1 #####
#################################################

# DvCAM LIBRARY from [misc],
# have 1 C/C++ sources, 0 headers
#set(name DvCAM)
#set(dir misc)
#set(${name}_SRCS
#    ${dir}/DvCAM.cpp
#    )
#add_library( ${name} ${${name}_SRCS} )
#list(APPEND add_LIBS ${name})
# deal with install, if any...
#install( TARGETS ${name}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib )
#install(FILES ${${name}_HDRS} DESTINATION include)
#################################################
##### EXECUTABLE 5 #####
#################################################

# src EXECUTABLE from [src],
# have 63 C/C++ sources, 1 rc, 56 headers
# exclude  ${dir}/Utilgrm.c  ${dir}/Utilgrm.h
set(name Dv32)
set(dir src)
include_directories( ${dir} )
set(${name}_SRCS
    ${dir}/DVHELP.c
    ${dir}/DvClipDlg.c
    ${dir}/DvContext.c
    ${dir}/DvGlaze.c
    ${dir}/DvMagnify.c
    ${dir}/DvRGB.c
    ${dir}/DvTif.c
    ${dir}/DvTrack.c
    ${dir}/DvType.c
    ${dir}/DvUnused.c
    ${dir}/Dvabout.c
    ${dir}/Dvb2g.c
    ${dir}/Dvb2g2.c
    ${dir}/Dvcapt.c
    ${dir}/Dvchild.c
    ${dir}/Dvclip.c
    ${dir}/Dvdata.c
    ${dir}/Dvdebug.c
    ${dir}/Dvdiag.c
    ${dir}/Dvdib.c
    ${dir}/Dvdrives.c
    ${dir}/Dvdupe.c
    ${dir}/Dvedit.c
    ${dir}/Dvenum.c
    ${dir}/Dverrors.c
    ${dir}/Dvfile.c
    ${dir}/Dvfile1.c
    ${dir}/Dvfile2.c
    ${dir}/Dvfile3.c
    ${dir}/Dvframe.c
    ${dir}/Dvgetgif.c
    ${dir}/Dvimage.c
    ${dir}/Dvinfo.c
    ${dir}/Dvini.c
    ${dir}/Dvinit.c
    ${dir}/Dvkey.c
    ${dir}/Dvlib.c
    ${dir}/Dvlib2.c
    ${dir}/Dvlibdlg.c
    ${dir}/Dvmain.c
    ${dir}/Dvmenu.c
    ${dir}/Dvnewf.c
    ${dir}/Dvopen.c
    ${dir}/Dvopt1.c
    ${dir}/Dvopt2.c
    ${dir}/Dvopt3.c
    ${dir}/Dvopt4.c
    ${dir}/Dvpaint.c
    ${dir}/Dvpal.c
    ${dir}/Dvpal24.c
    ${dir}/Dvprint.c
    ${dir}/Dvprint2.c
    ${dir}/Dvprogm.c
    ${dir}/Dvread.c
    ${dir}/Dvscanf.c
    ${dir}/Dvsound.c
    ${dir}/Dvsrch.c
    ${dir}/Dvtbl32.c
    ${dir}/Dvtimer.c
    ${dir}/Dvutil2.c
    ${dir}/Dvwait.c
    ${dir}/Dvwarn.c
    ${dir}/Dvwrite.c
    ${dir}/Dv32.rc
    ${dir}/Dvmem.c
    ${dir}/DvUtils.c
    ${dir}/wmDiag.c
    ${dir}/DvTrans.c
    )
set(${name}_HDRS
    ${dir}/DVENUM.h
    ${dir}/DVFUNC.h
    ${dir}/DVINIT.h
    ${dir}/DVLIB.h
    ${dir}/DVLIST.h
    ${dir}/DVPAINT.h
    ${dir}/DVPAL24.h
    ${dir}/DVPROGM.h
    ${dir}/DVROTATE.h
    ${dir}/DVSRCH.h
    ${dir}/Dv.h
    ${dir}/DvGlaze.h
    ${dir}/DvIni.h
    ${dir}/DvMagnify.h
    ${dir}/DvMagnify2.h
    ${dir}/DvResS.h
    ${dir}/DvTif.h
    ${dir}/DvTrack.h
    ${dir}/DvType.h
    ${dir}/DvUtil2.h
    ${dir}/Dvabout.h
    ${dir}/Dvb2g.h
    ${dir}/Dvcapt.h
    ${dir}/Dvchild.h
    ${dir}/Dvclip.h
    ${dir}/Dvdata.h
    ${dir}/Dvdiag.h
    ${dir}/Dvdib.h
    ${dir}/Dverrors.h
    ${dir}/Dvfile.h
    ${dir}/Dvfile2.h
    ${dir}/Dvframe.h
    ${dir}/Dvgif.h
    ${dir}/Dvgif2.h
    ${dir}/Dvinfo.h
    ${dir}/Dvmem.h
    ${dir}/Dvmemf.h
    ${dir}/Dvopts.h
    ${dir}/Dvpal.h
    ${dir}/Dvpal2.h
    ${dir}/Dvprint.h
    ${dir}/Dvres.h
    ${dir}/Dvtrans.h
    ${dir}/Dvvers.h
    ${dir}/Dvview.h
    ${dir}/Dvwait.h
    ${dir}/Dvwarn.h
    ${dir}/Dvwin32.h
    ${dir}/Dvwork.h
    ${dir}/Gmutils.h
    ${dir}/WJPGLIB2.h
    ${dir}/Win32.h
    ${dir}/Winlibs.h
    ${dir}/Wjpeglib.h
    ${dir}/Wmdiag.h
    ${dir}/icc.h
    ${dir}/wmDiag99.h
    ${dir}/DvTrans.h
    )
add_executable( ${name}  WIN32 ${${name}_SRCS} ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

if(ADD_WJPG32_2_DLL)
# WJPG32_2 EXECUTABLE from [WJPG32_2],
# have 52 C/C++ sources, 1 rc, 19 headers
set(name WJPG32_2)
set(dir WJPG32_2)
#     ${dir}/Win32.h
include_directories( ${dir} )
if (NOT ADD_WJPEG32_DLL)
include_directories( WJPG32 )
endif ()
set(${name}_SRCS
    ${dir}/JCAPIMIN.c
    ${dir}/JCAPISTD.c
    ${dir}/JCCOEFCT.c
    ${dir}/JCCOLOR.c
    ${dir}/JCDCTMGR.c
    ${dir}/JCHUFF.c
    ${dir}/JCINIT.c
    ${dir}/JCMAINCT.c
    ${dir}/JCMARKER.c
    ${dir}/JCMASTER.c
    ${dir}/JCOMAPI.c
    ${dir}/JCPARAM.c
    ${dir}/JCPHUFF.c
    ${dir}/JCPREPCT.c
    ${dir}/JCSAMPLE.c
    ${dir}/JCTRANS.c
    ${dir}/JDAPIMIN.c
    ${dir}/JDAPISTD.c
    ${dir}/JDCOEFCT.c
    ${dir}/JDCOLOR.c
    ${dir}/JDDCTMGR.c
    ${dir}/JDHUFF.c
    ${dir}/JDINPUT.c
    ${dir}/JDMAINCT.c
    ${dir}/JDMARKER.c
    ${dir}/JDMASTER.c
    ${dir}/JDMERGE.c
    ${dir}/JDPHUFF.c
    ${dir}/JDPOSTCT.c
    ${dir}/JDSAMPLE.c
    ${dir}/JDTRANS.c
    ${dir}/JERROR.c
    ${dir}/JFDCTFLT.c
    ${dir}/JFDCTFST.c
    ${dir}/JFDCTINT.c
    ${dir}/JIDCTFLT.c
    ${dir}/JIDCTFST.c
    ${dir}/JIDCTINT.c
    ${dir}/JIDCTRED.c
    ${dir}/JQUANT1.c
    ${dir}/JQUANT2.c
    ${dir}/JUTILS.c
    ${dir}/WBMP2JPG.c
    ${dir}/WCOMCFG.c
    ${dir}/WDATADST.c
    ${dir}/WDATASRC.c
    ${dir}/WJPG2BMP.c
    ${dir}/WJPGMAIN.c
    ${dir}/WMEMMGR6.c
    ${dir}/WMEMNOBS.c
    ${dir}/WRDBMP6.c
    ${dir}/WWRBMP6.c
    ${dir}/WJPG32_2.RC
    )
set(${name}_HDRS
    ${dir}/CDERROR.h
    ${dir}/CDJPEG.h
    ${dir}/JDCT.h
    ${dir}/JDHUFF.h
    ${dir}/JERROR.h
    ${dir}/JPEGINT.h
    ${dir}/JPGSTRU6.h
    ${dir}/Jchuff.h
    ${dir}/Jconfig.h
    ${dir}/Jinclude.h
    ${dir}/Jmemsys.h
    ${dir}/Jmorecfg.h
    ${dir}/Jpeglib.h
    ${dir}/Jversion.h
    ${dir}/RESOURCE.h
    ${dir}/WCOMCFG.h
    ${dir}/WMEMDIAG.h
    ${dir}/WWRBMP6.h
    )
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
#add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
#if (add_LIBS)
#    target_link_libraries( ${name} ${add_LIBS} )
#endif ()
#if (MSVC)
#    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
#endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )
endif()

if(ADD_WJPEG32_DLL)
# WJPEG32 EXECUTABLE from [WJPEG32],
# have 40 C/C++ sources, 1 rc, 11 headers
set(name WJPEG32)
set(dir WJPEG32)
include_directories( ${dir} )
if(NOT ADD_WJPG32_2_DLL)
include_directories( WJPG32_2 )
endif()

set(${name}_SRCS
    ${dir}/DVTOJPG.c
    ${dir}/WBSMOOTH.c
    ${dir}/WCCOLOR.c
    ${dir}/WCDEFLTS.c
    ${dir}/WCEXPAND.c
    ${dir}/WCHUFF.c
    ${dir}/WCMAIND.c
    ${dir}/WCMASTER.c
    ${dir}/WCMCU.c
    ${dir}/WCOMMON.c
    ${dir}/WCPIPE.c
    ${dir}/WCSAMPLE.c
    ${dir}/WDCOLOR.c
    ${dir}/WDDEFLTS.c
    ${dir}/WDHUFF.c
    ${dir}/WDMAIN.c
    ${dir}/WDMASTER.c
    ${dir}/WDMCU.c
    ${dir}/WDPHUFF.c
    ${dir}/WDPIPE.c
    ${dir}/WDSAMPLE.c
    ${dir}/WERROR.c
    ${dir}/WFWDDCT.c
    ${dir}/WGMAIN.c
    ${dir}/WMEMMGR.c
    ${dir}/WMEMSYS.c
    ${dir}/WQUANT1.c
    ${dir}/WQUANT2.c
    ${dir}/WRDBMP.c
    ${dir}/WRDGIF.c
    ${dir}/WRDJFIF.c
    ${dir}/WRDPPM.c
    ${dir}/WRDTARGA.c
    ${dir}/WREVDCT.c
    ${dir}/WUTILS.c
    ${dir}/WWRBMP.c
    ${dir}/WWRGIF.c
    ${dir}/WWRJFIF.c
    ${dir}/WWRPPM.c
    ${dir}/WWRTARGA.c
    ${dir}/WJPEG32.RC
    )
set(${name}_HDRS
    ${dir}/RESOURCE.h
    ${dir}/SETEXIT.h
    ${dir}/Wbmpsup.h
    ${dir}/Wcommon.h
    ${dir}/Wconfig.h
    ${dir}/Winclude.h
    ${dir}/Wjpegdat.h
    ${dir}/Wjpeglib.h
    ${dir}/Wmemsys.h
    ${dir}/Wversion.h
    ${dir}/wjdhuff.h
    )
#add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
#if (add_LIBS)
#    target_link_libraries( ${name} ${add_LIBS} )
#endif ()
#if (MSVC)
#    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
#endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )
endif(ADD_WJPEG32_DLL)

if(ADD_SHOWDIB2_EXE)
# ShowDIB2 EXECUTABLE from [ShowDIB2],
# have 4 C/C++ sources, 1 rc, 7 headers
set(name ShowDIB2)
set(dir ShowDIB2)
set(${name}_SRCS
    ${dir}/DiagFile.cpp
    ${dir}/SD2Util.cpp
    ${dir}/ShowDIB2.cpp
    ${dir}/StdAfx.cpp
    ${dir}/ShowDIB2.rc
    )
set(${name}_HDRS
    ${dir}/DiagFile.h
    ${dir}/SD2Util.h
    ${dir}/SDVers.h
    ${dir}/ShowDIB2.h
    ${dir}/StdAfx.h
    ${dir}/dibshow.h
    ${dir}/resource.h
    )
add_executable( ${name}  WIN32 ${${name}_SRCS} ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )
endif(ADD_SHOWDIB2_EXE)

if(ADD_SHOWDIB_EXE)
# SHOWDIB EXECUTABLE from [ShowDIB2\SHOWDIB],
# have 7 C/C++ sources, 1 rc, 5 headers
set(name SHOWDIB)
set(dir ShowDIB2/SHOWDIB)
set(${name}_SRCS
    ${dir}/Dib.c
    ${dir}/Dlgopen.c
    ${dir}/Drawdib.c
    ${dir}/Print.c
    ${dir}/Rgb.c
    ${dir}/Showdib.c
    ${dir}/preview.c
    ${dir}/SHOWDIB.RC
    )
set(${name}_HDRS
    ${dir}/Dlgopen.h
    ${dir}/RES.h
    ${dir}/Rgb.h
    ${dir}/Showdib.h
    ${dir}/preview.h
    )
add_executable( ${name}  WIN32 ${${name}_SRCS} ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )
endif(ADD_SHOWDIB_EXE)

# eof
