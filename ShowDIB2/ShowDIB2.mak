# Microsoft Developer Studio Generated NMAKE File, Based on ShowDIB2.dsp
!IF "$(CFG)" == ""
CFG=ShowDIB2 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ShowDIB2 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ShowDIB2 - Win32 Release" && "$(CFG)" != "ShowDIB2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ShowDIB2.mak" CFG="ShowDIB2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ShowDIB2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ShowDIB2 - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ShowDIB2.exe"


CLEAN :
	-@erase "$(INTDIR)\DiagFile.obj"
	-@erase "$(INTDIR)\Dib.obj"
	-@erase "$(INTDIR)\Dlgopen.obj"
	-@erase "$(INTDIR)\Drawdib.obj"
	-@erase "$(INTDIR)\preview.obj"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Rgb.obj"
	-@erase "$(INTDIR)\SD2Util.obj"
	-@erase "$(INTDIR)\Showdib.obj"
	-@erase "$(INTDIR)\ShowDIB2.obj"
	-@erase "$(INTDIR)\ShowDIB2.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ShowDIB2.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x40c /fo"$(INTDIR)\ShowDIB2.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ShowDIB2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\ShowDIB2.pdb" /machine:I386 /out:"$(OUTDIR)\ShowDIB2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\DiagFile.obj" \
	"$(INTDIR)\Dib.obj" \
	"$(INTDIR)\Dlgopen.obj" \
	"$(INTDIR)\Drawdib.obj" \
	"$(INTDIR)\preview.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\Rgb.obj" \
	"$(INTDIR)\SD2Util.obj" \
	"$(INTDIR)\Showdib.obj" \
	"$(INTDIR)\ShowDIB2.obj" \
	"$(INTDIR)\ShowDIB2.res"

"$(OUTDIR)\ShowDIB2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ShowDIB2.exe"


CLEAN :
	-@erase "$(INTDIR)\DiagFile.obj"
	-@erase "$(INTDIR)\Dib.obj"
	-@erase "$(INTDIR)\Dlgopen.obj"
	-@erase "$(INTDIR)\Drawdib.obj"
	-@erase "$(INTDIR)\preview.obj"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Rgb.obj"
	-@erase "$(INTDIR)\SD2Util.obj"
	-@erase "$(INTDIR)\Showdib.obj"
	-@erase "$(INTDIR)\ShowDIB2.obj"
	-@erase "$(INTDIR)\ShowDIB2.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ShowDIB2.exe"
	-@erase "$(OUTDIR)\ShowDIB2.ilk"
	-@erase "$(OUTDIR)\ShowDIB2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x40c /fo"$(INTDIR)\ShowDIB2.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ShowDIB2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\ShowDIB2.pdb" /debug /machine:I386 /out:"$(OUTDIR)\ShowDIB2.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\DiagFile.obj" \
	"$(INTDIR)\Dib.obj" \
	"$(INTDIR)\Dlgopen.obj" \
	"$(INTDIR)\Drawdib.obj" \
	"$(INTDIR)\preview.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\Rgb.obj" \
	"$(INTDIR)\SD2Util.obj" \
	"$(INTDIR)\Showdib.obj" \
	"$(INTDIR)\ShowDIB2.obj" \
	"$(INTDIR)\ShowDIB2.res"

"$(OUTDIR)\ShowDIB2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("ShowDIB2.dep")
!INCLUDE "ShowDIB2.dep"
!ELSE 
!MESSAGE Warning: cannot find "ShowDIB2.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ShowDIB2 - Win32 Release" || "$(CFG)" == "ShowDIB2 - Win32 Debug"
SOURCE=.\DiagFile.cpp

"$(INTDIR)\DiagFile.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Showdib\Dib.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Dib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Dib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\Dlgopen.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Dlgopen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Dlgopen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\Drawdib.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Drawdib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Drawdib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\preview.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\preview.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\preview.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\Print.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Print.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Print.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\Rgb.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Rgb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Rgb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\SD2Util.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\SD2Util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\SD2Util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Showdib\Showdib.c

!IF  "$(CFG)" == "ShowDIB2 - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /ML /W3 /WX /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Showdib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ShowDIB2 - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MLd /W3 /WX /Gm /GX /ZI /Od /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "DV322" /D "SHOWDIB2" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Showdib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ShowDIB2.cpp

"$(INTDIR)\ShowDIB2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ShowDIB2.rc

"$(INTDIR)\ShowDIB2.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

