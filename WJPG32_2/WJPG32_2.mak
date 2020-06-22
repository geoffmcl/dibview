# Microsoft Developer Studio Generated NMAKE File, Based on WJPG32_2.dsp
!IF "$(CFG)" == ""
CFG=WJPG32_2 - Win32 Release
!MESSAGE No configuration specified. Defaulting to WJPG32_2 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "WJPG32_2 - Win32 Release" && "$(CFG)" != "WJPG32_2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WJPG32_2 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\WJpg32_2.dll"


CLEAN :
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WBmp2Jpg.obj"
	-@erase "$(INTDIR)\WComCfg.obj"
	-@erase "$(INTDIR)\WDataDst.obj"
	-@erase "$(INTDIR)\Wdatasrc.obj"
	-@erase "$(INTDIR)\Wjpg2bmp.obj"
	-@erase "$(INTDIR)\Wjpg32_2.res"
	-@erase "$(INTDIR)\WjpgMain.obj"
	-@erase "$(INTDIR)\WMemMgr6.obj"
	-@erase "$(INTDIR)\Wmemnobs.obj"
	-@erase "$(INTDIR)\WRdBmp6.obj"
	-@erase "$(INTDIR)\WWrBmp6.obj"
	-@erase "$(OUTDIR)\WJpg32_2.dll"
	-@erase "$(OUTDIR)\WJPG32_2.exp"
	-@erase "$(OUTDIR)\WJPG32_2.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MT /W3 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Wjpg32_2.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WJPG32_2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\WJPG32_2.pdb" /machine:I386 /def:".\WJPG32_2.def" /out:"$(OUTDIR)\WJPG32_2.dll" /implib:"$(OUTDIR)\WJPG32_2.lib" 
DEF_FILE= \
	".\WJPG32_2.def"
LINK32_OBJS= \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\WBmp2Jpg.obj" \
	"$(INTDIR)\WComCfg.obj" \
	"$(INTDIR)\WDataDst.obj" \
	"$(INTDIR)\Wdatasrc.obj" \
	"$(INTDIR)\Wjpg2bmp.obj" \
	"$(INTDIR)\WjpgMain.obj" \
	"$(INTDIR)\WMemMgr6.obj" \
	"$(INTDIR)\Wmemnobs.obj" \
	"$(INTDIR)\WRdBmp6.obj" \
	"$(INTDIR)\WWrBmp6.obj" \
	"$(INTDIR)\Wjpg32_2.res"

"$(OUTDIR)\WJpg32_2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WJPG32_2 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\WJPG32_2.dll"


CLEAN :
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WBmp2Jpg.obj"
	-@erase "$(INTDIR)\WComCfg.obj"
	-@erase "$(INTDIR)\WDataDst.obj"
	-@erase "$(INTDIR)\Wdatasrc.obj"
	-@erase "$(INTDIR)\Wjpg2bmp.obj"
	-@erase "$(INTDIR)\Wjpg32_2.res"
	-@erase "$(INTDIR)\WjpgMain.obj"
	-@erase "$(INTDIR)\WMemMgr6.obj"
	-@erase "$(INTDIR)\Wmemnobs.obj"
	-@erase "$(INTDIR)\WRdBmp6.obj"
	-@erase "$(INTDIR)\WWrBmp6.obj"
	-@erase "$(OUTDIR)\WJPG32_2.dll"
	-@erase "$(OUTDIR)\WJPG32_2.exp"
	-@erase "$(OUTDIR)\WJPG32_2.ilk"
	-@erase "$(OUTDIR)\WJPG32_2.lib"
	-@erase "$(OUTDIR)\WJPG32_2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MTd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Wjpg32_2.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WJPG32_2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\WJPG32_2.pdb" /debug /machine:I386 /def:".\WJPG32_2.def" /out:"$(OUTDIR)\WJPG32_2.dll" /implib:"$(OUTDIR)\WJPG32_2.lib" 
DEF_FILE= \
	".\WJPG32_2.def"
LINK32_OBJS= \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\WBmp2Jpg.obj" \
	"$(INTDIR)\WComCfg.obj" \
	"$(INTDIR)\WDataDst.obj" \
	"$(INTDIR)\Wdatasrc.obj" \
	"$(INTDIR)\Wjpg2bmp.obj" \
	"$(INTDIR)\WjpgMain.obj" \
	"$(INTDIR)\WMemMgr6.obj" \
	"$(INTDIR)\Wmemnobs.obj" \
	"$(INTDIR)\WRdBmp6.obj" \
	"$(INTDIR)\WWrBmp6.obj" \
	"$(INTDIR)\Wjpg32_2.res"

"$(OUTDIR)\WJPG32_2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("WJPG32_2.dep")
!INCLUDE "WJPG32_2.dep"
!ELSE 
!MESSAGE Warning: cannot find "WJPG32_2.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "WJPG32_2 - Win32 Release" || "$(CFG)" == "WJPG32_2 - Win32 Debug"
SOURCE=.\Jcapimin.c

"$(INTDIR)\Jcapimin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcapistd.c

"$(INTDIR)\Jcapistd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jccoefct.c

"$(INTDIR)\Jccoefct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jccolor.c

"$(INTDIR)\Jccolor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcdctmgr.c

"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jchuff.c

"$(INTDIR)\Jchuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcinit.c

"$(INTDIR)\Jcinit.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcmainct.c

"$(INTDIR)\Jcmainct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcmarker.c

"$(INTDIR)\Jcmarker.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcmaster.c

"$(INTDIR)\Jcmaster.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcomapi.c

"$(INTDIR)\Jcomapi.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcparam.c

"$(INTDIR)\Jcparam.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcphuff.c

"$(INTDIR)\Jcphuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcprepct.c

"$(INTDIR)\Jcprepct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jcsample.c

"$(INTDIR)\Jcsample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jctrans.c

"$(INTDIR)\Jctrans.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdapimin.c

"$(INTDIR)\Jdapimin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdapistd.c

"$(INTDIR)\Jdapistd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdcoefct.c

"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdcolor.c

"$(INTDIR)\Jdcolor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jddctmgr.c

"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdhuff.c

"$(INTDIR)\Jdhuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdinput.c

"$(INTDIR)\Jdinput.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdmainct.c

"$(INTDIR)\Jdmainct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdmarker.c

"$(INTDIR)\Jdmarker.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdmaster.c

"$(INTDIR)\Jdmaster.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdmerge.c

"$(INTDIR)\Jdmerge.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdphuff.c

"$(INTDIR)\Jdphuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdpostct.c

"$(INTDIR)\Jdpostct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdsample.c

"$(INTDIR)\Jdsample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jdtrans.c

"$(INTDIR)\Jdtrans.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jerror.c

"$(INTDIR)\Jerror.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jfdctflt.c

"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jfdctfst.c

"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jfdctint.c

"$(INTDIR)\Jfdctint.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jidctflt.c

"$(INTDIR)\Jidctflt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jidctfst.c

"$(INTDIR)\Jidctfst.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jidctint.c

"$(INTDIR)\Jidctint.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jidctred.c

"$(INTDIR)\Jidctred.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jquant1.c

"$(INTDIR)\Jquant1.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jquant2.c

"$(INTDIR)\Jquant2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Jutils.c

"$(INTDIR)\Jutils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WBmp2Jpg.c

"$(INTDIR)\WBmp2Jpg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WComCfg.c

"$(INTDIR)\WComCfg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WDataDst.c

"$(INTDIR)\WDataDst.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdatasrc.c

"$(INTDIR)\Wdatasrc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wjpg2bmp.c

"$(INTDIR)\Wjpg2bmp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WjpgMain.c

"$(INTDIR)\WjpgMain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WMemMgr6.c

"$(INTDIR)\WMemMgr6.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wmemnobs.c

"$(INTDIR)\Wmemnobs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WRdBmp6.c

"$(INTDIR)\WRdBmp6.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WWrBmp6.c

"$(INTDIR)\WWrBmp6.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wjpg32_2.rc

"$(INTDIR)\Wjpg32_2.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

