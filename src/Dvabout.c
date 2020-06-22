
/* ==============================================
 *
 *      File:  DVABOUT.C
 *
 *   Purpose:  Contains the about box's window procedure.
 *
 * Functions:  ABOUTDLG
 *
 *  Comments:  Nothing really special here...Put in a separate module
 *             so it doesn't eat memory when no about box is around.
 *
 *   History:   Date     Reason
 *             6/1/91    Created
 *             6 Mch 96  Added Date and Version.
 *				Aug 96	32-Bit version
 *				Nov 97	Single VER_DATE is here.
 *
 * ============================================= */
#include "dv.h"

extern   int	DVGetCwd( LPSTR lpb, DWORD siz );   // service in DvUtil.c
//extern   char	szIniFile[];   // [260] INI file name in DvIni.c
extern   TCHAR    gszMLib1[]; // = "...load of WJPEG32.DLL";
extern   TCHAR    gszMLib2[]; // = "...load of WJPG32_2.DLL";
// = { DEF_LIB2_NAME }; // 32-bit = "WJPG32_2.DLL"
extern   char     szJpgLib2[];

BOOL DisplaySystemVersion( LPTSTR lps );
//#define	MXTBUF		128
// Only have ONE of these
char	szVerDate[] = VER_DATE;
char	*pREnh = "Enhanced";
char	*pRStand = "Standard";
// ==== set date and time of compile ===
TCHAR gszCompDate[] = __DATE__;
TCHAR gszCompTime[] = __TIME__;
// =====================================

SYSTEM_INFO     gsi;
MEMORYSTATUS   gms;

extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);


// Use to change the divisor from Kb to Mb.

#define DIV 1024
// #define DIV 1

char * divisor = "K";
// char *divisor = "";

// Handle the width of the field in which to print numbers this way to
// make changes easier. The asterisk in the print format specifier
// "%*ld" takes an int from the argument list, and uses it to pad and
// right-justify the number being formatted.
#define WIDTH 7

BOOL	InitAbout( HWND hDlg )
{
	BOOL	flg;
	static char	buf1[ (MXTBUF * 2) ];
	static char	buf2[ (MXTBUF * 2) ];
	LPSTR	lps1, lps2;
	int	i;
	HWND	hOwnr;
	DWORD ddVersion;
	DWORD ddMem;
#ifndef	WIN32
	LPSTR	lpt;
	DWORD ddFlags;
	UINT	uFree;
#endif	// !WIN32

   lps1 = &buf1[0];
	lps2 = &buf2[0];
   GetSystemInfo( &gsi );
   gms.dwLength = 0; // check if the function fills in the value
   GlobalMemoryStatus( &gms );   // get dynamic memory status 

	hOwnr = GetWindow( hDlg, GW_OWNER ); 
	if(hOwnr)
		CenterWindow( hDlg, hOwnr );

	i = GetDlgItemText( hDlg, IDD_DATE, lps1, MXTBUF ); 
	if(i)
	{
		sprintf( lps2, lps1, (LPSTR) &szVerDate[0] );
		SetDlgItemText( hDlg, IDD_DATE, lps2 );
	}


	i = GetDlgItemText( hDlg, IDD_VERSION, lps1, MXTBUF ); 
	if(i)
	{
		sprintf( lps2, lps1, VER_MAJOR, VER_MINOR, VER_MINOR2, VER_BUILD );
		SetDlgItemText( hDlg, IDD_VERSION, lps2 );
	}

	i = GetDlgItemText( hDlg, IDD_WINVER, lps1, MXTBUF ); 
	if(i)
	{
		ddVersion = GetVersion();
		sprintf( lps2, lps1, LOBYTE(LOWORD(ddVersion)),
		    HIBYTE(LOWORD(ddVersion)));
		SetDlgItemText( hDlg, IDD_WINVER, lps2 );
	}

#ifdef	WIN32

   sprintf( lps2, "Compiled on %s at %s.", gszCompDate, gszCompTime );
	SetDlgItemText( hDlg, IDD_RESOURCE, lps2 );

	lps2[0] = 0;
	SetDlgItemText( hDlg, IDD_MODE, lps2 );
	//SetDlgItemText( hDlg, IDD_GDIRESOURCE, lps2 );
	//SetDlgItemText( hDlg, IDD_USERRESOURCE, lps2 );
	SetDlgItemText( hDlg, IDD_GDIRESOURCE,  gszMLib1 );
	SetDlgItemText( hDlg, IDD_USERRESOURCE, gszMLib2 );
   ddMem   = MXTBUF;
   if( GetComputerName( lps1, // computer name
      &ddMem ) ) // size of name buffer
   {
      sprintf(lps2,"Computer: %s",lps1);
      ddMem   = MXTBUF;
      if( GetUserName(lps1,&ddMem) )
      {
         sprintf(EndBuf(lps2)," User: %s",lps1);
      }
   	SetDlgItemText( hDlg, IDD_MODE, lps2 );
   }
//  printf ("%ld percent of memory is in use.\n",
//          stat.dwMemoryLoad);
//  printf ("There are %*ld total %sbytes of physical memory.\n",
//          WIDTH, stat.dwTotalPhys/DIV, divisor);
//  printf ("There are %*ld free %sbytes of physical memory.\n",
//          WIDTH, stat.dwAvailPhys/DIV, divisor);
   if( gms.dwLength )
   {
      sprintf(lps2,
         "Memory: %ld %sB Total. %ld %sB (%d%%) Free.",
         (long)(gms.dwTotalPhys / DIV), divisor,
         (long)(gms.dwAvailPhys / DIV), divisor,
         (int)(gms.dwMemoryLoad - 100) );
	   SetDlgItemText( hDlg, IDD_MEMORY, lps2 );
   }
   else
   {
   	i = GetDlgItemText( hDlg, IDD_MEMORY, lps1, MXTBUF ); 
   	if(i)
	   {
		   ddMem = GetFreeSpace( 0 ) / 1024L;
		   sprintf( lps2, lps1, ddMem );
		   SetDlgItemText( hDlg, IDD_MEMORY, lps2 );
	   }
   }

#else	// !WIN32

	i = GetDlgItemText( hDlg, IDD_MODE, lps1, MXTBUF ); 
	if(i)
	{
		ddFlags = GetWinFlags();
		if( ddFlags & WF_ENHANCED )
			lpt = pREnh;
		else
			lpt = pRStand;
		sprintf( lps2, lps1, lpt );
		SetDlgItemText( hDlg, IDD_MODE, lps2 );
	}


#endif	// WIN32 y/n

#ifndef	WIN32
	i = GetDlgItemText( hDlg, IDD_RESOURCE, lps1, MXTBUF ); 
	if(i)
	{
		uFree = GetFreeSystemResources( GFSR_SYSTEMRESOURCES );
		sprintf( lps2, lps1, uFree );
		SetDlgItemText( hDlg, IDD_RESOURCE, lps2 );
	}

	i = GetDlgItemText( hDlg, IDD_GDIRESOURCE, lps1, MXTBUF ); 
	if(i)
	{
//               GFSR_GDIRESOURCES     Returns the percentage of free space
//                                     for GDI resources. GDI resources
//                                     include device-context handles,
//                                     brushes, pens, regions, fonts, and
//                                     bitmaps.
		uFree = GetFreeSystemResources( GFSR_GDIRESOURCES );
		sprintf( lps2, lps1, uFree );
		SetDlgItemText( hDlg, IDD_GDIRESOURCE, lps2 );
	}

	i = GetDlgItemText( hDlg, IDD_USERRESOURCE, lps1, MXTBUF ); 
	if(i)
	{
//               GFSR_USERRESOURCES    Returns the percentage of free space
//                                     for USER resources. These resources
//                                     include window and menu handles.
		uFree = GetFreeSystemResources( GFSR_USERRESOURCES );
		sprintf( lps2, lps1, uFree );
		SetDlgItemText( hDlg, IDD_USERRESOURCE, lps2 );
	}
#endif	// !WIN32

   // added F20000825 - Show RUNTIME MODULE and CURRENT WORK DIRECTORY
   strcpy(lps1,"EXE:");
   if( GetModuleFileName( ghDvInst, EndBuf(lps1), MXTBUF ) )
      strcat(lps1,MEOR);
   strcat(lps1,"CWD:");
   DVGetCwd(EndBuf(lps1),MXTBUF);
   if( gszIniFile[0] )
   {
      strcat(lps1,MEOR"INI:");
      strcat(lps1,gszIniFile);
   }
   if( DisplaySystemVersion( lps2 ) )
   {
      strcat(lps1,MEOR"OS:");
      strcat(lps1,lps2);
   }
   SetDlgItemText( hDlg, IDC_EDIT1, lps1 );

	flg = TRUE;

   return( flg );

}

/* ======================================================================
 *
 * Function:   ABOUTDLG
 *
 * Purpose:    About Dialog box message handler.  Does nothing special,
 *             except close down when needed.
 *             From menu item IDM_ABOUT using "ABOUTDLG" template
 *
 * Parms:      hDlg    == Handle to About dialog box.
 *             message == Message to handle.
 *             wParam  == Depends on message.
 *             lParam  == Depends on message.
 *
 * History:   Date      Reason
 *             6/01/91  Created
 *             
 * ==================================================================== */
//BOOL MLIBCONV ABOUTDLG(HWND hDlg, 
//                     unsigned message, 
//                         WORD wParam, 
//                         LONG lParam)

INT_PTR CALLBACK ABOUTDLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   switch (message)
   {
      case WM_INITDIALOG:
         return( InitAbout( hDlg ) );

      case WM_COMMAND:
         {
            DWORD cmd = LOWORD(wParam);
            if( ( cmd == IDD_OK   ) ||   // this IS the button
                ( cmd == IDOK     ) ||       // "OK" box selected?        
                ( cmd == IDCANCEL ) )     // System menu close command?
            {
               EndDialog(hDlg, TRUE);     // Exits the dialog box 
               return (TRUE);
            }
         }
         break;
    }
   return( FALSE );                    // Didn't process a message
}

#ifdef	WIN32

long	Do_IDM_ABOUT( HWND hWnd )
{
	long	lRet = 1;

	DialogBox( ghDvInst,             // current instance
		szAboutDLG,        // resource to use 
		hWnd,              // parent handle
		ABOUTDLG );        // About() instance address

	return lRet;
}

#else	/* !WIN32 */

long	Do_IDM_ABOUT( HWND hWnd )
{
	long	lRet = 1;
	int		iRet;
	FARPROC lpDlgProc;

	lpDlgProc = MakeProcInstance(ABOUTDLG, ghDvInst);


	iRet = DialogBox( ghDvInst,             // current instance
		szAboutDLG,        // resource to use 
		hWnd,              // parent handle
		lpDlgProc );        // About() instance address

	FreeProcInstance(lpDlgProc);

	return lRet;
}

#endif	/* !WIN32 */



BOOL DisplaySystemVersion( LPTSTR lps )
{
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
   // which is supported on Windows 2000.
   //
   // If that fails, try using the OSVERSIONINFO structure.
   if( !lps )
      return FALSE;

   *lps = 0;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return FALSE;
   }

   switch (osvi.dwPlatformId)
   {
      case VER_PLATFORM_WIN32_NT:
      // Test for the product.
         if ( osvi.dwMajorVersion <= 4 )
            wsprintf( lps, "MS Windows NT ");

         if ( osvi.dwMajorVersion == 5 )
            wsprintf( lps, "MS Windows 2000 ");

      // Test for workstation versus server.
#ifdef _WIN98_
         if( bOsVersionInfoEx )
         {
            if ( osvi.wProductType == VER_NT_WORKSTATION )
               wsprintf(EndBuf(lps), "Professional " );

            if ( osvi.wProductType == VER_NT_SERVER )
               wsprintf(EndBuf(lps), "Server " );
         }
         else
#endif // #ifdef _WIN98_
         {
            HKEY hKey;
            char szProductType[80];
            DWORD dwBufLen;

            RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
               0, KEY_QUERY_VALUE, &hKey );
            RegQueryValueEx( hKey, "ProductType", NULL, NULL,
               (LPBYTE) szProductType, &dwBufLen);
            RegCloseKey( hKey );
            if ( lstrcmpi( "WINNT", szProductType) == 0 )
               wsprintf( EndBuf(lps), "Workstation " );
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
               wsprintf( EndBuf(lps), "Server " );
         }

         break;

      case VER_PLATFORM_WIN32_WINDOWS:

         if ((osvi.dwMajorVersion > 4) || 
            ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0)))
         {
             wsprintf(lps, "MS Windows 98 ");
         } 
         else wsprintf(lps, "MS Windows 95 ");

         break;

      case VER_PLATFORM_WIN32s:
         wsprintf(lps, "MS Win32s ");
         break;
   }

   // Display version, service pack (if any), and build number.
   wsprintf(EndBuf(lps), "version %d.%d %s (Build %d).",
            osvi.dwMajorVersion,
            osvi.dwMinorVersion,
            osvi.szCSDVersion,
            osvi.dwBuildNumber & 0xFFFF );

   return TRUE; 
}


// eof DvAbout.c

