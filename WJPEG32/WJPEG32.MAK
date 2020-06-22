# Microsoft Developer Studio Generated NMAKE File, Based on WJPEG32.DSP
!IF "$(CFG)" == ""
CFG=WJPEG32 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to WJPEG32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "WJPEG32 - Win32 Release" && "$(CFG)" != "WJPEG32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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

!IF  "$(CFG)" == "WJPEG32 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\WJPEG32.dll"


CLEAN :
	-@erase "$(INTDIR)\Dvtojpg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Wbsmooth.obj"
	-@erase "$(INTDIR)\Wccolor.obj"
	-@erase "$(INTDIR)\Wcdeflts.obj"
	-@erase "$(INTDIR)\Wcexpand.obj"
	-@erase "$(INTDIR)\Wchuff.obj"
	-@erase "$(INTDIR)\Wcmaind.obj"
	-@erase "$(INTDIR)\Wcmaster.obj"
	-@erase "$(INTDIR)\Wcmcu.obj"
	-@erase "$(INTDIR)\Wcommon.obj"
	-@erase "$(INTDIR)\Wcpipe.obj"
	-@erase "$(INTDIR)\Wcsample.obj"
	-@erase "$(INTDIR)\Wdcolor.obj"
	-@erase "$(INTDIR)\Wddeflts.obj"
	-@erase "$(INTDIR)\Wdhuff.obj"
	-@erase "$(INTDIR)\Wdmain.obj"
	-@erase "$(INTDIR)\Wdmaster.obj"
	-@erase "$(INTDIR)\Wdmcu.obj"
	-@erase "$(INTDIR)\Wdpipe.obj"
	-@erase "$(INTDIR)\Wdsample.obj"
	-@erase "$(INTDIR)\Werror.obj"
	-@erase "$(INTDIR)\Wfwddct.obj"
	-@erase "$(INTDIR)\Wgmain.obj"
	-@erase "$(INTDIR)\Wmemmgr.obj"
	-@erase "$(INTDIR)\Wmemsys.obj"
	-@erase "$(INTDIR)\Wquant1.obj"
	-@erase "$(INTDIR)\Wquant2.obj"
	-@erase "$(INTDIR)\Wrdbmp.obj"
	-@erase "$(INTDIR)\Wrdgif.obj"
	-@erase "$(INTDIR)\Wrdjfif.obj"
	-@erase "$(INTDIR)\Wrdppm.obj"
	-@erase "$(INTDIR)\Wrdtarga.obj"
	-@erase "$(INTDIR)\Wrevdct.obj"
	-@erase "$(INTDIR)\Wutils.obj"
	-@erase "$(INTDIR)\Wwrbmp.obj"
	-@erase "$(INTDIR)\Wwrgif.obj"
	-@erase "$(INTDIR)\Wwrjfif.obj"
	-@erase "$(INTDIR)\Wwrppm.obj"
	-@erase "$(INTDIR)\Wwrtarga.obj"
	-@erase "$(OUTDIR)\WJPEG32.dll"
	-@erase "$(OUTDIR)\WJPEG32.exp"
	-@erase "$(OUTDIR)\WJPEG32.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MT /W3 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WJPEG32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\WJPEG32.pdb" /machine:I386 /def:".\Wjpeg32.def" /out:"$(OUTDIR)\WJPEG32.dll" /implib:"$(OUTDIR)\WJPEG32.lib" 
DEF_FILE= \
	".\Wjpeg32.def"
LINK32_OBJS= \
	"$(INTDIR)\Dvtojpg.obj" \
	"$(INTDIR)\Wbsmooth.obj" \
	"$(INTDIR)\Wccolor.obj" \
	"$(INTDIR)\Wcdeflts.obj" \
	"$(INTDIR)\Wcexpand.obj" \
	"$(INTDIR)\Wchuff.obj" \
	"$(INTDIR)\Wcmaind.obj" \
	"$(INTDIR)\Wcmaster.obj" \
	"$(INTDIR)\Wcmcu.obj" \
	"$(INTDIR)\Wcommon.obj" \
	"$(INTDIR)\Wcpipe.obj" \
	"$(INTDIR)\Wcsample.obj" \
	"$(INTDIR)\Wdcolor.obj" \
	"$(INTDIR)\Wddeflts.obj" \
	"$(INTDIR)\Wdhuff.obj" \
	"$(INTDIR)\Wdmain.obj" \
	"$(INTDIR)\Wdmaster.obj" \
	"$(INTDIR)\Wdmcu.obj" \
	"$(INTDIR)\Wdpipe.obj" \
	"$(INTDIR)\Wdsample.obj" \
	"$(INTDIR)\Werror.obj" \
	"$(INTDIR)\Wfwddct.obj" \
	"$(INTDIR)\Wgmain.obj" \
	"$(INTDIR)\Wmemmgr.obj" \
	"$(INTDIR)\Wmemsys.obj" \
	"$(INTDIR)\Wquant1.obj" \
	"$(INTDIR)\Wquant2.obj" \
	"$(INTDIR)\Wrdbmp.obj" \
	"$(INTDIR)\Wrdgif.obj" \
	"$(INTDIR)\Wrdjfif.obj" \
	"$(INTDIR)\Wrdppm.obj" \
	"$(INTDIR)\Wrdtarga.obj" \
	"$(INTDIR)\Wrevdct.obj" \
	"$(INTDIR)\Wutils.obj" \
	"$(INTDIR)\Wwrbmp.obj" \
	"$(INTDIR)\Wwrgif.obj" \
	"$(INTDIR)\Wwrjfif.obj" \
	"$(INTDIR)\Wwrppm.obj" \
	"$(INTDIR)\Wwrtarga.obj"

"$(OUTDIR)\WJPEG32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WJPEG32 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\WJPEG32.dll"


CLEAN :
	-@erase "$(INTDIR)\Dvtojpg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Wbsmooth.obj"
	-@erase "$(INTDIR)\Wccolor.obj"
	-@erase "$(INTDIR)\Wcdeflts.obj"
	-@erase "$(INTDIR)\Wcexpand.obj"
	-@erase "$(INTDIR)\Wchuff.obj"
	-@erase "$(INTDIR)\Wcmaind.obj"
	-@erase "$(INTDIR)\Wcmaster.obj"
	-@erase "$(INTDIR)\Wcmcu.obj"
	-@erase "$(INTDIR)\Wcommon.obj"
	-@erase "$(INTDIR)\Wcpipe.obj"
	-@erase "$(INTDIR)\Wcsample.obj"
	-@erase "$(INTDIR)\Wdcolor.obj"
	-@erase "$(INTDIR)\Wddeflts.obj"
	-@erase "$(INTDIR)\Wdhuff.obj"
	-@erase "$(INTDIR)\Wdmain.obj"
	-@erase "$(INTDIR)\Wdmaster.obj"
	-@erase "$(INTDIR)\Wdmcu.obj"
	-@erase "$(INTDIR)\Wdpipe.obj"
	-@erase "$(INTDIR)\Wdsample.obj"
	-@erase "$(INTDIR)\Werror.obj"
	-@erase "$(INTDIR)\Wfwddct.obj"
	-@erase "$(INTDIR)\Wgmain.obj"
	-@erase "$(INTDIR)\Wmemmgr.obj"
	-@erase "$(INTDIR)\Wmemsys.obj"
	-@erase "$(INTDIR)\Wquant1.obj"
	-@erase "$(INTDIR)\Wquant2.obj"
	-@erase "$(INTDIR)\Wrdbmp.obj"
	-@erase "$(INTDIR)\Wrdgif.obj"
	-@erase "$(INTDIR)\Wrdjfif.obj"
	-@erase "$(INTDIR)\Wrdppm.obj"
	-@erase "$(INTDIR)\Wrdtarga.obj"
	-@erase "$(INTDIR)\Wrevdct.obj"
	-@erase "$(INTDIR)\Wutils.obj"
	-@erase "$(INTDIR)\Wwrbmp.obj"
	-@erase "$(INTDIR)\Wwrgif.obj"
	-@erase "$(INTDIR)\Wwrjfif.obj"
	-@erase "$(INTDIR)\Wwrppm.obj"
	-@erase "$(INTDIR)\Wwrtarga.obj"
	-@erase "$(OUTDIR)\WJPEG32.dll"
	-@erase "$(OUTDIR)\WJPEG32.exp"
	-@erase "$(OUTDIR)\WJPEG32.ilk"
	-@erase "$(OUTDIR)\WJPEG32.lib"
	-@erase "$(OUTDIR)\WJPEG32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MTd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DV322" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WJPEG32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\WJPEG32.pdb" /debug /machine:I386 /def:".\Wjpeg32.def" /out:"$(OUTDIR)\WJPEG32.dll" /implib:"$(OUTDIR)\WJPEG32.lib" /pdbtype:sept 
DEF_FILE= \
	".\Wjpeg32.def"
LINK32_OBJS= \
	"$(INTDIR)\Dvtojpg.obj" \
	"$(INTDIR)\Wbsmooth.obj" \
	"$(INTDIR)\Wccolor.obj" \
	"$(INTDIR)\Wcdeflts.obj" \
	"$(INTDIR)\Wcexpand.obj" \
	"$(INTDIR)\Wchuff.obj" \
	"$(INTDIR)\Wcmaind.obj" \
	"$(INTDIR)\Wcmaster.obj" \
	"$(INTDIR)\Wcmcu.obj" \
	"$(INTDIR)\Wcommon.obj" \
	"$(INTDIR)\Wcpipe.obj" \
	"$(INTDIR)\Wcsample.obj" \
	"$(INTDIR)\Wdcolor.obj" \
	"$(INTDIR)\Wddeflts.obj" \
	"$(INTDIR)\Wdhuff.obj" \
	"$(INTDIR)\Wdmain.obj" \
	"$(INTDIR)\Wdmaster.obj" \
	"$(INTDIR)\Wdmcu.obj" \
	"$(INTDIR)\Wdpipe.obj" \
	"$(INTDIR)\Wdsample.obj" \
	"$(INTDIR)\Werror.obj" \
	"$(INTDIR)\Wfwddct.obj" \
	"$(INTDIR)\Wgmain.obj" \
	"$(INTDIR)\Wmemmgr.obj" \
	"$(INTDIR)\Wmemsys.obj" \
	"$(INTDIR)\Wquant1.obj" \
	"$(INTDIR)\Wquant2.obj" \
	"$(INTDIR)\Wrdbmp.obj" \
	"$(INTDIR)\Wrdgif.obj" \
	"$(INTDIR)\Wrdjfif.obj" \
	"$(INTDIR)\Wrdppm.obj" \
	"$(INTDIR)\Wrdtarga.obj" \
	"$(INTDIR)\Wrevdct.obj" \
	"$(INTDIR)\Wutils.obj" \
	"$(INTDIR)\Wwrbmp.obj" \
	"$(INTDIR)\Wwrgif.obj" \
	"$(INTDIR)\Wwrjfif.obj" \
	"$(INTDIR)\Wwrppm.obj" \
	"$(INTDIR)\Wwrtarga.obj"

"$(OUTDIR)\WJPEG32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("WJPEG32.DEP")
!INCLUDE "WJPEG32.DEP"
!ELSE 
!MESSAGE Warning: cannot find "WJPEG32.DEP"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "WJPEG32 - Win32 Release" || "$(CFG)" == "WJPEG32 - Win32 Debug"
SOURCE=.\Dvtojpg.c

"$(INTDIR)\Dvtojpg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wbsmooth.c

"$(INTDIR)\Wbsmooth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wccolor.c

"$(INTDIR)\Wccolor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcdeflts.c

"$(INTDIR)\Wcdeflts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcexpand.c

"$(INTDIR)\Wcexpand.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wchuff.c

"$(INTDIR)\Wchuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcmaind.c

"$(INTDIR)\Wcmaind.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcmaster.c

"$(INTDIR)\Wcmaster.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcmcu.c

"$(INTDIR)\Wcmcu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcommon.c

"$(INTDIR)\Wcommon.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcpipe.c

"$(INTDIR)\Wcpipe.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wcsample.c

"$(INTDIR)\Wcsample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdcolor.c

"$(INTDIR)\Wdcolor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wddeflts.c

"$(INTDIR)\Wddeflts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdhuff.c

"$(INTDIR)\Wdhuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdmain.c

"$(INTDIR)\Wdmain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdmaster.c

"$(INTDIR)\Wdmaster.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdmcu.c

"$(INTDIR)\Wdmcu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdpipe.c

"$(INTDIR)\Wdpipe.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wdsample.c

"$(INTDIR)\Wdsample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Werror.c

"$(INTDIR)\Werror.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wfwddct.c

"$(INTDIR)\Wfwddct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wgmain.c

"$(INTDIR)\Wgmain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wmemmgr.c

"$(INTDIR)\Wmemmgr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wmemsys.c

"$(INTDIR)\Wmemsys.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wquant1.c

"$(INTDIR)\Wquant1.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wquant2.c

"$(INTDIR)\Wquant2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrdbmp.c

"$(INTDIR)\Wrdbmp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrdgif.c

"$(INTDIR)\Wrdgif.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrdjfif.c

"$(INTDIR)\Wrdjfif.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrdppm.c

"$(INTDIR)\Wrdppm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrdtarga.c

"$(INTDIR)\Wrdtarga.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wrevdct.c

"$(INTDIR)\Wrevdct.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wutils.c

"$(INTDIR)\Wutils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wwrbmp.c

"$(INTDIR)\Wwrbmp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wwrgif.c

"$(INTDIR)\Wwrgif.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wwrjfif.c

"$(INTDIR)\Wwrjfif.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wwrppm.c

"$(INTDIR)\Wwrppm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Wwrtarga.c

"$(INTDIR)\Wwrtarga.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

