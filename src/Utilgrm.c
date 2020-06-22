
// Utilgrm.c
// oct, 2001 - grm
// general utility functions ...

#include	"Utilgrm.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif   // _DEBUG

#ifdef   SPY5
extern   HWND  ghwndSpyApp;   // main app. window (for message box)
#define  ghMainWnd      ghwndSpyApp
#endif   // SPY5

// Fa4Util.c
// #include	"Fa4.h"
// #include	"Fa4Util.h"
#define	MXLINES	20
#define	MXLINEB	1024
#define	MXLINEB2	MXLINEB + 16

#ifndef  ISNUM
#define  ISNUM(a)    ( ( a >= '0' ) && ( a <= '9' ) )
#endif   // #ifndef ISNUM


//INT	g_iLnBuf = 0;
//TCHAR	g_szLnBuf[ MXLINES * MXLINEB2 ];

//TCHAR _s_sprtfbuf[1024];

// ***************************
/* ========================================================
 * MODULE :	Log.c
 */
INT   InStri( LPTSTR lpb, LPTSTR lps );
VOID  CloseDiag(VOID);
VOID  CreateDiag( VOID );
void  oi( LPTSTR lpb );
int   prt( LPTSTR lpd );
// something that needs to be checked- maybe CRITICAL
int  _cdecl chkme( LPTSTR lpf, ... );
VOID  AppendDateTime( LPTSTR lpb, LPSYSTEMTIME pst );
LPTSTR   GetDT5Stg( VOID );

#undef  USEMULTDIAG
// ======================================================================
HANDLE   ghDiag = 0;
TCHAR    gszDefDiag[] = TEXT("TEMPSPY5.TXT");
TCHAR    gszDiag[264];

static HANDLE   _sCreatDiag(VOID)
{
   ghDiag = CreateFile(
      gszDiag,                      // file name
      GENERIC_READ | GENERIC_WRITE, // access mode
      0,    // FILE_SHARE_READ,              // share mode
      0,                            // SD
      CREATE_ALWAYS,                // how to create
      FILE_ATTRIBUTE_NORMAL,        // file attributes
      0 );                          // handle to template file
   return ghDiag;
}

static HANDLE   _sOpenDiag(VOID)
{
   HANDLE h = CreateFile(
      gszDiag,                      // file name
      GENERIC_READ | GENERIC_WRITE, // access mode
      0,              // share mode
      0,                            // SD
      OPEN_EXISTING,                // how to create
      FILE_ATTRIBUTE_NORMAL,        // file attributes
      0 );                          // handle to template file
   return h;
}

VOID  CloseDiag(VOID)
{
   if( VFH(ghDiag) )
   {
      CloseHandle(ghDiag);
   }
   ghDiag = 0;
}

HANDLE   ghLog = 0;
DWORD    gdwTotLog;
DWORD    gdwLogCnt;

VOID  CloseLog( VOID )
{
   if( VFH(ghLog) )
      CloseHandle(ghLog);

   ghLog = 0;

   if(gdwTotLog)
   {
      sprtf( "Closed LOG with %d messages. (%d bytes)\r\n", gdwLogCnt, gdwTotLog );
   }
}

VOID  _write2Log( LPTSTR lpb )
{
   HANDLE   h = ghLog;
   DWORD dwi = strlen(lpb);
   if( dwi && VFH(h) )
   {
      DWORD dww;
      if(( WriteFile(h,lpb,dwi,&dww,0) ) &&
         ( dww == dwi ) )
      {
         // ok
         gdwTotLog += dwi; // add to LOG total
         gdwLogCnt++;
      }
      else
      {
         CloseLog();
         ghLog = INVALID_HANDLE_VALUE;
         sprtf( "WARNING: Write to LOG FAILED!\r\n" );

      }
   }
}

#define  MXLOGSTR    512
VOID  WriteLog( LPTSTR lpd )
{
   static TCHAR _s_prtbuf2[MXLOGSTR+16];
   LPTSTR      lpb = &_s_prtbuf2[0];
   int   i, j, k;
   TCHAR    c, d;
   k = 0;
   i = strlen(lpd); 
   if(i) //  > 0 ) && ( i < 1024 ) && VFH(ghLog) )
   {
      d = 0;
      for( j = 0; j < i; j++ )
      {
         c = lpd[j];
         if( c == '\r' )
         {
            if( ( j + 1 ) < i )
            {
               if( lpd[ j + 1 ] != '\n' )
               {
                  lpb[k++] = c;
                  c = '\n';
               }
            }
            else
            {
                  lpb[k++] = c;
                  c = '\n';
            }
         }
         else if( c == '\n' )
         {
            if( d != '\r' )
            {
               lpb[k++] = '\r';  // fill in the Cr
            }
         }
         lpb[k++] = c;
         d = c;
         if( k >= MXLOGSTR )
         {
            lpb[k] = 0;
            _write2Log(lpb);
            k = 0;
         }
      }

      if(k)
      {
         lpb[k] = 0;
         _write2Log(lpb);
      }
   }
}


HANDLE  CreateLog( LPTSTR lpf )
{
   HANDLE h = CreateFile(
      lpf,                      // file name
      GENERIC_READ | GENERIC_WRITE, // access mode
      0,              // share mode
      0,                            // SD
      CREATE_ALWAYS,                // how to create
      FILE_ATTRIBUTE_NORMAL,        // file attributes
      0 );                          // handle to template file

   gdwTotLog = 0;
   gdwLogCnt = 0;

   if( !VFH(h) )
      sprtf( "WARNING: Failed to OPEN %s log!\r\n", lpf );

   ghLog = h;

   return h;
}

VOID  CreateDiag( VOID )
{
   LPTSTR   lpf = &gszDiag[0];
#ifdef   _UsbUtil_H
   GetModulePath(lpf);
   strcat(lpf, gszDefDiag);
#else // !_UsbUtil_H
   LPTSTR   p;
   GetModuleFileName(NULL, lpf, 256);
   p = strrchr(lpf, '\\');
   if(p)
      p++;
   else
      p = lpf;
   *p = 0;
#ifndef  NDEBUG
   {
      DWORD   iPos;
      iPos = InStri(lpf, "DEBUG");
      if(( iPos ) &&
         ( iPos == (strlen(lpf) - 5) ) )
      {
         iPos--;
         p = &lpf[iPos];
         *p = 0;
      }
   }
#endif   // !NDEBUG
   strcpy(p, gszDefDiag);
#ifdef   USEMULTDIAG
   {
      HANDLE   h;
      DWORD    dwo = 5;

Try_Next:
      h = _sOpenDiag();
      if( VFH(h) )
      {
         CloseHandle(h);
      }
      else  // FAILED
      {
         DWORD dwe = GetLastError();
         //if( dwe == ERROR_FILE_NOT_FOUND )
         //{
            // ok, continue to create
         //}
         //else
         if( dwe == ERROR_SHARING_VIOLATION )   // ERROR_ACCESS_DENIED )
         {
            // this suggests another instance has this file
            DWORD dwi = strlen(lpf);
            TCHAR c = lpf[dwi - dwo];
            if( c < '9' )
            {
               c++;
               lpf[dwi - dwo] = c;
               goto Try_Next;
            }
            else
            {
               lpf[dwi - dwo] = '0';
               c = lpf[dwi - dwo - 1];
               if( c < '9' )
               {
                  c++;
                  lpf[dwi - dwo - 1] = c;
                  goto Try_Next;
               }
               else
               {
                  lpf[dwi - dwo - 1] = '0';
                  c = lpf[dwi - dwo - 2];
                  if( c < '9' )
                  {
                     c++;
                     lpf[dwi - dwo - 1] = c;
                     goto Try_Next;
                  }
               }
            }
         }
      }
   }
#endif   // USEMULTDIAG

#endif   // #ifdef   _UsbUtil_H
   _sCreatDiag();

}

void  oi( LPTSTR lpb )
{
   DWORD dwi, dww;

   if( dwi = strlen(lpb) )
   {
      if( ghDiag == 0 )
         CreateDiag();

      if( VFH(ghDiag) )
      {
         if( ( WriteFile(ghDiag,lpb,dwi,&dww,0) ) &&
             ( dwi == dww ) )
         {
            // happy
         }
         else
         {
            CloseDiag();
            ghDiag = INVALID_HANDLE_VALUE;
         }
      }
   }
}

int   prt( LPTSTR lpd )
{
   static TCHAR _s_prtbuf[1024];
   LPTSTR      lpb = &_s_prtbuf[0];
   int   i, j, k;
   TCHAR    c, d;
   k = 0;
   if( i = strlen(lpd) )
   {
      d = 0;
      for( j = 0; j < i; j++ )
      {
         c = lpd[j];
         if( c == '\r' )
         {
            if( ( j + 1 ) < i )
            {
               if( lpd[ j + 1 ] != '\n' )
               {
                  lpb[k++] = c;
                  c = '\n';
               }
            }
            else
            {
                  lpb[k++] = c;
                  c = '\n';
            }
         }
         else if( c == '\n' )
         {
            if( d != '\r' )
            {
               lpb[k++] = '\r';  // fill in the Cr
            }
         }
         lpb[k++] = c;
         d = c;
      }
      if(k)
      {
         lpb[k] = 0;
         oi(lpb);
      }
   }
   return k;
}


#ifdef  USB_DEBUG
/* =====================================================
 * int _cdecl sprt( LPTSTR lpf, ... )
 *
 */
int _cdecl sprtf( LPTSTR lpf, ... )
{
   static TCHAR _s_sprtfbuf[1024];
   LPTSTR      lpb = &_s_sprtfbuf[0];
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   return (prt(lpb));
}

// ======================================================================
#endif   // !USB_NDEBUG

// something that needs to be checked- maybe CRITICAL
//
int  _cdecl chkme( LPTSTR lpf, ... )
{
   static TCHAR _s_chkmebuf[1024];
   LPTSTR      lpb = &_s_chkmebuf[0];
   int         i;
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   i = vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   sprtf(lpb);

   i = lstrlen(lpb);
   if( ( i > 2         ) &&
       ( lpb[0] == 'C' ) &&
       ( ( lpb[1] == ':' ) || ( lpb[1] == 'R' ) ) )
   {
      //i = MessageBox(ghMainWnd,
      i = MessageBox(NULL,
         lpb,
         "CRITICAL ERROR: CHECK ME",
         (MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_SYSTEMMODAL) );
      if( i == IDABORT )
      {
         //DestroyWindow(ghMainWnd);
         exit(-1);
      }
   }
   return i;
}


// eof - UsbLog.c

// ***************************

LPTSTR   GetNxtBuf( VOID )
{
   // (MXLINEB * MXLINES)
static INT	s_iLnBuf = 0;
static TCHAR	s_szLnBuf[ MXLINES * MXLINEB2 ];
   INT   i = s_iLnBuf;      //     GW.ws_iLnBuf
   i++;     // cycle of buffers 
   if( i >= MXLINES )
      i = 0;
   s_iLnBuf = i;
   // [1024] byte chunks ...
   return( &s_szLnBuf[ (MXLINEB2 * i) ] );    // GW.ws_szLnBuf
}

DWORD TrimIB( LPTSTR lps )
{
   DWORD    dwr = strlen(lps);
   LPTSTR   p   = lps;
   DWORD    dwk = 0;
   while(dwr)
   {
      if( *p > ' ' )
         break;
      dwk++;
      p++;
      dwr--;   // update return length
   }
   if(dwk)
      strcpy(lps,p);    // copy remainder up to beginning
   dwk = dwr;
   while(dwk--)
   {
      if( lps[dwk] > ' ' )
         break;
      lps[dwk] = 0;
      dwr--;
   }
   return dwr;
}
   
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : InStr
// Return type: INT 
// Arguments  : LPTSTR lpb
//            : LPTSTR lps
// Description: Return the position of the FIRST instance of the string in lps
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
INT   InStr( LPTSTR lpb, LPTSTR lps )
{
   INT   iRet = 0;
   INT   i, j, k, l, m;
   TCHAR    c;
   i = strlen(lpb);
   j = strlen(lps);
   if( i && j && ( i >= j ) )
   {
      c = *lps;   // get the first we are looking for
      l = i - ( j - 1 );   // get the maximum length to search
      for( k = 0; k < l; k++ )
      {
         if( lpb[k] == c )
         {
            // found the FIRST char so check until end of compare string
            for( m = 1; m < j; m++ )
            {
               if( lpb[k+m] != lps[m] )   // on first NOT equal
                  break;   // out of here
            }
            if( m == j )   // if we reached the end of the search string
            {
               iRet = k + 1;  // return NUMERIC position (that is LOGICAL + 1)
               break;   // and out of the outer search loop
            }
         }
      }  // for the search length
   }
   return iRet;
}
INT   InStri( LPTSTR lpb, LPTSTR lps )
{
   INT   iRet = 0;
   INT   i, j, k, l, m;
   INT   c;
   i = strlen(lpb);
   j = strlen(lps);
   if( i && j && ( i >= j ) )
   {
      c = toupper(*lps);   // get the first we are looking for
      l = i - ( j - 1 );   // get the maximum length to search
      for( k = 0; k < l; k++ )
      {
         if( toupper(lpb[k]) == c )
         {
            // found the FIRST char so check until end of compare string
            for( m = 1; m < j; m++ )
            {
               if( toupper(lpb[k+m]) != toupper(lps[m]) )   // on first NOT equal
                  break;   // out of here
            }
            if( m == j )   // if we reached the end of the search string
            {
               iRet = k + 1;  // return NUMERIC position (that is LOGICAL + 1)
               break;   // and out of the outer search loop
            }
         }
      }  // for the search length
   }
   return iRet;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Mid
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl - Pointer to line
//            : DWORD dwb  - Begin here
//            : DWORD dwl  - for this length
// Description: Returns a buffer containing the MIDDLE portion of a string.
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Mid( LPTSTR lpl, DWORD dwb, DWORD dwl )
{
   LPTSTR   lps = GetNxtBuf();
//   LPTSTR   pt;
   DWORD    dwk = strlen(lpl);
   DWORD    dwi, dwr;
   *lps = 0;
   if( ( dwl ) && 
      ( dwb ) &&
      ( dwl ) &&
      ( dwb <= dwk ) &&
      ( dwl <= (dwk - (dwb - 1)) ) )
   {
      dwr = 0;
      for(dwi = (dwb - 1); (dwi < dwk), (dwr < dwl); dwi++ )
      {
//         pt = &lpl[dwi];
         lps[dwr++] = lpl[dwi];
      }
      lps[dwr] = 0;
   }
   return lps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Left
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl
//            : DWORD dwi
// Description: Return the LEFT prortion of a string
//              Emulates the Visual Basic function
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Left( LPTSTR lpl, DWORD dwi )
{
   LPTSTR   lps = GetNxtBuf();
   DWORD    dwk;
   for( dwk = 0; dwk < dwi; dwk++ )
      lps[dwk] = lpl[dwk];
   lps[dwk] = 0;
   return lps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Right
// Return type: LPTSTR 
// Arguments  : LPTSTR lpl
//            : DWORD dwl
// Description: Returns a buffer containing the RIGHT postion of a string
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Right( LPTSTR lpl, DWORD dwl )
{
   LPTSTR   lps = GetNxtBuf();
   DWORD    dwk = strlen(lpl);
   DWORD    dwi;
   *lps = 0;
   if( ( dwl ) &&
      ( dwk ) &&
      ( dwl <= dwk ) )
   {
      if( dwl == dwk )  // is it right ALL
         dwi = 0;
      else
         dwi = dwk - dwl;
      strcpy(lps, &lpl[dwi] );
   }

   return lps;
}


//typedef struct _SYSTEMTIME { 
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
//} SYSTEMTIME, *PSYSTEMTIME; 
//Members
//wYear 
//Specifies the current year. 
//wMonth 
//Specifies the current month; January = 1, February = 2, and so on. 
//wDayOfWeek 
//Specifies the current day of the week; Sunday = 0, Monday = 1, and so on. 
//wDay 
//Specifies the current day of the month. 
//wHour 
//Specifies the current hour. 
//wMinute 
//Specifies the current minute. 
//wSecond 
//Specifies the current second. 
//wMilliseconds 
//Specifies the current millisecond.
// ADDCVSDATE - Get the latest DATE
// ctime( &ltime ) = Fri Apr 29 12:25:12 1994

LPTSTR   pDays[] = {
   { "Sun" },
   { "Mon" },
   { "Tue" },
   { "Wed" },
   { "Thu" },
   { "Fri" },
   { "Sat" }
};

LPTSTR   pMths[] = {
   { "Jan" },
   { "Feb" },
   { "Mar" },
   { "Apr" },
   { "May" },
   { "Jun" },
   { "Jul" },
   { "Aug" },
   { "Sep" },
   { "Oct" },
   { "Nov" },
   { "Dec" }
};

#define  EATSPACE(p)    while( *p && (*p <= ' '))p++
#define  EATCHARS(p)    while( *p && (*p >  ' '))p++

BOOL  strbgn( LPTSTR lps, LPTSTR lpd )
{
   BOOL  bRet = FALSE;
   DWORD i = strlen(lpd);
   if( i <= strlen(lps) )
   {
      DWORD   j;
      for( j = 0; j < i; j++ )
      {
         if( lps[j] != lpd[j] )
            return FALSE;
      }
      bRet = TRUE;
   }
   return bRet;
}

BOOL  Stg2SysTm( LPTSTR lps, SYSTEMTIME * pt )
{
   LPTSTR   lpd;
   INT      i;

   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;

   for( i = 0; i < 7; i++ )
   {
      lpd = pDays[i];
      if( strbgn( lps, lpd ) )
         break;
   }
   if( i == 7 )
      return FALSE;

   pt->wDayOfWeek = (WORD)i;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;

   for( i = 0; i < 12; i++ )
   {
      lpd = pMths[i];
      if( strbgn( lps, lpd ) )
         break;
   }
   if( i == 12 )
      return FALSE;

   pt->wMonth = (WORD)(i + 1);

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   pt->wDay = (WORD) atoi(lps);

   if( ( pt->wDay < 1 ) || ( pt->wDay > 31 ) )
      return FALSE;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   if(( strlen(lps) < 8 ) ||   // 12:45:78
      ( lps[2] != ':'   ) ||
      ( lps[5] != ':'   ) )
      return FALSE;

   pt->wHour   = (WORD)atoi(lps);
   pt->wMinute = (WORD) atoi( &lps[3] );
   pt->wSecond = (WORD) atoi( &lps[6] );
   pt->wMilliseconds = 0;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   pt->wYear = (WORD)atoi(lps);

   return TRUE;
}

// added April, 2001
VOID  AppendDateTime( LPTSTR lpb, LPSYSTEMTIME pst )
{
   sprintf(EndBuf(lpb),
      "%02d/%02d/%02d %02d:%02d",
      (pst->wDay & 0xffff),
      (pst->wMonth & 0xffff),
      (pst->wYear % 100),
      (pst->wHour & 0xffff),
      (pst->wMinute & 0xffff) );
}

LPTSTR   GetDT5Stg( VOID )
{
   LPTSTR   lps = GetNxtBuf();
   SYSTEMTIME  st;
   *lps = 0;
   GetLocalTime(&st);
   AppendDateTime(lps,&st);
   return lps;
}

VOID  AppendTime( LPTSTR lpb, LPSYSTEMTIME pst )
{
   sprintf(EndBuf(lpb),
      "%02d:%02d:%02d",
      (pst->wHour & 0xffff),
      (pst->wMinute & 0xffff),
      (pst->wSecond & 0xffff) );
}

LPTSTR   GetTmStg( VOID )
{
   LPTSTR   lps = GetNxtBuf();
   SYSTEMTIME  st;
   *lps = 0;

   // GetSystemTime( &st ) if UTC / GMT required / desired
   GetLocalTime(&st);
   AppendTime(lps,&st);

   return lps;
}

//void  prt(LPTSTR lpb);   // to log file output

int   _cdecl sprtf( LPTSTR lpf, ... )
{
   static TCHAR _s_sprtfbuf[1024];
   LPTSTR   lpb = &_s_sprtfbuf[0];
   int   i;
   va_list arglist;
   va_start(arglist, lpf);
   i = vsprintf( lpb, lpf, arglist );
   va_end(arglist);
   prt(lpb);
   return i;
}

LPTSTR   DSecs2YDHMSStg( double db )
{
   LPTSTR   lps = GetNxtBuf();
   double   dsind = (60*60*24);
   double   dsiny = (dsind * 365);

   *lps = 0;
   if( db >= dsiny )
   {
      int yrs = (int)(db / dsiny);
      db = (db - (dsiny * (double)yrs));
      if(yrs > 1)
         sprintf(lps, "%d years ", yrs);
      else if(yrs == 1)
         strcpy(lps, "1 year ");
      else
         sprintf(lps, "%d year(s) ", yrs);
   }
   if( db >= dsind )
   {
      int dys = (int)(db / dsind);
      db = (db - (dsind * (double)dys));
      if(dys > 1)
         sprintf(EndBuf(lps), "%d days ", dys);
      else if(dys == 1)
         strcat(lps, "1 day ");
      else
         sprintf(EndBuf(lps), "%d day(s) ", dys);
   }

   if( db > 0 )
   {
      int   hrs = (int)( db / (60*60));
      db = ( db - (60*60*hrs) );
      sprintf(EndBuf(lps), "%02d:", hrs);
      if( db > 0 )
      {
         int mins = (int)( db / 60 );
         int secs;

         db = ( db - (60 * mins) );
         secs = (int)db;
         sprintf(EndBuf(lps), "%02d:%02d", mins, secs);
      }
      else
      {
         strcat(lps, "00:00");
      }
   }
   else
   {
      strcat(lps, "00:00:00");
   }

   return lps;
}

BOOL  Chk4Debug( LPTSTR lpd )
{
   BOOL     bret = FALSE;
   TCHAR    sztmp[264];
//   LPTSTR ptmp = &gszTmpBuf[0];
//   LPTSTR   lpb = &_s_sprtfbuf[0];
//   LPTSTR   ptmp = &_s_sprtfbuf[0];
   LPTSTR   ptmp = &sztmp[0];

   LPTSTR   p;
   DWORD  dwi;

   strcpy(ptmp, lpd);   // copy input
   dwi = strlen(ptmp);  // and length
   if(dwi)
   {
      dwi--;
      if(ptmp[dwi] == '\\')
      {
         ptmp[dwi] = 0;
         p = strrchr(ptmp, '\\');
         if(p)
         {
            p++;
            if( strcmpi(p, "DEBUG") == 0 )
            {
               *p = 0;
               strcpy(lpd,ptmp);    // use this
               bret = TRUE;
            }
         }
      }
   }
   return bret;
}

VOID  GetModulePath( LPTSTR lpb )
{
   LPTSTR   p;
   GetModuleFileName( NULL, lpb, 256 );
   p = strrchr( lpb, '\\' );
   if( p )
      p++;
   else
      p = lpb;
   *p = 0;
#ifndef  NDEBUG
   Chk4Debug( lpb );
#endif   // !NDEBUG

}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : IsValidFile4
// Return type: DWORD 
// Arguments  : LPTSTR lpf
//            : PWIN32_FIND_DATA pfd
// Description: Check if this is a FILE, FOLDER (directory) or WILD CARD
//              file name, and return the appropriate value.
// RETURN values
//#define  IS_FILE     1
//#define  IS_FOLDER   2
//#define  IS_WILD     4
///////////////////////////////////////////////////////////////////////////////
DWORD  IsValidFile4( LPTSTR lpf, PWIN32_FIND_DATA pfd )
{
   DWORD     flg = 0;
   HANDLE   hFind;
   WIN32_FIND_DATA   fd;
   hFind = FindFirstFile( lpf, pfd );
   if( VFH(hFind) )
   {
      if( pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
         flg = IS_FOLDER;
      else
      {
         if( FindNextFile( hFind, &fd ) )  // search handle and data buffer
            flg = IS_WILD;
         else
            flg = IS_FILE;
      }
      FindClose(hFind);
   }
   return flg;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : IsValidFile
// Return type: DWORD 
// Arguments  : LPTSTR lpf
// Description: Check if this is a FILE, FOLDER (directory) or WILD CARD
//              file name, and return the appropriate value.
// RETURN values
//#define  IS_FILE     1
///////////////////////////////////////////////////////////////////////////////
DWORD  IsValidFile( LPTSTR lpf )
{
   WIN32_FIND_DATA   fd;
   if( IsValidFile4( lpf, &fd ) == IS_FILE )
      return 1;
   else
      return 0;
}


//
//  FUNCTION: CenterWindow(HWND, HWND)
//
//  PURPOSE:  Center one window over another.
//
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//
//  RETURN VALUE:
//
//    TRUE  - Success
//    FALSE - Failure
//
//  COMMENTS:
//
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
//

BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the Height and Width of the parent window
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

    // Get the display limits
    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
         xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
}

// some timing functions
// ===============================================================================
#define  MXTMS    32    // NOTE this MAXIMUM at any one time, else unpredictable

typedef  struct { /* tm */
   BOOL  tm_bInUse;
   BOOL  tm_bt;
   LARGE_INTEGER  tm_lif, tm_lib, tm_lie;
   DWORD tm_dwtc;
}GTM, * PGTM;

GTM   sGtm[MXTMS] = { 0 }; // init to ZERO
int   iNxt = 0;

int  GetMTime( void )
{
   return MXTMS;
}
int  GetCTime( void )
{
   return iNxt;
}

VOID  InitTimers( VOID )
{
   int   i;
   PGTM  ptm = &sGtm[0];

   for( i = 0; i < MXTMS; i++ )
   {
      ptm->tm_bInUse = FALSE;
      ptm++;
   }
}

int  GetTimer( PGTM * pptm )
{
   PGTM  ptm = &sGtm[0];
   int   i = (int)-1;
   int   j;
   for( j = 0; j < MXTMS; j++ )
   {
      if( !ptm->tm_bInUse )
      {
         ptm->tm_bInUse = TRUE;
         i = j;
         *pptm = ptm;
         break;
      }
      ptm++;
   }
   return i;
}

/* =================================
 * int  GetBTime(void) - was SetBTime( void ) for a while
 *
 * Purpose: Set the beginning timer, and return the INDEX of that timer
 *
 * Return: Index (offset) of timer SET
 *
 */
int  GetBTime( void )
{
   PGTM  ptm;
   int   i;
   i = GetTimer( &ptm );
   if( i != (int)-1 )
   {
      ptm->tm_bt = QueryPerformanceFrequency( &ptm->tm_lif ); 
      if(ptm->tm_bt)
         QueryPerformanceCounter( &ptm->tm_lib ); // counter value
      else
         ptm->tm_dwtc = GetTickCount(); // ms since system started
   }
   return i;
}

/* =================================
 * double GetETime( int i )
 *
 * Purpose: Return ELAPSED time as double (in seconds) of the index given
 *
 * Return: If index is with the range of 0 to (MXTMS - 1) then
 *          compute ELAPSED time as a double in SECONDS
 *
 *         Else results indeterminate
 *
 */
double GetETime( int i )
{
   DWORD          dwd;
   double         db;
   LARGE_INTEGER  lid;
   PGTM           ptm;
   if( ( i < 0     ) ||
       ( i >= MXTMS ) )
   {
      db = (double)100.0;  // return an idiot number!!!
   }
   else
   {
      ptm = &sGtm[i];
      if( ptm->tm_bt )
      {
         QueryPerformanceCounter( &ptm->tm_lie ); // counter value
         lid.QuadPart = ( ptm->tm_lie.QuadPart - ptm->tm_lib.QuadPart ); // get difference
         db  = (double)lid.QuadPart / (double)ptm->tm_lif.QuadPart;
      }
      else
      {
         dwd = (GetTickCount() - ptm->tm_dwtc);   // ms elapsed
         db = ((double)dwd / 1000.0);
      }
      // make this timer available
      ptm->tm_bInUse = FALSE;
   }

   return db;

}

// get a snapshot, time in (double) secs since start
// **************
double GetRTime( int i )
{
   DWORD          dwd;
   double         db;
   LARGE_INTEGER  lid;
   PGTM           ptm;
   if( ( i < 0     ) ||
       ( i >= MXTMS ) )
   {
      db = (double)-1;  // return an idiot number!!!
   }
   else
   {
      ptm = &sGtm[i];
      if( ptm->tm_bt )
      {
         QueryPerformanceCounter( &ptm->tm_lie ); // counter value
         lid.QuadPart = ( ptm->tm_lie.QuadPart - ptm->tm_lib.QuadPart ); // get difference
         db  = (double)lid.QuadPart / (double)ptm->tm_lif.QuadPart;
      }
      else
      {
         dwd = (GetTickCount() - ptm->tm_dwtc);   // ms elapsed
         db = ((double)dwd / 1000.0);
      }
      // make this timer available
      // ptm->tm_bInUse = FALSE;, but this is ONLY a read function
      // get running time ... so if NOT freed.

   }

   return db;

}


LPTSTR Rect2Stg( PRECT lpr )
{
   LPTSTR lps = GetNxtBuf();
   *lps = 0;
   if(lpr)
   {
      sprintf( lps,
         "(%d,%d,%d,%d)",
         lpr->left,
         lpr->top,
         lpr->right,
         lpr->bottom );
   }
   return lps;
}

// EXTRACT from UTILS\grmLib.c
/* Oct 99 update - retreived from DDBData.c */
// Oct 2001 update - name Buffer2Stg changed to
// ===========================================================
// void Buffer2Str( LPTSTR lps, LPTSTR lpb, int decimal,
//				 int sign, int precision )
//
// Purpose: Convert the string of digits from the _ecvt
//			function to a nice human readable form.
//
// 1999 Sept 7 - Case of removing ?.?0000 the zeros
// 2001 Oct    - changed name (g to r) - grm
// ===========================================================
void Buffer2Str( LPTSTR lps, LPTSTR lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;

	k = 0;					// Start at output beginning
	cad = 0;				// Count AFTER the decimal
	j = strlen( lpb );		// Get LENGTH of buffer digits

	if( sign )				// if the SIGN flag is ON
		lps[k++] = '-';		// Fill in the negative

	l = decimal;
	if( l < 0 )
	{
		// A NEGATIVE decimal position
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		// Possible shortened use of the digit string
		// ie possible LOSS of DIGITS to fit the precision requested.
		if( decimal == 0 )
		{
			if( ( precision - 1 ) < j )
			{
				//chkme();
				j = precision - 1;
			}
		}
		else
		{
			if( precision < j )
			{
//				chkme();
				j = precision;
			}
		}
	}

	sig = 0;	// Significant character counter
	// Process each digit of the digit list in the buffer
	// or LESS than the list if precision is LESS!!!
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];		// Get a DIGIT
		if( i == decimal )	// Have we reached the DECIMAL POINT?
		{
			// At the DECIMAL point
			if( i == 0 )	
			{
				// if no other digits BEFORE the decimal
				lps[k++] = '0';	// then plonk in a zero now
			}
			lps[k++] = '.';	// and ADD the decimal point
			cad++;
		}
		// Check for adding a comma for the THOUSANDS
		if( ( decimal > 0 ) &&
			( sig ) &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';	// Add in a comma
		}
		lps[k++] = c;	// Add this digit to the output
		if( sig )		// If we have HAD a significant char
		{
			sig++;		// Then just count another, and another etc
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}	// while processing the digit list

	// FIX980509 - If digit length is LESS than decimal position
	// =========================================================
	if( ( decimal > 0 ) &&
		( i < decimal ) )
	{
		c = '0';
		while( i < decimal )
		{
			if( ( decimal > 0 ) &&
				( sig ) &&
				( i < decimal ) )
			{
				m = decimal - i;
				if( (m % 3) == 0 )
					lps[k++] = ',';	// Add in a comma
			}
			lps[k++] = c;	// Add this digit to the output
			i++;
		}
	}
	// =========================================================
	if( cad )
		cad--;
	lps[k] = 0;		// zero terminate the output
	// FIX990907 - Remove unsightly ZEROs after decimal point
    for( i = 0; i < k; i++ )
    {
        if( lps[i] == '.' )
            break;
    }
    if( ( i < k ) &&
        ( lps[i] == '.' ) )
    {
        i++;
        if( lps[i] == '0' )
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
            if( k > i )
            {
                // we have backed to a not '0' value so STOP
            }
            else
            {
                // we backed all the way, so remove the DECIMAL also
                i--;
                lps[i] = 0;
            }
        }
        else
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
        }
    }

}




///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Dbl2Stg
// Return type: LPTSTR 
// Arguments  : double value
//            : int prec
// Description: Convert double to good form
//              prec defaults to 16 if none given
// 2001 Oct - was called Dbl2Str in many UNTILs, but changed to - grm
///////////////////////////////////////////////////////////////////////////////
LPTSTR   Dbl2Stg( double value, int prec )
{
    int     decimal, sign, precision;
    char *  buffer;
    LPTSTR  lps = GetNxtBuf();

    if( prec )
        precision = prec;
    else
        precision = 16;

    buffer = _ecvt( value, precision, &decimal, &sign );

    // of course, as part change the sub-call to tidy up
    Buffer2Str( lps, buffer, decimal, sign, precision );

    return lps;

}

LPTSTR   GetTitleStg( LPTSTR lpf )
{
    LPTSTR  lps = GetNxtBuf();
    LPTSTR  p;
    p = strrchr(lpf, '\\');
    if(p)
    {
       p++;
       strcpy(lps,p);
    }
    else
    {
       if((strlen(lpf) > 2 ) &&
          (lpf[1] == ':'   ) )
       {
          // if a DRIVE
          strcpy(lps, &lpf[2]);
       }
       else
       {
          strcpy(lps,lpf);
       }
    }
    return lps;
}

// eof - Fa4Util.c
// eof - Utilgrm.c
