# Microsoft Developer Studio Project File - Name="WJPEG32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WJPEG32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WJPEG32.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WJPEG32.MAK" CFG="WJPEG32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WJPEG32 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WJPEG32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WJPEG32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WJPEG32_"
# PROP BASE Intermediate_Dir "WJPEG32_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "WJPEG32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WJPEG320"
# PROP BASE Intermediate_Dir "WJPEG320"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MTd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "WJPEG32 - Win32 Release"
# Name "WJPEG32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\Dvtojpg.c
# End Source File
# Begin Source File

SOURCE=.\Wbsmooth.c
# End Source File
# Begin Source File

SOURCE=.\Wccolor.c
# End Source File
# Begin Source File

SOURCE=.\Wcdeflts.c
# End Source File
# Begin Source File

SOURCE=.\Wcexpand.c
# End Source File
# Begin Source File

SOURCE=.\Wchuff.c
# End Source File
# Begin Source File

SOURCE=.\Wcmaind.c
# End Source File
# Begin Source File

SOURCE=.\Wcmaster.c
# End Source File
# Begin Source File

SOURCE=.\Wcmcu.c
# End Source File
# Begin Source File

SOURCE=.\Wcommon.c
# End Source File
# Begin Source File

SOURCE=.\Wcpipe.c
# End Source File
# Begin Source File

SOURCE=.\Wcsample.c
# End Source File
# Begin Source File

SOURCE=.\Wdcolor.c
# End Source File
# Begin Source File

SOURCE=.\Wddeflts.c
# End Source File
# Begin Source File

SOURCE=.\Wdhuff.c
# End Source File
# Begin Source File

SOURCE=.\Wdmain.c
# End Source File
# Begin Source File

SOURCE=.\Wdmaster.c
# End Source File
# Begin Source File

SOURCE=.\Wdmcu.c
# End Source File
# Begin Source File

SOURCE=.\Wdpipe.c
# End Source File
# Begin Source File

SOURCE=.\Wdsample.c
# End Source File
# Begin Source File

SOURCE=.\Werror.c
# End Source File
# Begin Source File

SOURCE=.\Wfwddct.c
# End Source File
# Begin Source File

SOURCE=.\Wgmain.c
# End Source File
# Begin Source File

SOURCE=.\Wjpeg32.def
# End Source File
# Begin Source File

SOURCE=.\Wmemmgr.c
# End Source File
# Begin Source File

SOURCE=.\Wmemsys.c
# End Source File
# Begin Source File

SOURCE=.\Wquant1.c
# End Source File
# Begin Source File

SOURCE=.\Wquant2.c
# End Source File
# Begin Source File

SOURCE=.\Wrdbmp.c
# End Source File
# Begin Source File

SOURCE=.\Wrdgif.c
# End Source File
# Begin Source File

SOURCE=.\Wrdjfif.c
# End Source File
# Begin Source File

SOURCE=.\Wrdppm.c
# End Source File
# Begin Source File

SOURCE=.\Wrdtarga.c
# End Source File
# Begin Source File

SOURCE=.\Wrevdct.c
# End Source File
# Begin Source File

SOURCE=.\Wutils.c
# End Source File
# Begin Source File

SOURCE=.\Wwrbmp.c
# End Source File
# Begin Source File

SOURCE=.\Wwrgif.c
# End Source File
# Begin Source File

SOURCE=.\Wwrjfif.c
# End Source File
# Begin Source File

SOURCE=.\Wwrppm.c
# End Source File
# Begin Source File

SOURCE=.\Wwrtarga.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\Wbmpsup.h
# End Source File
# Begin Source File

SOURCE=.\Wcommon.h
# End Source File
# Begin Source File

SOURCE=.\Wconfig.h
# End Source File
# Begin Source File

SOURCE=.\Winclude.h
# End Source File
# Begin Source File

SOURCE=.\Wjpegdat.h
# End Source File
# Begin Source File

SOURCE=.\Wjpeglib.h
# End Source File
# Begin Source File

SOURCE=.\Wmemsys.h
# End Source File
# Begin Source File

SOURCE=.\Wversion.h
# End Source File
# End Group
# End Target
# End Project
