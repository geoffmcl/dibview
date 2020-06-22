

// DbWait.h, DDBWait.h, DvWait.h
#ifndef	_DvWait_H
#define	_DvWait_H

#define SRW_BGN         1       /* Begin a WAIT if NOT already ON */
#define SRW_INC         2       /* Just bump to next animation */
#define SRW_END         3       /* If the LAST, remove WAIT */

extern	void	SRCloseWait( void );
extern	void	SRWait( BOOL, DWORD );

#ifdef	ADDSTATUS

extern	void SetSBTip( LPSTR );
extern	void SetSBTip1( void );
extern	void SetSBReady( void );

#define	DBWaitON \
{\
	SRWait( TRUE, SRW_BGN );\
	SetSBTip( "Wait - Moment ." );\
}

#define	DBWaitINC \
{\
	SRWait( TRUE, SRW_INC );\
	SetSBTip1();\
}

#define	DBWaitOFF \
{\
	SRWait( FALSE, SRW_END );\
	SetSBReady();\
}

#else	// !ADDSTATUS

#define	DBWaitON	SRWait( TRUE, SRW_BGN )
#define	DBWaitINC	SRWait( TRUE, SRW_INC )
#define	DBWaitOFF	SRWait( FALSE, SRW_END )

#endif	// ADDSTATUS

#endif	/* _DvWait_H */
// End - DbWait.h


