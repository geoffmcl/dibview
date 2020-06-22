

// DvTimer.c

#include	"dv.h"	// single inclusinve include
#include <time.h> // for time_t type cast

extern DWORD g_dwSibCount;
extern VOID Load_Sibling_File( HWND hWnd, DWORD num );

extern	void	PalAnim( HWND );

#ifdef	ADDTIMER1
extern	DWORD	DoAutoLoad( PLE pHead );
//extern	BOOL	fAutoLoad;
extern	void	NetAnim( HWND );
extern	void	EditBMPTimer( void );
extern	PMWL  CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller );
extern   VOID  GlazeMDIC1( HWND hWnd );
extern   HWND	ChkFileOpen( LPSTR lpf );

BOOL	fDnAutoLoad = FALSE;
DWORD	CTimeCnt = 0;
HWND	CTimeHnd[MXANIMS+1];

DWORD	GetAnims( HWND FAR *lpw, DWORD max )
{
	DWORD	i, j;
	j = 0;
	if( max )
	{
		for( i = 0; i < CTimeCnt; i++ )
		{
			if( CTimeHnd[i] )
			{
				lpw[j++] = CTimeHnd[i];
			}
			if( j >= max )
				break;
		}
	}
	return( j );
}

BOOL	DeleteAnim( HWND hwnd )
{
	BOOL	flg;
	DWORD	i;
	flg = TRUE;	/* Set error */
	if( CTimeCnt )
	{
		for( i = 0; i < CTimeCnt; i++ )
		{
			if( CTimeHnd[i] == hwnd )
				break;
		}
		if( i < CTimeCnt )
		{
			for( ; i < CTimeCnt; i++ )
			{
				CTimeHnd[i] = CTimeHnd[i+1];
			}
			CTimeHnd[i] = 0;
			CTimeCnt--;
			CTimeHnd[CTimeCnt] = 0;
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	AddToAnims( HWND hwnd )
{
	BOOL	flg;
	DWORD	i;

	flg = TRUE;	/* Set ERROR */
	if( CTimeCnt < MXANIMS )
	{
		for( i = 0; i < CTimeCnt; i++ )
		{
			if( CTimeHnd[i] == hwnd )
			{
				flg = FALSE;	/* OK, already in animation counter ... */
				break;
			}
		}
		if( i == CTimeCnt )
		{
			CTimeHnd[i] = hwnd;
			CTimeCnt++;
			flg = FALSE;	/* OK, added to animation counter ... */
		}
	}
	return( flg );
}

VOID  Frm_TIMER1( HWND hWnd )
{
   // { &szGens[0], &szAutoRLLast[0],it_BoolT,  (LPSTR)&gfAutoRLLast, &gfChgRLL, 0, 0 },
   if( gfAutoRLLast )
   {
      PLIST_ENTRY pHead, pNext;
      PMWL        pmwl, pwln;
      LPTSTR      lpf;

      pHead = &gsAutoList;
      pwln  = 0;
      if( !IsListEmpty(pHead) )
      {
         Traverse_List( pHead, pNext )
         {
            pmwl = (PMWL)pNext;
            lpf = &pmwl->wl_szFile[0];
            if( !(pmwl->wl_dwFlag & (flg_IsLoaded | flg_Reviewed)) )
            {
               if(( gnDIBsOpen       ) && // If already HAVE CHILDREN
                  ( ChkFileOpen(lpf) ) )  // and is already open
               {
                  // skip second open
                  pmwl->wl_dwFlag |= flg_IsLoaded; // currently LOADED in a child window
               }
               else
               {
                  pwln = CommonFileOpen( hWnd, &pmwl->wl_szFile[0], df_IDOPEN ); 
				      if(pwln)
                  {
                     pwln->wl_dwFlag |= flg_IsLoaded;
                     pwln->wl_dwFlag |= flg_Reviewed;   // reviewed for OPEN in timer
                     pmwl->wl_dwFlag |= flg_IsLoaded; // currently LOADED in a child window
                  }
               }

               //pwln->wl_dwFlag |= flg_Reviewed;   // reviewed for OPEN in timer
               pmwl->wl_dwFlag |= flg_Reviewed;   // reviewed for OPEN in timer
            }
         }
      }
   }
}

void	Dbg_WM_TIMER( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
//   static   DWORD gdwDoTMsg = 0;
   static   BOOL  bDnTime1 = FALSE;
   static   time_t   bgn, now;

   if( ( gdwDoTMsg % 360 ) == 0 )
   {
      if( !bDnTime1 )
      {
         time( &bgn );
         sprtf( "Main: Done FIRST Timer message timer id %d e=%s"MEOR,
            wParam,
            ELAPSTG );
         bDnTime1 = TRUE;
//         Frm_TIMER1( hWnd );  // just ONCE
      }
      else
      {
         DWORD mins;
         time_t   diff;
         time( &now );
         diff = now - bgn;
         mins  = (DWORD)(diff / 60);
         diff -= mins * 60;
         sprtf( "Main: Time msg id %d at %d:%02d m:s. e=%s"MEOR, wParam,
            mins, diff,
            ELAPSTG );  // (now - bgn) );
      }
   }


}

/* ==============================================================
 * Frm_WM_TIMER( HWND, UINT, WPARAM, LPARAM )
 *
 * Purpose: To handle GIF animations
 *
 * ============================================================== */
void	Frm_WM_TIMER( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	DWORD	i;
	if( wParam == TIMER_ID1 )
	{
      if( gdwDoTMsg == 0 )
         Frm_TIMER1( hWnd );  // just ONCE

		if( ( gfAutoLoad   ) &&
			 ( !fDnAutoLoad ) )
		{
			fDnAutoLoad = TRUE;
			DoAutoLoad( &gsFileList );  // autoload on this timer message
		}

		if( CTimeCnt ) // any DIB children needing a tick or two
		{
			for( i = 0; i < CTimeCnt; i++ )
			{
				if( CTimeHnd[i] ) // if we have a child handle
				{
               // post timer message to its queue
					PostMessage( CTimeHnd[i], msg, wParam, lParam );
					//break;
				}
			}
		}

		EditBMPTimer();

      Dbg_WM_TIMER( hWnd, msg, wParam, lParam );

      gdwDoTMsg++; // increment timer message count

      i = g_dwSibCount;
      if(i) {
         g_dwSibCount--;   // REDUCE count
         Load_Sibling_File( hWnd, i );  // LOAD FILE
      }
	}

//   GlazeMDIC1( hWnd );

}

#endif	/* ADDTIMER1 */

// If required the main WM_TIMER is passed onto the child
void	ChildWndTimer( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   static int iDoCWT1 = 0;
   if( ( iDoCWT1 % 360 ) == 0 )
   {
      // show child timer message
      sprtf( "Child: WM_TIMER from timer id %d (%#x)."MEOR,
         wParam, wParam );
   }
   iDoCWT1++;

#ifdef	ADDTIMER1

	if( wParam == TIMER_ID1 )
	{
		NetAnim( hWnd );
	}
#endif	/* ADDTIMER1 */

	if( wParam == TIMER_ID2 )
	{
		PalAnim( hWnd );
	}
}

// eof - DvTimer.c
