// DvIni.h
#ifndef  _DvIni_H_
#define  _DvIni_H_

extern int   iVerbosity;
#define VERB1 (iVerbosity > 0)
#define VERB2 (iVerbosity > 1)
#define VERB3 (iVerbosity > 2)
#define VERB4 (iVerbosity > 3)
#define VERB5 (iVerbosity > 4)
#define VERB6 (iVerbosity > 5)
#define VERB7 (iVerbosity > 6)
#define VERB8 (iVerbosity > 7)
#define VERB9 (iVerbosity > 8)
#define VERB10 (iVerbosity > 9)
#define VERB11 (iVerbosity > 10)
#define VERB12 (iVerbosity > 11)

#ifdef   USENEWWINSIZE
// TCHAR szGOut[] = "OutSaved";
extern BOOL gbGotWP;
extern WINDOWPLACEMENT g_sWP;
extern VOID UpdateWP( HWND hwnd );
// FIX20080316 - get RUNTIME PATH (exclude DEBUG folder)
extern void  GetModulePath( PTSTR lpb );
#endif   //  USENEWWINSIZE
extern void GetAppData(PTSTR);

extern char gszAppData[];	// [260] = "\0";
extern char gszCacheData[]; //  [260] = "\0";


#endif // _DvIni_H_
// eof - DvIni.h

