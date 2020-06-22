
// Utilgrm.h
#ifndef  _Utilgrm_H
#define  _Utilgrm_H

#define  WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>

#define  IS_FILE     1
#define  IS_FOLDER   2
#define  IS_WILD     4
extern   DWORD TrimIB( LPTSTR lps );

extern   INT   InStr( LPTSTR lpb, LPTSTR lps );
extern   INT   InStri( LPTSTR lpb, LPTSTR lps );

extern   LPTSTR   Mid( LPTSTR lpl, DWORD dwb, DWORD dwl );
extern   LPTSTR   Left( LPTSTR lpl, DWORD dwi );
extern   LPTSTR   Right( LPTSTR lpl, DWORD dwl );

extern   BOOL  strbgn( LPTSTR lps, LPTSTR lpd );
extern   BOOL  Stg2SysTm( LPTSTR lps, SYSTEMTIME * pt );

extern   int   _cdecl sprtf( LPTSTR lpf, ... );
// something that needs to be checked
// begin "CR"[ITICAL] or "C:" to put up MessageBox(Abort App?)
extern   int  _cdecl chkme( LPTSTR lpf, ... );

extern  VOID  GetModulePath( LPTSTR lpb );
extern   DWORD  IsValidFile4( LPTSTR lpf, PWIN32_FIND_DATA pfd );
extern   DWORD  IsValidFile( LPTSTR lpf );

extern   VOID  CloseDiag(VOID);
extern   VOID  CreateDiag( VOID );
extern   void  oi( LPTSTR lpb );
extern   int   prt( LPTSTR lpd );

extern   VOID  AppendDateTime( LPTSTR lpb, LPSYSTEMTIME pst );
extern   LPTSTR   GetDT5Stg( VOID );
extern   LPTSTR   GetTmStg( VOID );

// A 20K rotating buffer, each 1K
// #define	MXLINES	20
// #define	MXLINEB	1024
// static TCHAR	s_szLnBuf[ MXLINES * MXLINEB2 ];
extern   LPTSTR   GetNxtBuf( VOID );

// center dialogs on owner window
extern   BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
extern   LPTSTR Rect2Stg( PRECT lpr );

extern   LPTSTR   Dbl2Stg( double value, int prec );  // say num. of secs 1.004
// convert _ecvt() output to reasonable #0.#### shape
extern   void Buffer2Str( LPTSTR lps, LPTSTR lpb, int decimal,
				 int sign, int precision );

// not really needed, see GTM sGtm[MXTMS] = { 0 }; // init to ZERO in Utilgrm.c
extern   VOID  InitTimers( VOID );

//int  GetTimer( PGTM * pptm )
// and mindful of a MAX TIMER value - in use at one time
extern   int   GetMTime( void );    // get MAX timers avail
extern   int   GetCTime( void );    // get used timers

// *START*
//extern   BOOL  GetGTimer( void );   // if one is free
extern   int  GetBTime( void );
// RUNNING
// get a snapshot of the time passing
extern   double GetRTime( int i );
// *STOP*
// get and CLOSE / FREE this timer block;
extern   double GetETime( int i );

#ifndef  EndBuf
#define  EndBuf(a)   ( a + strlen(a) )
#endif   // EndBuf

#ifndef  VFH
#define  VFH(a)   ( a && ( a != INVALID_HANDLE_VALUE ) )
#endif   // !VFH

extern   VOID  CloseLog( VOID );
extern   VOID  WriteLog( LPTSTR lpb );
extern   HANDLE  CreateLog( LPTSTR lpf );

extern   LPTSTR   GetTitleStg( LPTSTR lpf );

#endif   // _Utilgrm_H
// eof - Utilgrm.h
