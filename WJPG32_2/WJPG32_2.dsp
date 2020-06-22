# Microsoft Developer Studio Project File - Name="WJPG32_2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WJPG32_2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WJPG32_2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WJPG32_2.mak" CFG="WJPG32_2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WJPG32_2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WJPG32_2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WJPG32_2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Zp1 /MT /W3 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "WJPG32_2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Zp1 /MTd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "WJPG32_2 - Win32 Release"
# Name "WJPG32_2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Jcapimin.c
# End Source File
# Begin Source File

SOURCE=.\Jcapistd.c
# End Source File
# Begin Source File

SOURCE=.\Jccoefct.c
# End Source File
# Begin Source File

SOURCE=.\Jccolor.c
# End Source File
# Begin Source File

SOURCE=.\Jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=.\Jchuff.c
# End Source File
# Begin Source File

SOURCE=.\Jcinit.c
# End Source File
# Begin Source File

SOURCE=.\Jcmainct.c
# End Source File
# Begin Source File

SOURCE=.\Jcmarker.c
# End Source File
# Begin Source File

SOURCE=.\Jcmaster.c
# End Source File
# Begin Source File

SOURCE=.\Jcomapi.c
# End Source File
# Begin Source File

SOURCE=.\Jcparam.c
# End Source File
# Begin Source File

SOURCE=.\Jcphuff.c
# End Source File
# Begin Source File

SOURCE=.\Jcprepct.c
# End Source File
# Begin Source File

SOURCE=.\Jcsample.c
# End Source File
# Begin Source File

SOURCE=.\Jctrans.c
# End Source File
# Begin Source File

SOURCE=.\Jdapimin.c
# End Source File
# Begin Source File

SOURCE=.\Jdapistd.c
# End Source File
# Begin Source File

SOURCE=.\Jdcoefct.c
# End Source File
# Begin Source File

SOURCE=.\Jdcolor.c
# End Source File
# Begin Source File

SOURCE=.\Jddctmgr.c
# End Source File
# Begin Source File

SOURCE=.\Jdhuff.c
# End Source File
# Begin Source File

SOURCE=.\Jdinput.c
# End Source File
# Begin Source File

SOURCE=.\Jdmainct.c
# End Source File
# Begin Source File

SOURCE=.\Jdmarker.c
# End Source File
# Begin Source File

SOURCE=.\Jdmaster.c
# End Source File
# Begin Source File

SOURCE=.\Jdmerge.c
# End Source File
# Begin Source File

SOURCE=.\Jdphuff.c
# End Source File
# Begin Source File

SOURCE=.\Jdpostct.c
# End Source File
# Begin Source File

SOURCE=.\Jdsample.c
# End Source File
# Begin Source File

SOURCE=.\Jdtrans.c
# End Source File
# Begin Source File

SOURCE=.\Jerror.c
# End Source File
# Begin Source File

SOURCE=.\Jfdctflt.c
# End Source File
# Begin Source File

SOURCE=.\Jfdctfst.c
# End Source File
# Begin Source File

SOURCE=.\Jfdctint.c
# End Source File
# Begin Source File

SOURCE=.\Jidctflt.c
# End Source File
# Begin Source File

SOURCE=.\Jidctfst.c
# End Source File
# Begin Source File

SOURCE=.\Jidctint.c
# End Source File
# Begin Source File

SOURCE=.\Jidctred.c
# End Source File
# Begin Source File

SOURCE=.\Jquant1.c
# End Source File
# Begin Source File

SOURCE=.\Jquant2.c
# End Source File
# Begin Source File

SOURCE=.\Jutils.c
# End Source File
# Begin Source File

SOURCE=.\WBmp2Jpg.c
# End Source File
# Begin Source File

SOURCE=.\WComCfg.c
# End Source File
# Begin Source File

SOURCE=.\WDataDst.c
# End Source File
# Begin Source File

SOURCE=.\Wdatasrc.c
# End Source File
# Begin Source File

SOURCE=.\Wjpg2bmp.c
# End Source File
# Begin Source File

SOURCE=.\WJPG32_2.def
# End Source File
# Begin Source File

SOURCE=.\WjpgMain.c
# End Source File
# Begin Source File

SOURCE=.\WMemMgr6.c
# End Source File
# Begin Source File

SOURCE=.\Wmemnobs.c
# End Source File
# Begin Source File

SOURCE=.\WRdBmp6.c
# End Source File
# Begin Source File

SOURCE=.\WWrBmp6.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\Cderror.h
# End Source File
# Begin Source File

SOURCE=.\Cdjpeg.h
# End Source File
# Begin Source File

SOURCE=.\Jchuff.h
# End Source File
# Begin Source File

SOURCE=.\Jconfig.h
# End Source File
# Begin Source File

SOURCE=.\Jdct.h
# End Source File
# Begin Source File

SOURCE=.\Jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\Jerror.h
# End Source File
# Begin Source File

SOURCE=.\Jinclude.h
# End Source File
# Begin Source File

SOURCE=.\Jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\Jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\Jpegint.h
# End Source File
# Begin Source File

SOURCE=.\Jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\Jpgstru6.h
# End Source File
# Begin Source File

SOURCE=.\Jversion.h
# End Source File
# Begin Source File

SOURCE=.\WComCfg.h
# End Source File
# Begin Source File

SOURCE=.\Win32.h
# End Source File
# Begin Source File

SOURCE=.\WMemDiag.h
# End Source File
# Begin Source File

SOURCE=.\WWrBmp6.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\wjpg32.ico
# End Source File
# Begin Source File

SOURCE=.\Wjpg32_2.rc
# End Source File
# End Group
# End Target
# End Project
