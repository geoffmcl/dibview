
/* 
 * Copyright 1995-99 Informatique Rouge. All rights reserved.
 *
 *  MODULE   : DDBWait.c, DvWait.c
 *
 *  PURPOSE  : Put up a WAIT cursor
 *
 *  FUNCTIONS:
 *	SRWait( BOOL, DWORD ) - Put up or take down the WAIT
 *
 *  HISTORY  :
 *  29 Sep 95  Created                                  Geoff R. McLane
 *  25 Sep 99  Added to DIBVIEW project (Dv32)          grm
 *
 */

//#include	"DDB.h"	/* General inclusive include ... */
#include	"dv.h"	/* General inclusive include ... */

#define	ghInst			ghDvInst	/* = WrkStr.w_hDvInst */
#define	HCUR_ERROR	((HCURSOR)-1)
#define	MXCURS		5	/* 5 in a sequence ... */

//HCURSOR	hOldCur = 0;	/* Old CURSOR ... 0 when DOWN or with previous */
//HCURSOR	HCurCur = 0;	/* Cursor resource ... */
HCURSOR	HCurC1[ MXCURS + 8 ] = { 0 };	/* set of cursors */
#define	hOldCur		HCurC1[ MXCURS + 1 ]
#define	HCurCur		HCurC1[ MXCURS + 2 ]

DWORD	CurNum = 0;
DWORD	wUseCnt = 0;	/* This allows nested calls - but WAIT stays UP */

/*	=====================================================
	USED AS:
	DBWaitON	= SRWait( TRUE, SRW_BGN )
	DBWaitINC	= SRWait( TRUE, SRW_INC ), AND
	DBWaitOFF	= SRWait( FALSE, SRW_END )

	===================================================== */
void	ShowLoadFailed( int	iIDC )
{
	static int _sfDnShow = 0;

	if( !_sfDnShow )
	{
		LPSTR	lps, lpt;

		lps = &gszMsg[0];
		lpt = &gszTit[0];

		lstrcpy( lpt, "RESOURCE LOAD ERROR" );
		wsprintf( lps,
			"ERROR: Unable to load the cursor\r\n"
			"with a value of %d(0x%x). Check the\r\n"
			"projects resource and their values!",
			iIDC, iIDC );

		MessageBox( NULL,
			lps,
			lpt,
			( MB_ICONINFORMATION | MB_OK ) );

		_sfDnShow = TRUE;
	}
}

void	SRCloseWait_NOT_USED( void )
{
	DWORD	i;
	HCURSOR	hc;
	if( hOldCur )
	{
		SetCursor ( hOldCur ) ;	/* Restore CURSOR ... */
		hOldCur = 0 ;	/* Remove old handle ... */
		CurNum = 0;
	}
	if( ( HCurCur ) &&
		( HCurCur != HCUR_ERROR ) )	/* If this has been loaded ... */
	{
		DestroyCursor( HCurCur );
	}
	HCurCur = 0;
	for( i = 0; i < MXCURS; i++ )
	{
		if( ( hc = HCurC1[i]   ) &&
			( hc != HCUR_ERROR ) )
		{
			DestroyCursor( hc );
		}
		HCurC1[i] = 0;
	}
}

void	SRWait( BOOL flg, DWORD typ )
{
	HCURSOR	hOld;
	HCURSOR	hNew;

 if( flg )
 {
	if( typ == SRW_BGN )
		wUseCnt++;			/* Bump the used count ... */

	if( hOldCur == 0 )	/* If we DO NOT HAVE an OLD CURSOR ... */
	{
		CurNum = 0;	/* Start of sequence ... */
		if( HCurCur )	/* If CURSOR loaded ... */
		{
			hOld = SetCursor( HCurCur ) ;
			hOldCur = hOld;
		}
		else
		{
			if ( HCurCur = LoadCursor( ghInst,
				MAKEINTRESOURCE( IDC_CURSOR1 ) ) )
			{
				hOldCur = SetCursor( HCurCur ) ;
			}
			else
			{
				ShowLoadFailed( IDC_CURSOR1 );
				hOldCur = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
			}
		}
	}
	else	/* First WAIT CURSOR already up ... */
	{	/* We will INCREMENT Cursor type ... if possible ... */
		if( HCurC1[CurNum] == 0 )
		{
			if( HCurC1[CurNum] = LoadCursor( ghInst, 
					MAKEINTRESOURCE( (IDC_CURSOR2 + CurNum) ) ) )
			{
				SetCursor( HCurC1[CurNum] ) ;
				CurNum++;
			}
			else
			{
				ShowLoadFailed( IDC_CURSOR2 + CurNum );
				HCurC1[CurNum] = HCUR_ERROR;
			}
		}
		else
		{
			hNew = HCurC1[CurNum];
			if( ( HCurCur            ) &&
				( hNew               ) &&
				( hNew != HCUR_ERROR ) )
			{
				if( CurNum >= MXCURS )	/* If at the END OF THE RUN ... */
				{
					SetCursor( HCurCur );	/* Back to START ... */
					CurNum = 0;
				}
				else
				{
					SetCursor( hNew );
					CurNum++;
				}
			}
		}
	}
 }
 else	/* Not TRUE, thus take it DOWN if SRW_END and wUseCnt = 0 */
 {
	if( typ == SRW_END )	/* If an END type ... */
	{
		if( wUseCnt )	/* and we have a USER count ... */
			wUseCnt--;	/* Reduce a USER ... */
	}
	if( ( wUseCnt == 0 ) &&
		( hOldCur      ) )	/* If there is an OLD cursor ... */
	{
		SetCursor ( hOldCur ) ;	/* Restore CURSOR ... */
		hOldCur = 0 ;	/* Remove old handle ... */
		CurNum = 0;
	}
 }
}

// eof - DDBWait.c -> DvWait.c

