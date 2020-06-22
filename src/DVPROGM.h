
// DvProgM.h
#ifndef	_DvProgM_H
#define	_DvProgM_H

// macros to run progress window
/*	====================================================
	USE BY SET OF:
		1	BeginProgM(0);	-- setup the progress meter --
		2	ProgMsg( "Moment, doing colour count ..." );
			for( i = 0; i < Height; i++ )
			{	// For each ROW
		3		ProgP( ( i / Height ) * 100 );
		4	KillProg();

	====================================================	*/
#ifdef	ADDPROGM

extern	BOOL	SetPercent( HWND hWnd );
extern	HWND	CreateInstofProg( void );
extern	void	PutProgM( void );
extern	void	StartProgM( int iPct );
extern	void	KillProgM( void );
extern	void	CheckMessages( void );
extern	void	SetProgM( DWORD dn, DWORD tot );
extern	void	SetProgP( int iPct );
extern	void	SetProgM1( void );
extern	void	SetProgMInfo( LPSTR lpi );
extern	void	SetProgMInfo2( LPSTR lpi );

/* setup the progress meter */
#define	BeginProgM(a)		StartProgM( a )
/* set message like - "Moment, doing colour count ..." */
#define	ProgMsg(lpi)		SetProgMInfo(lpi)

/* then pass % like for( i = 0; i < Height; i++ )  */
/*			{	// For each ROW	*/
/*		3		ProgP( ( i / Height ) * 100 ); */
#define	ProgP(p)			SetProgP(p)


#define	KillProg			KillProgM

#else	/* !ADDPROGM */

/* then DEFINE NOTHING */
#define	BeginProgM(a)
#define	ProgMsg(lpi)
#define	ProgP(p)
#define	KillProg

#endif	/* ADDPROGM y/n */

#endif	/* _DvProgM_H */
// eof - DvProgM.h
